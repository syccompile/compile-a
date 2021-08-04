#include "../ir.h"
#include "../context/context.h"
#include <unordered_map>

namespace { // helper

// 将IR中所有使用的寄存器参数（前4个参数）都换成VAR
void
substitute_param(IR::List &l) {
  IR::List new_l;

  // 获取函数信息
  new_l.splice(new_l.end(), l, l.begin());
  std::string funcname = new_l.front()->a0->name;
  auto functab_ent = context.functab->get(funcname);

  std::vector<IR::Addr::Ptr> param_var_map(4);

  auto get_addr = [&](int v) -> IR::Addr::Ptr {
    if      (v>=4)                      return nullptr;
    else if (param_var_map[v]==nullptr) return param_var_map[v] = context.allocator.allocate_addr();
    else                                return param_var_map[v];
  };

  // 将寄存器中 PARAM类型变量 的使用转为对应的 VAR类型变量 的使用
  while (!(l.empty())) {
    auto ir = l.front();

    if (ir->a0!=nullptr && ir->op_!=IR::Op::PARAM) {
      if (ir->a0->kind==IR::Addr::Kind::PARAM && ir->a0->val<4)
        ir->a0 = get_addr(ir->a0->val);
      else if (ir->a0->kind==IR::Addr::RET)
        ir->a0 = functab_ent->get_param_addr(0);
    }

    if (ir->a1!=nullptr) {
      if (ir->a1->kind==IR::Addr::Kind::PARAM && ir->a1->val<4)
        ir->a1 = get_addr(ir->a1->val);
      else if (ir->a1->kind==IR::Addr::RET)
        ir->a1 = functab_ent->get_param_addr(0);
    }

    if (ir->a2!=nullptr) {
      if (ir->a2->kind==IR::Addr::Kind::PARAM && ir->a2->val<4)
        ir->a2 = get_addr(ir->a2->val);
      else if (ir->a2->kind==IR::Addr::RET)
        ir->a2 = functab_ent->get_param_addr(0);
    }
      
    new_l.splice(new_l.end(), l, l.begin());
  }

  // 在函数头部加入MOV指令
  l.splice(l.end(), new_l, new_l.begin());
  for (int i=0 ; i<4 ; i++) {
    auto var_addr = get_addr(i);
    if (var_addr!=nullptr) l.push_back(IR::make_binary(IR::Op::MOV, var_addr, functab_ent->get_param_addr(i)));
  }
  l.splice(l.end(), new_l);
}

void
substitute_time_func(IR::List &l) {
  for (auto ir: l) {
    if (ir->op_ == IR::Op::CALL) {
      if      (ir->a0->name == "starttime") ir->a0->name = "_sysy_starttime";
      else if (ir->a0->name == "stoptime")  ir->a0->name = "_sysy_stoptime";
    }
  }
}

// 将一个未知类型的地址a移动到寄存器（VAR）地址中
// 返回：[新地址, 需要向全局变量定义加入的IR，需要向函数定义加入的IR]
std::tuple<IR::Addr::Ptr, IR::List, IR::List>
move_into_var(IR::Addr::Ptr a) {

  IR::Addr::Ptr ret = nullptr;
  VarTabEntry::Ptr_const ent = nullptr;
  IR::List ret_def, ret_func;

#define CASE(TYPE) case IR::Addr::Kind:: TYPE :

  switch(a->kind) {
    CASE(VAR)
    CASE(IMM)
      ret = a;
      break;
      
    CASE(PARAM)
      // 通过寄存器传过来的参数
      if (a->val<4) {
        ret = a;
      }
      // 通过栈传过来的参数
      else {
        ret = context.allocator.allocate_addr();
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, a));
      }
      break;
      
    CASE(NAMED_LABEL)
      ret = context.allocator.allocate_addr();
      ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, a));
      break;

      default:
        ret = a;
        break;
  }

#undef CASE

  return std::make_tuple(ret, ret_def, ret_func);

}

}; // helper

void
ir_armify(std::list<IR::List> &defs, IR::List &func) {
  
  substitute_time_func(func);
  substitute_param(func);

  // 获取函数信息
  std::string funcname = func.front()->a0->name;
  auto functab_ent = context.functab->get(funcname);

  IR::List new_func;

  while (!(func.empty())) {
    auto ir = func.front();

    // ir为除法
    if (ir->op_==IR::Op::DIV) {
      auto param_divident = IR::make_binary(IR::Op::PARAM, functab_ent->get_param_addr(0), ir->a1);
      auto param_divisor  = IR::make_binary(IR::Op::PARAM, functab_ent->get_param_addr(1), ir->a2);
      auto call           = IR::make_unary (IR::Op::CALL,  IR::Addr::make_named_label("__aeabi_idiv"));
      auto move_into_var  = IR::make_binary(IR::Op::MOV,   ir->a0, functab_ent->get_param_addr(0));

      func.pop_front();
      func.push_front(move_into_var);
      func.push_front(call);
      func.push_front(param_divident);
      func.push_front(param_divisor);

      continue;
    }

    // ir为取模
    else if (ir->op_==IR::Op::MOD) {
      auto param_divident = IR::make_binary(IR::Op::PARAM, functab_ent->get_param_addr(0), ir->a1);
      auto param_divisor  = IR::make_binary(IR::Op::PARAM, functab_ent->get_param_addr(1), ir->a2);
      auto call           = IR::make_unary (IR::Op::CALL,  IR::Addr::make_named_label("__aeabi_idivmod"));
      auto move_into_var  = IR::make_binary(IR::Op::MOV,   ir->a0, functab_ent->get_param_addr(1));

      func.pop_front();
      func.push_front(move_into_var);
      func.push_front(call);
      func.push_front(param_divident);
      func.push_front(param_divisor);
      
      continue;
    }

    // ir为除法、取模外的算术逻辑型，a0  = a1 op a2
    // 或者是cmp型     nil = a1 op a2
    if (ir->is_al() || ir->op_==IR::Op::CMP) {
      // 首先保证a1、a2在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      auto [a2, a2_def_app, a2_func_app] = move_into_var(ir->a2);
      // 将需要加入的IR加入对应IR串中
      if (!(a1_def_app.empty())) defs.push_back(a1_def_app);
      if (!(a2_def_app.empty())) defs.push_back(a2_def_app);
      new_func.splice(new_func.end(), a1_func_app);
      new_func.splice(new_func.end(), a2_func_app);
      // 修改原IR
      ir->a1 = a1;
      ir->a2 = a2;
    }

    // ir为移动型
    else if (ir->is_mov()) {
      // 首先保证a1在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      // 将需要加入的IR加入对应IR串中
      if (!(a1_def_app.empty())) defs.push_back(a1_def_app);
      new_func.splice(new_func.end(), a1_func_app);
      // 修改原IR
      ir->a1 = a1;
    }

    // ir为传参型
    else if (ir->op_==IR::Op::PARAM) {
      // 首先保证a1在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      if (!(a1_def_app.empty())) defs.push_back(a1_def_app);
      new_func.splice(new_func.end(), a1_func_app);
      // 修改原IR
      ir->a1 = a1;
    }

    // 数组变址取数
    else if (ir->op_==IR::Op::LOAD) {
      // 保证基址在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      // 将需要加入的IR加入对应IR串中
      if (!(a1_def_app.empty())) defs.push_back(a1_def_app);
      new_func.splice(new_func.end(), a1_func_app);
      // 修改原ir
      ir->a1 = a1;
    }

    // 数组变址存数
    else if (ir->op_==IR::Op::STORE) {
      // 保证基址在VAR中
      auto [a0, a0_def_app, a0_func_app] = move_into_var(ir->a0);
      // 将需要加入的IR加入对应IR串中
      if (!(a0_def_app.empty())) defs.push_back(a0_def_app);
      new_func.splice(new_func.end(), a0_func_app);
      // 修改原ir
      ir->a0 = a0;

      // 保证源地址在VAR中
      auto [a2, a2_def_app, a2_func_app] = move_into_var(ir->a2);
      // 将需要加入的IR加入对应IR串中
      if (!(a2_def_app.empty())) defs.push_back(a2_def_app);
      new_func.splice(new_func.end(), a2_func_app);
      // 修改原ir
      ir->a2 = a2;
    }

    new_func.splice(new_func.end(), func, func.begin());
  }

  func.splice(func.end(), new_func);

}
