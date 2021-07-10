#include "translate_to_asm.h"
#include <unordered_map>
#include <sstream>

// TODO 根据IR操作码获取ARM操作码
std::string
get_asm_opcode(IR::Op op) {

#define CASE(IR_OP, ASM_OP) case IR::Op:: IR_OP : return std::string(ASM_OP)

  switch(op) {
    CASE(ADD, "add");
    CASE(SUB, "sub");
    CASE(MUL, "mul");
    CASE(DIV, "div");

    CASE(MOV, "mov");
    CASE(MOVLE, "movle");
    CASE(MOVLT, "movlt");
    CASE(MOVGE, "movge");
    CASE(MOVGT, "movgt");
    CASE(MOVEQ, "moveq");
    CASE(MOVNE, "movne");

    CASE(JMP, "b");
    CASE(JLE, "ble");
    CASE(JLT, "blt");
    CASE(JGE, "jge");
    CASE(JGT, "jgt");
    CASE(JE, "je");
    CASE(JNE, "jne");
    default: return std::string("");
  }

#undef CASE

}

template <class T> std::string
to_string(T val) {
  std::stringstream sstream;
  sstream << val;
  return sstream.str();
}

std::pair<std::list<IR::List>, std::list<IR::List> >
split_decl_func(IR::List &l) {
  std::list<IR::List> decl, func;

  while (!(l.empty())) {
    if (l.front()->op_==IR::Op::VARDEF) {
      IR::List tmp;
      tmp.splice(tmp.end(), l, l.begin());
      while (tmp.back()->op_ != IR::Op::VAREND) tmp.splice(tmp.end(), l, l.begin());
      decl.push_back(tmp);
    }
    else if (l.front()->op_==IR::Op::FUNCDEF) {
      IR::List tmp;
      tmp.splice(tmp.end(), l, l.begin());
      while (tmp.back()->op_ != IR::Op::FUNCEND) tmp.splice(tmp.end(), l, l.begin());
      func.push_back(tmp);
    }
    else l.pop_front();
  }

  return std::make_pair(decl, func);
}

std::list<std::string>
translate_function(IR::List &l) {
  std::list<std::string> ret;

  // 栈帧分配表<虚寄存器号, 栈帧地址>
  std::unordered_map<int, int> var_alloc;
  // 栈帧分配位置
  int acc = 32;

  // 局部数组表
  std::unordered_map<int, bool> array_defined_in_frame;

  // 查找虚拟地址栈帧位置
  // 如果还未给虚拟地址分配空间，就分配空间
  auto get_offset = [&var_alloc, &acc](int regnum, int size=4) {
    if (var_alloc.count(regnum)) return var_alloc[regnum];
    var_alloc[regnum] = acc + size;
    acc += size;
    return acc;
  };

  // IR地址存储的值移入寄存器
  auto move_value = [&var_alloc, &acc, &get_offset](IR::Addr::Ptr addr, int dst_reg) {
    std::list<std::string> r;

    // 变量型地址
    if (addr->kind==IR::Addr::Kind::VAR) {
      r.push_back(std::string("ldr\tr" + to_string(dst_reg) + ", [fp, #-") + to_string(get_offset(addr->val)) + "]");\
    }
    // 参数型地址
    else if (addr->kind==IR::Addr::PARAM) {
      if (addr->val>=4) r.push_back(std::string("ldr\tr") + to_string(dst_reg) + ", [fp, #" + to_string((addr->val-3)*4) + "]");
      else              r.push_back(std::string("mov\tr") + to_string(dst_reg) + ", r" + to_string(addr->val));
    }
    // 具名变量
    else if (addr->kind==IR::Addr::NAMED_LABEL) {
      r.push_back(std::string("ldr\tr") + to_string(dst_reg) + ", =" + addr->name);
    }
    // 立即数
    else r.push_back(std::string("mov\tr" + to_string(dst_reg) + ", #") + to_string(addr->val));

    return r;
  };
  // IR地址对应的地址移入寄存器
  auto move_addr = [&var_alloc, &acc, &get_offset, &ret](IR::Addr::Ptr addr, int dst_reg) {
    std::list<std::string> r;
    // 变量型地址
    if (addr->kind==IR::Addr::Kind::VAR) {
      r.push_back(std::string("add\tr") + to_string(dst_reg) + ", fp, #-" + to_string(get_offset(addr->val)));
    }
    // 具名变量
    else {
      r.push_back(std::string("ldr\tr") + to_string(dst_reg) + ", ." + addr->name + "_offset");
    }

    return r;
  };
  // 寄存器存入IR地址
  auto store_value = [&var_alloc, &acc, &get_offset, &ret](IR::Addr::Ptr addr, int src_reg) {
    std::list<std::string> r;
    // 变量型地址
    if (addr->kind==IR::Addr::Kind::VAR) {
      r.push_back(std::string("str\tr") + to_string(src_reg) + ", [fp, #-" + to_string(get_offset(addr->val)) + "]");
    }
    // 参数型地址
    else if (addr->kind==IR::Addr::PARAM) {
      if (addr->val>=4) r.push_back(std::string("str\tr") + to_string(src_reg) + ", [fp, #" + to_string((addr->val-3)*4) + "]");
      else              r.push_back(std::string("mov\tr") + to_string(addr->val) + ", r" + to_string(src_reg));
    }
    // 具名变量
    else {
      r.push_back(std::string("str\tr") + to_string(src_reg) + ", =" + addr->name);
    }
    return r;
  };

  auto store_arr = [&](IR::Addr::Ptr addr, IR::Addr::Ptr offset, int src_reg, int tmp_reg1, int tmp_reg2) {
    std::list<std::string> r;
    // 首先加载数组基址
    if (addr->kind==IR::Addr::Kind::NAMED_LABEL) {
      r.splice(r.end(), move_addr(addr, tmp_reg1));
    }
    else if (addr->kind==IR::Addr::Kind::VAR) {
      if (array_defined_in_frame.count(addr->val)) r.splice(r.end(), move_addr(addr, tmp_reg1));
      else                                         r.splice(r.end(), move_value(addr, tmp_reg1));
    }
    else if (addr->kind==IR::Addr::Kind::PARAM) {
      r.splice(r.end(), move_value(addr, tmp_reg1));  
    }

    // 然后加载变址
    r.splice(r.end(), move_value(offset, tmp_reg2));

    // 执行store
    r.push_back(std::string("str\tr") + to_string(src_reg) + " , [r" + to_string(tmp_reg1) + ", r" + to_string(tmp_reg2) + "]");

    return r;
  };

  auto load_arr = [&](IR::Addr::Ptr addr, IR::Addr::Ptr offset, int dst_reg, int tmp_reg1, int tmp_reg2) {
    std::list<std::string> r;
    // 首先加载数组基址
    if (addr->kind==IR::Addr::Kind::NAMED_LABEL) {
      r.splice(r.end(), move_addr(addr, tmp_reg1));
    }
    else if (addr->kind==IR::Addr::Kind::VAR) {
      if (array_defined_in_frame.count(addr->val)) r.splice(r.end(), move_addr(addr, tmp_reg1));
      else                                         r.splice(r.end(), move_value(addr, tmp_reg1));
    }
    else if (addr->kind==IR::Addr::Kind::PARAM) {
      r.splice(r.end(), move_value(addr, tmp_reg1));  
    }

    // 然后加载变址
    r.splice(r.end(), move_value(offset, tmp_reg2));

    // 执行load
    r.push_back(std::string("ldr\tr") + to_string(dst_reg) + " , [r" + to_string(tmp_reg1) + ", r" + to_string(tmp_reg2) + "]");

    return r;
  };

  auto load_param = [&](IR::Addr::Ptr addr, int dst_reg) {
    std::list<std::string> r;
    if (addr->kind==IR::Addr::Kind::NAMED_LABEL) {
      // auto ent = context
      r.splice(r.end(), move_addr(addr, dst_reg));
    }
    else if (addr->kind==IR::Addr::Kind::VAR) {
      if (array_defined_in_frame.count(addr->val)) r.splice(r.end(), move_addr(addr, dst_reg));
      else                                         r.splice(r.end(), move_value(addr, dst_reg));
    }
    else if (addr->kind==IR::Addr::Kind::PARAM) {
      r.splice(r.end(), move_value(addr, dst_reg));  
    }
    else if (addr->kind==IR::Addr::Kind::IMM) {
      r.splice(r.end(), move_value(addr, dst_reg));
    }
    return r;
  }; 

  // 函数名称
  std::string name = l.front()->a0->name;
  ret.push_back(std::string(".global\t") + name);
  ret.push_back(std::string(".type\t") + name + ", %function");
  ret.push_back(name + ":");
  // 将被调用者保存的9个寄存器统统压栈
  ret.push_back("push\t{fp, lr, r4-r10}");
  // 将fp指向新的栈帧
  ret.push_back("add\tfp, sp, #32");

  while (!l.empty()) {
    auto ir = l.front();

    // 算术逻辑型
    if (ir->op_<IR::Op::MOD) {
      // 首先获取栈帧地址
      // 源1
      ret.splice(ret.end(), move_value(ir->a1, 4));
      // 源2
      ret.splice(ret.end(), move_value(ir->a2, 5));

      // 运算
      ret.push_back(get_asm_opcode(ir->op_) + "\tr6, r4, r5");
      // 写回
      ret.splice(ret.end(), store_value(ir->a0, 6));
    }
    // MOV类型
    else if (ir->op_<=IR::Op::MOVNE) {
      // 首先获取栈帧地址
      // 源1
      ret.splice(ret.end(), move_value(ir->a1, 4));
      // 运算
      ret.push_back(get_asm_opcode(ir->op_) + "\tr6, r4");
      // 写回
      ret.splice(ret.end(), store_value(ir->a0, 6));
    }
    // CMP
    else if (ir->op_==IR::Op::CMP) {
      // 首先获取源地址
      // 源1
      ret.splice(ret.end(), move_value(ir->a1, 4));
      // 源2
      ret.splice(ret.end(), move_value(ir->a2, 5));
      // 运算
      ret.push_back(std::string("cmp\t") + "r4, r5");
    }
    // LABEL
    else if (ir->op_==IR::Op::LABEL) {
      ret.push_back(".L" + to_string(ir->a0->val) + ":");
    }
    // JMP
    else if (ir->op_<=IR::Op::JNE) {
      ret.push_back(get_asm_opcode(ir->op_) + "\t.L" + to_string(ir->a0->val));
    }
    // PARAM+CALL
    else if (ir->op_==IR::Op::PARAM) {
      // 先把所有的PARAM都放进list中
      std::list<IR::Ptr> param_list;
      while (l.front()->op_==IR::Op::PARAM) param_list.splice(param_list.end(), l, l.begin());
      // 计算传参所需栈帧的大小
      // 除了前四个参数放在寄存器内，后面的参数都要压栈
      int stack_size = (param_list.size()*4-16);
      if (stack_size<0) stack_size = 0;
      
      // 第四个参数以后的参数压栈
      while (param_list.size()>=8) {
        for (int i=0 ; i<4 ; i++) {
          ret.splice(ret.end(), move_value(param_list.back()->a0, i)); 
          param_list.pop_back();
        }
        ret.push_back(std::string("push\t{r0-r4}"));
      }
      int remain_push = param_list.size() - 4;
      for (int i=0 ; i<remain_push ; i++) {
        ret.splice(ret.end(), move_value(param_list.back()->a0, i)); 
        param_list.pop_back();
      }
      if (remain_push>1) ret.push_back(std::string("push\t{r0-r" + to_string(remain_push-1) + "}"));
      else if (remain_push==1) ret.push_back(std::string("str\tr0, [sp, #4]!"));

      // 前四个参数放在a0到a3中
      int cnt = 0;
      for (auto &ir_: param_list) {
        ret.splice(ret.end(), move_value(ir_->a1, cnt++));
      }
      param_list.clear();
      // 执行函数调用
      ret.push_back(std::string("bl\t") + l.front()->a0->name);
      // 恢复栈
      ret.push_back(std::string("add\t sp, sp, #") + to_string(stack_size));
    }
    // CALL
    else if (ir->op_==IR::Op::CALL) {
      ret.push_back("bl\t" + l.front()->a0->name);
    }
    // RET
    else if (ir->op_==IR::Op::RET) {
      ret.push_back("add\tsp, fp, #-32");
      ret.push_back("pop\t{fp, pc, r4-r10}");
    }
    // LOAD
    else if (ir->op_==IR::Op::LOAD) {
      ret.splice(ret.end(), load_arr(ir->a1, ir->a2, 4, 5, 6));
      ret.splice(ret.end(), store_value(ir->a0, 4));
    }
    // STORE
    else if (ir->op_==IR::Op::STORE) {
      ret.splice(ret.end(), move_value(ir->a2, 4));
      ret.splice(ret.end(), store_arr(ir->a0, ir->a1, 4, 5, 6));
    }
    // ALLOC_IN_STACK
    else if (ir->op_==IR::Op::ALLOC_IN_STACK) {
      get_offset(ir->a0->val, ir->a1->val*4);
    }
    l.pop_front();
    ret.push_back("");
  }
  return ret;
}

std::list<std::string>
translate_decl(IR::List &l) {

  std::list<std::string> ret;

  std::string name = l.front()->a0->name;
  ret.push_back(std::string(".global\t") + name);
  ret.push_back(name + ":");
  l.pop_front();

  while (l.front()->op_!=IR::Op::VAREND) {
    auto ir = l.front();

    if (ir->op_==IR::Op::DATA) {
      ret.push_back(std::string(".word\t") + to_string(ir->a0->val));
    }
    else if (ir->op_==IR::Op::ZERO) {
      ret.push_back(std::string(".space\t") + to_string(ir->a0->val*4));
    }

    l.pop_front();
  }

  ret.push_back(std::string(".") + name + "_offset:");
  ret.push_back(std::string(".word\t") + name);

  return ret;
}

std::list<std::string>
translate_arm(IR::List &l) {
  std::list<std::string> ret;

  auto pll = split_decl_func(l);
  // 翻译全局变量
  ret.push_back(std::string(".data"));
  for (auto &decl: pll.first) ret.splice(ret.end(), translate_decl(decl));

  // 翻译函数
  ret.push_back(std::string(".text"));
  for (auto &func: pll.second) ret.splice(ret.end(), translate_function(func));

  return ret;
}
