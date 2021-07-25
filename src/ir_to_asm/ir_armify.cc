#include "../ir.h"
#include "../context/context.h"
#include <unordered_map>

namespace { // helper

// 将val循环左移n位
uint32_t
rol(uint32_t val, uint32_t n) {
  n %= 32u;
  uint32_t ret;
  ret  = val << n;
  ret |= (val & (0xffffffffu << (32-n))) >> (32-n);
  return ret;
}

// 判断是否为arm立即数
bool
is_arm_imm(uint32_t imm) {
  for (int j=0 ; j<8 ; j++) {
    uint32_t rol_result = rol(imm, j*2);
    if (rol_result <= 0x000000ffu) return true;
  }
  return false;
}

// 将IR中所有使用的寄存器参数（前4个参数）都换成VAR
void
substitute_param(IR::List &l) {
  IR::List new_l;

  // get function name
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
    if (ir->a1->kind==IR::Addr::Kind::PARAM && ir->a1->val<4)
      ir->a1 = get_addr(ir->a1->val);
    else if (ir->a1->kind==IR::Addr::RET)
      ir->a1 = functab_ent->get_param_addr(0);

    if (ir->a2->kind==IR::Addr::Kind::PARAM && ir->a1->val<4)
      ir->a2 = get_addr(ir->a2->val);
    else if (ir->a2->kind==IR::Addr::RET)
      ir->a2 = functab_ent->get_param_addr(0);
      
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

// 将一个未知类型的地址a移动到寄存器（VAR）地址中
// 返回：[新地址, 需要向全局变量定义加入的IR，需要向函数定义加入的IR]
std::tuple<IR::Addr::Ptr, IR::List, IR::List>
move_into_var(IR::Addr::Ptr a) {

  // 存储无法被ARM指令表示的立即数之内存地址
  static std::unordered_map<int, IR::Addr::Ptr> imm_map;
  // 存储全局数组标号地址
  static std::unordered_map<std::string, IR::Addr::Ptr> glob_var_map;

  IR::Addr::Ptr ret = nullptr;
  VarTabEntry::Ptr_const ent = nullptr;
  IR::List ret_def, ret_func;

#define CASE(TYPE) case IR::Addr::Kind:: TYPE :

  switch(a->kind) {
    CASE(VAR)
      ret = a;
      break;
      
    CASE(IMM)
      // 立即数本身可被ARM指令表示
      if (is_arm_imm(a->val)) {
        ret = a;
      }
      // 立即数反值也可
      // 利用MVN
      // 例：mvn %1, 0 同 mov %1, ~0 同 mov %1, -1
      else if (is_arm_imm(~(a->val))) {
        ret = context.allocator.allocate_addr();
        ret_func.push_back(IR::make_binary(
          IR::Op::MVN,
          ret,
          IR::Addr::make_imm(~(a->val))
        ));
      }
      // 其他立即数：必须从内存载入
      else {
        IR::Addr::Ptr imm_globl_addr = nullptr;
        ret = context.allocator.allocate_addr();

        // 先查找是否已经有该数的全局变量
        // 若无
        if (imm_map.count(a->val) == 0) {
          // 建立地址
          imm_globl_addr = IR::Addr::make_named_label(std::string(".imm_") + std::to_string(a->val));
          // 创建新全局变量
          ret_def.push_back(IR::make_unary(IR::Op::VARDEF, imm_globl_addr));
          ret_def.push_back(IR::make_unary(IR::Op::DATA, IR::Addr::make_imm(a->val)));
          ret_def.push_back(IR::make_no_operand(IR::Op::VAREND));
          // 加入表中
          imm_map[a->val] = imm_globl_addr;
        }
        // 若有
        else {
          imm_globl_addr = imm_map[a->val];
        }

        // 移动到VAR中
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, imm_globl_addr));
      }
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
      // 从全局变量表获取信息
      ent = context.vartab_cur->get(a->name);
      ret = context.allocator.allocate_addr();

      // a代表数组名
      // 此时移入a的地址
      if (ent->is_array()) {
        IR::Addr::Ptr label_addr = nullptr;
        // 如果这个名字的标号地址还未生成
        if (glob_var_map.count(a->name)==0) {
          label_addr = IR::Addr::make_named_label(std::string(".var_ptr_") + a->name);
          // 创建新全局变量
          ret_def.push_back(IR::make_unary(IR::Op::VARDEF, label_addr));
          ret_def.push_back(IR::make_unary(IR::Op::DATA, IR::Addr::make_imm(a->val)));
          ret_def.push_back(IR::make_no_operand(IR::Op::VAREND));
          // 加入表中
          glob_var_map[a->name] = label_addr;
        }
        // 如果这个名字的标号地址已经生成
        else {
          label_addr = glob_var_map[a->name];
        }
        // 移动到VAR中
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, label_addr));
      }
      // a代表单一变量名
      // 此时移入a的内容
      else {
        // 移动到VAR中
        ret_func.push_back(IR::make_binary(IR::Op::MOV, ret, a));
      }
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
ir_armify(IR::List &defs, IR::List &func) {
  
  substitute_param(func);

  IR::List new_func;

  while (!(func.empty())) {
    auto ir = func.front();

    // ir为算术逻辑型，a0  = a1 op a2
    // 或者是cmp型     nil = a1 op a2
    if (ir->is_al() || ir->op_==IR::Op::CMP) {
      // 首先保证a1、a2在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      auto [a2, a2_def_app, a2_func_app] = move_into_var(ir->a2);
      // 将需要加入的IR加入对应IR串中
      defs.splice(defs.end(), a1_def_app);
      defs.splice(defs.end(), a2_def_app);
      new_func.splice(func.end(), a1_func_app);
      new_func.splice(func.end(), a2_func_app);
      // 修改原IR
      ir->a1 = a1;
      ir->a2 = a2;
    }

    // ir为移动型
    else if (ir->is_mov()) {
      // 首先保证a1在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      // 将需要加入的IR加入对应IR串中
      defs.splice(defs.end(), a1_def_app);
      new_func.splice(func.end(), a1_func_app);
      // 修改原IR
      ir->a1 = a1;
    }

    // ir为传参型
    else if (ir->op_==IR::Op::PARAM) {
      // 若目的为寄存器（前4个参数）
      // 则无需多余寄存器
      // 反之，则需要先将目标保存在寄存器中
      if (ir->a2->val>4) {
        // 首先保证IR在VAR中
        auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
        // 将需要加入的IR加入对应IR串中
        defs.splice(defs.end(), a1_def_app);
        new_func.splice(func.end(), a1_func_app);
        // 修改原IR
        ir->a1 = a1;
      }
    }

    // 数组变址取数
    else if (ir->op_==IR::Op::LOAD) {
      // 保证基址在VAR中
      auto [a1, a1_def_app, a1_func_app] = move_into_var(ir->a1);
      // 将需要加入的IR加入对应IR串中
      defs.splice(defs.end(), a1_def_app);
      new_func.splice(func.end(), a1_func_app);
      // 修改原ir
      ir->a1 = a1;
    }

    // 数组变址存数
    else if (ir->op_==IR::Op::STORE) {
      // 保证基址在VAR中
      auto [a0, a0_def_app, a0_func_app] = move_into_var(ir->a0);
      // 将需要加入的IR加入对应IR串中
      defs.splice(defs.end(), a0_def_app);
      new_func.splice(func.end(), a0_func_app);
      // 修改原ir
      ir->a0 = a0;

      // 保证源地址在VAR中
      auto [a2, a2_def_app, a2_func_app] = move_into_var(ir->a2);
      // 将需要加入的IR加入对应IR串中
      defs.splice(defs.end(), a2_def_app);
      new_func.splice(func.end(), a2_func_app);
      // 修改原ir
      ir->a2 = a2;
    }

    new_func.splice(new_func.end(), func, func.begin());
  }

  func.splice(func.end(), new_func);

}
