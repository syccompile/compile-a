#include "../ir.h"
#include "../reg_allocate/reg_allocate.h"
#include "reg_assign.h"
#include <list>
#include <string>
#include <cassert>

namespace {  // helper

using std::string;
using std::to_string;

// 栈分配格局
/*
  -----------------
 |   寄存器保存区   |
  -----------------
 |   溢出变量区     |
  -----------------
 |   局部数组区     |
  -----------------
 |   参数传递区     |
  -----------------  <- sp
 */
class Stack_Alloc {
public:
  int param_size;
  int local_arr_size;
  int spill_size;
  int register_save_size;

  // color -> stack_frame
  std::unordered_map<int, int> spill;
  // ir_addr.val -> stack_frame
  std::unordered_map<int, int> local_arr;

  int spill_offset() { return param_size + local_arr_size; };
  int local_arr_offset() { return param_size; };
  int param_offset() { return 0; };
  int frame_offset() { return param_size + local_arr_size + spill_size + register_save_size; };
};

std::string
get_arm_opcode(IR::Op op) {

#define CASE(IR_OP, ARM_OP) case IR::Op:: IR_OP : return string(ARM_OP)

  switch(op) {
    CASE(ADD, "add");
    CASE(SUB, "sub");
    CASE(MUL, "mul");
    CASE(DIV, "div");
    CASE(AND, "and");
    CASE(OR,  "orr");
    CASE(XOR, "eor");
    default: return string("");
  }

#undef CASE

}

Stack_Alloc
calculate_stack(IR::List &l, std::unordered_map<int, int> &color_assign) {
  Stack_Alloc ret;

  // 计算函数顶部需要的保存寄存器的空间
  // 需要用到的变量和r10, r11, lr
  // TODO 叶子节点优化
  ret.register_save_size = color_assign.size() + 3;

  // 计算溢出颜色所需的空间和对应偏移
  auto normal_addr_proc = [&color_assign, &ret](IR::Addr::Ptr ir_addr) {
    if (ir_addr != nullptr && ir_addr->get_color() != color_graph::none_color()) {
      if (color_assign.count(ir_addr->get_color())) ;
      else if (ret.spill.count(ir_addr->get_color())) ;
      else {
        ret.spill_size += 1;
        ret.spill[ir_addr->get_color()] = ret.spill_size;
      }
    }
  };

  // 计算局部数组与函数传参所需空间与偏移
  for (auto &ir: l) {
    if (ir->op_ == IR::Op::ALLOC_IN_STACK) {
      ret.local_arr_size += ir->a1->val;
      ret.local_arr[ir->a0->val] = ret.local_arr_size;
    }
    else if (ir->op_ == IR::Op::PARAM) {
      ret.param_size = std::max(ret.param_size, ir->a0->val-4);
    }
    else {
      normal_addr_proc(ir->a0);
      normal_addr_proc(ir->a1);
      normal_addr_proc(ir->a2);
    }
  }

  return ret;
}

// 将VAR型变量移动到内存中
std::pair<string, std::list<string> >
move_var_to_reg(Stack_Alloc &stack, std::unordered_map<int, int> color_assign, IR::Addr::Ptr ir_addr, std::string tmp_regname) {
  
  color addr_color = ir_addr->get_color();
  int   addr_val   = ir_addr->val;
  std::list<string> ret;
  string ret_reg;

  // 如果属于预着色
  if (addr_color < 4) ret_reg = std::string("r") + to_string(addr_color);
  // 如果属于寄存器内颜色
  else if (color_assign.count(addr_color)) ret_reg = std::string("r") + to_string(color_assign[addr_color]);
  // 如果属于溢出颜色
  else if (stack.spill.count(addr_color)) {
    ret_reg = tmp_regname;
    ret.push_back(string("ldr\t") + tmp_regname + " [sp, #" + to_string(4*(stack.spill_offset()+stack.spill[addr_color]-1)) + "]");
  }
  // 如果属于局部数组
  // 那么就将局部数组地址存入count
  else if (stack.local_arr.count(addr_val)) {
    ret_reg = tmp_regname;
    int arr_offset = 4 * (stack.local_arr_offset() + stack.local_arr[addr_val] - 1);
    ret.push_back(string("add\t") + tmp_regname + " sp, #" + to_string(arr_offset));
  }

  return std::make_pair(ret_reg, ret);
}

// 生成将寄存器内容移动到VAR型变量的语句
std::pair<string, std::list<string> >
store_reg_to_var(Stack_Alloc &stack, std::unordered_map<int, int> color_assign, IR::Addr::Ptr ir_addr, std::string tmp_regname) {

  color addr_color = ir_addr->get_color();
  int   addr_val   = ir_addr->val;
  std::list<string> ret;
  string ret_reg;

  // 如果属于预着色
  if (addr_color < 4) ret_reg = std::string("r") + to_string(addr_color);
  // 如果属于寄存器内颜色
  else if (color_assign.count(addr_color)) ret_reg = std::string("r") + to_string(color_assign[addr_color]);
  // 如果属于溢出颜色
  else if (stack.spill.count(addr_color)) {
    ret_reg = tmp_regname;
    ret.push_back(string("str\t") + tmp_regname + " [sp, #" + to_string(4*(stack.spill_offset()+stack.spill[addr_color]-1)) + "]");
  }

  return std::make_pair(ret_reg, ret);
}

std::list<string>
translate(Stack_Alloc &stack, std::unordered_map<int, int> color_assign, IR::Ptr ir) {
  std::list<std::string> ret;

  if (ir->is_al()) {
    auto [a1_reg, a1_arm] = move_var_to_reg(stack, color_assign, ir->a1, string("r10"));
    auto [a2_reg, a2_arm] = move_var_to_reg(stack, color_assign, ir->a2, string("fp"));
    auto [a0_reg, a0_arm] = store_reg_to_var(stack, color_assign, ir->a0, string("fp"));
    // 将a1与a2取入寄存器
    ret.splice(ret.end(), a1_arm);
    ret.splice(ret.end(), a2_arm);
    // 计算a0值
    ret.push_back(get_arm_opcode(ir->op_) + "\t" + a0_reg + ", " + a1_reg + a2_reg);
    // 将a0存入分配到的地址中
    ret.splice(ret.end(), a0_arm);
  }
  else if (ir->is_mov()) {
    
  }
  else if (ir->is_jmp()) {

  }
}

};  // helper

std::list<string>
translate_function(IR::List &l) {
  std::list<string> ret;

  // register allocate and assign
  register_allocate(l);
  auto color_assign = register_assign(l);

  // stack space calculation
  Stack_Alloc stack = calculate_stack(l, color_assign);

  // generate function ir code
  string name = l.front()->a0->name;
  ret.push_back(string(".global\t") + name);
  ret.push_back(string(".type\t") + name + ", %function");
  ret.push_back(string(".arm"));
  ret.push_back(name + ":");

  // pre-process
  ret.push_back(string("push\t{lr, r4-r11}"));
  ret.push_back(string("sub\tsp, sp, #") + to_string(stack.stack_size()));

  // actual translate
  ret.splice(ret.end(), translate(stack));

  // post-process
  ret.push_back(string("add\tsp, sp, #") + to_string(stack.stack_size()));
  ret.push_back(string("pop\t{pc, r4-r11}"));
}
