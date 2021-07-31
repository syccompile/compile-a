#include "../ir.h"
#include "../reg_allocate/reg_allocate.h"
#include "../context/context.h"
#include "frame_info.h"
#include "reg_assign.h"
#include <list>
#include <string>
#include <cassert>
#include <iostream>

namespace {  // helper

using std::string;
using std::to_string;

std::string
get_arm_opcode(IR::Op op) {

#define CASE(IR_OP, ARM_OP) case IR::Op:: IR_OP : return string(ARM_OP)

  switch(op) {
    CASE(ADD, "\tadd");
    CASE(SUB, "\tsub");
    CASE(MUL, "\tmul");
    CASE(DIV, "\tdiv");
    CASE(AND, "\tand");
    CASE(OR,  "\torr");
    CASE(XOR, "\teor");
    CASE(MOV, "\tmov");
    CASE(MVN, "\tmvn");
    CASE(MOVLE, "\tmovle");
    CASE(MOVLT, "\tmovlt");
    CASE(MOVGE, "\tmovge");
    CASE(MOVGT, "\tmovgt");
    CASE(MOVEQ, "\tmoveq");
    CASE(MOVNE, "\tmovne");
    CASE(JMP, "\tb");
    CASE(JLE, "\tble");
    CASE(JLT, "\tblt");
    CASE(JGE, "\tbge");
    CASE(JGT, "\tbgt");
    CASE(JE,  "\tbeq");
    CASE(JNE, "\tbne");
    default: return string("");
  }

#undef CASE

}

// 将变量移动到寄存器中
std::pair<string, std::list<string> >
move_to_reg(FrameInfo &frame, IR::Addr::Ptr ir_addr, std::string tmp_regname) {

  auto & reg_assign = frame.reg_assign;
  
  color addr_color = ir_addr->get_color();
  int   addr_val   = ir_addr->val;
  std::list<string> ret;
  string ret_reg;

  if (ir_addr->kind == IR::Addr::Kind::VAR) {
    // 如果属于预着色
    if (addr_color <= 4 && addr_color > 0) ret_reg = std::string("r") + to_string(addr_color-1);
    // 如果属于寄存器内颜色
    else if (reg_assign.count(addr_color)) ret_reg = std::string("r") + to_string(reg_assign[addr_color]);
    // 如果属于溢出颜色
    else if (frame.spill.count(addr_color)) {
      ret_reg = tmp_regname;
      ret.push_back(string("\tldr\t") + tmp_regname + ", [sp, #" + to_string(4*(frame.spill_offset()+frame.spill[addr_color])) + "]");
    }
    // 如果属于局部数组
    // 那么就将局部数组地址存入count
    else if (frame.local_arr.count(addr_val)) {
      ret_reg = tmp_regname;
      int arr_offset = 4 * (frame.local_arr_offset() + frame.local_arr[addr_val]);
      ret.push_back(string("\tadd\t") + tmp_regname + ", sp, #" + to_string(arr_offset));
    }
  }
  else if (ir_addr->kind == IR::Addr::Kind::PARAM) {
    // 如果属于在寄存器内的参数
    if (addr_color != color_graph::none_color()) {
      ret_reg = string("r") + to_string(addr_color-1);
    }
    // 如果属于栈传递的参数
    else {
      ret_reg = tmp_regname;
      int param_offset = 4 * (frame.frame_offset() + addr_val - 4);
      ret.push_back(string("\tldr\t") + tmp_regname + ", [sp, #" + to_string(param_offset) + "]");
    }
  }
  // ir_armify保证全局变量一定是单一变量
  else if (ir_addr->kind == IR::Addr::Kind::NAMED_LABEL) {
    ret_reg = tmp_regname;
    // 先获取全局变量信息
    auto vartab_ent = context.vartab_cur->get(ir_addr->name);
    // 先移动地址
    ret.push_back(string("\tldr\t") + tmp_regname + ", =" + ir_addr->name);
    // 如果是全局变量，则需要移动值
    if (vartab_ent==nullptr || !(vartab_ent->is_array())) 
      ret.push_back(string("\tldr\t") + tmp_regname + ", [" + tmp_regname + "]");
  }
  // 立即数
  else if (ir_addr->kind == IR::Addr::Kind::IMM) {
    ret_reg = tmp_regname;
    ret.push_back(string("\tmov\t") + tmp_regname + ", #" + std::to_string(ir_addr->val));
  }

  return std::make_pair(ret_reg, ret);
}

// 生成将寄存器内容移动到VAR型变量的语句
std::pair<string, std::list<string> >
store_from_reg(FrameInfo &frame, IR::Addr::Ptr ir_addr, std::string result_regname, std::string idle_regname) {

  auto & reg_assign = frame.reg_assign;

  color addr_color = ir_addr->get_color();
  int   addr_val   = ir_addr->val;
  std::list<string> ret;
  string ret_reg;

  if (ir_addr->kind == IR::Addr::Kind::VAR) {
    // 如果属于预着色
    if (addr_color <= 4 && addr_color > 0) ret_reg = std::string("r") + to_string(addr_color-1);
    // 如果属于寄存器内颜色
    else if (reg_assign.count(addr_color)) ret_reg = std::string("r") + to_string(reg_assign[addr_color]);
    // 如果属于溢出颜色
    else if (frame.spill.count(addr_color)) {
      ret_reg = result_regname;
      ret.push_back(string("\tstr\t") + result_regname + ", [sp, #" + to_string(4*(frame.spill_offset()+frame.spill[addr_color])) + "]");
    }
    // 如果属于局部数组
    else if (frame.local_arr.count(addr_val)) {
      ret_reg = result_regname;
      ret.push_back(string("\tadd\t") + result_regname + ", sp, #" + to_string(4*(frame.local_arr_offset() + frame.local_arr[addr_val])));
    }
  }
  else if (ir_addr->kind == IR::Addr::Kind::PARAM) {
    // 如果属于在寄存器内的参数
    if (addr_color != color_graph::none_color()) {
      ret_reg = string("r") + to_string(addr_color-1);
    }
    // 如果属于栈传递的参数
    else {
      ret_reg = result_regname;
      int param_offset = 4 * (frame.frame_offset() + addr_val - 4);
      ret.push_back(string("\tstr\t") + result_regname + ", [sp, #" + to_string(param_offset));
    }
  }
  // ir_armify保证全局变量一定是变量型的
  else if (ir_addr->kind == IR::Addr::Kind::NAMED_LABEL) {
    ret_reg = result_regname;
    ret.push_back(string("\tldr\t") + idle_regname + ", =" + ir_addr->name);
    ret.push_back(string("\tstr\t") + result_regname + ", [" + idle_regname + "]");
  }
  // 立即数
  else if (ir_addr->kind == IR::Addr::Kind::IMM) {
    ret_reg = result_regname;
    ret.push_back(string("\tmov\t") + result_regname + ", #" + std::to_string(ir_addr->val));
  }

  return std::make_pair(ret_reg, ret);
}

std::list<string>
translate(FrameInfo &frame, IR::Ptr ir) {
  std::list<std::string> ret;

  // 上次是否处理了ret
  // 如果处理到funcend，并且上次没有ret，那么就需要手动插入返回语句
  static bool last_ret = false;

  if (ir->is_al()) {
    auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r10"));
    auto [a2_reg, a2_arm] = move_to_reg(frame, ir->a2, string("fp"));
    auto [a0_reg, a0_arm] = store_from_reg(frame, ir->a0, string("fp"), string("r10"));
    // 将a1与a2取入寄存器
    ret.splice(ret.end(), a1_arm);
    ret.splice(ret.end(), a2_arm);
    // 计算a0值
    ret.push_back(get_arm_opcode(ir->op_) + "\t" + a0_reg + ", " + a1_reg + ", " + a2_reg);
    // 将a0存入分配到的地址中
    ret.splice(ret.end(), a0_arm);
  }
  // 特殊处理MOV
  else if (ir->op_ == IR::Op::MOV) {
    if (ir->a0->get_color() == ir->a1->get_color() && ir->a0->get_color() != color_graph::none_color()) goto end;
    
    auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r10"));
    auto [a0_reg, a0_arm] = store_from_reg(frame, ir->a0, string("r10"), string("fp"));
    // 将a1取入寄存器
    ret.splice(ret.end(), a1_arm);
    // 移动
    ret.push_back(get_arm_opcode(ir->op_) + "\t" + a0_reg + ", " + a1_reg);
    // 将a0存入相应位置
    ret.splice(ret.end(), a0_arm);
  }
  else if (ir->is_mov()) {
    auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r10"));
    auto [a0_reg, a0_arm] = store_from_reg(frame, ir->a0, string("r10"), string("fp"));
    // 将a1取入寄存器
    ret.splice(ret.end(), a1_arm);
    // 移动
    ret.push_back(get_arm_opcode(ir->op_) + "\t" + a0_reg + ", " + a1_reg);
    // 将a0存入相应位置
    ret.splice(ret.end(), a0_arm);
  }
  else if (ir->op_ == IR::Op::CMP) {
    auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r10"));
    auto [a2_reg, a2_arm] = move_to_reg(frame, ir->a2, string("fp"));
    // 将a1与a2取入寄存器
    ret.splice(ret.end(), a1_arm);
    ret.splice(ret.end(), a2_arm);
    ret.push_back(string("\tcmp") + "\t" + a1_reg + ", " + a2_reg);
  }
  else if (ir->op_ == IR::Op::LABEL) {
    ret.push_back(string(".L") + to_string(ir->a0->val) + ":");
  }
  else if (ir->is_jmp()) {
    ret.push_back(get_arm_opcode(ir->op_) + "\t.L" + to_string(ir->a0->val));
  }
  else if (ir->op_ == IR::Op::PARAM) {
    // 前四个参数
    if (ir->a0->get_color() != color_graph::none_color()) {
      auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r10"));
      // 将a1取入寄存器
      ret.splice(ret.end(), a1_arm);
      // 移动
      ret.push_back(string("\tmov\t") + "r" + to_string(ir->a0->get_color()-1) + ", " + a1_reg);
    }
    // 后面的参数：压栈
    else {
      auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r10"));
      int offset = 4*(ir->a0->val-4);
      ret.push_back(string("\tstr\t") + a1_reg + ", [sp, #" + to_string(offset) + "]");
    }
  }
  else if (ir->op_ == IR::Op::CALL) {
    ret.push_back(string("\tbl\t") + ir->a0->name);
  }
  else if (ir->op_ == IR::Op::RET) {
    auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r0"));
    // 将a1取入寄存器
    ret.splice(ret.end(), a1_arm);
    // 移动
    ret.push_back(string("\tmov\tr0, ") + a1_reg);

    // 返回
    ret.splice(ret.end(), frame.ret_statements());
  }
  else if (ir->op_ == IR::Op::FUNCEND) {
    if (!last_ret) ret.splice(ret.end(), frame.ret_statements());
  }
  // LOAD
  else if (ir->op_ == IR::Op::LOAD) {
    auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("fp"));
    auto [a2_reg, a2_arm] = move_to_reg(frame, ir->a2, string("r10"));
    auto [a0_reg, a0_arm] = store_from_reg(frame, ir->a0, string("r10"), string("fp"));
    // 将a1与a2取入寄存器
    ret.splice(ret.end(), a1_arm);
    ret.splice(ret.end(), a2_arm);
    // 执行变址取数
    ret.push_back(string("\tldr\t") + a0_reg + ", [" + a1_reg + ", " + a2_reg + ", lsl #2]");
    // 存储a0
    ret.splice(ret.end(), a0_arm);
  }
  // STORE
  else if (ir->op_ == IR::Op::STORE) {
    auto [a0_reg, a0_arm] = move_to_reg(frame, ir->a0, string("fp"));
    auto [a1_reg, a1_arm] = move_to_reg(frame, ir->a1, string("r10"));
    // a2一定是VAR类型变量，不会用到fp
    auto [a2_reg, a2_arm] = store_from_reg(frame, ir->a2, string("r10"), string("fp"));
    // 将a0与a1取入寄存器
    ret.splice(ret.end(), a0_arm);
    ret.splice(ret.end(), a1_arm);
    // 计算变址
    ret.push_back(string("\tadd\t") + a0_reg + ", " + a0_reg + ", " + a1_reg + ", lsl #2");
    // 将a2取入寄存器，并检查是否需要a9
    ret.splice(ret.end(), a2_arm);
    // 执行变址存数
    ret.push_back(string("\tstr\t") + a2_reg + ", [" + a0_reg + "]");
  }

  end:
  if (ir->op_ == IR::Op::RET) last_ret = true;
  else                        last_ret = false;
  return ret;
}

};  // helper

std::list<string>
translate_function(IR::List &l) {
  std::list<string> ret;

  // register allocate and assign
  register_allocate(l);
  auto color_assign = register_assign(l);

  // stack space calculation
  FrameInfo frame = FrameInfo::cal_frameinfo(l, color_assign);

  // generate function ir code
  string name = l.front()->a0->name;
  ret.push_back(string(".text"));
  ret.push_back(string(".global\t") + name);
  ret.push_back(string(".type\t") + name + ", %function");
  ret.push_back(string(".arm"));
  ret.push_back(name + ":");

  ret.splice(ret.end(), frame.init_statements());

  // actual translate
  for (auto ir: l) {
    ret.splice(ret.end(), translate(frame, ir));
  }

  return ret;
}