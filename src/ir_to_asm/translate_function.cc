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

// 将一个立即数拆分为arm立即数之和
std::list<uint32_t>
split_to_arm_imm(uint32_t imm) {
  std::list<uint32_t> ret;
  
  if (is_arm_imm(imm)) {
    ret.push_back(imm);
  }
  else {
    std::vector<uint32_t> tmp{0xffu, 0xff00u, 0xff0000u, 0xff000000u};
    for (auto &i: tmp) {
      auto result = imm & i;
      if (result!=0u) ret.push_back(result);
    }
  }

  return ret;
}

string
get_arm_opcode(IR::Op op) {

#define CASE(IR_OP, ARM_OP) case IR::Op:: IR_OP : return string(ARM_OP)

  switch(op) {
    CASE(ADD, "\tadd");
    CASE(SUB, "\tsub");
    CASE(MUL, "\tmul");
    CASE(DIV, "\tdiv");
    CASE(SHL, "\tlsl");
    CASE(SHR, "\tasr");
    CASE(AND, "\tand");
    CASE(OR,  "\torr");
    CASE(XOR, "\teor");
    CASE(MOV, "\tmov");
    CASE(MVN, "\tmvn");
    CASE(MOVLE, "\tIT\tLE\n\tmovle");
    CASE(MOVLT, "\tIT\tLT\n\tmovlt");
    CASE(MOVGE, "\tIT\tGE\n\tmovge");
    CASE(MOVGT, "\tIT\tGT\n\tmovgt");
    CASE(MOVEQ, "\tIT\tEQ\n\tmoveq");
    CASE(MOVNE, "\tIT\tNE\n\tmovne");
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
      // 防止爆立即数
      auto spill_offsets = split_to_arm_imm(4 * (frame.spill_offset() + frame.spill[addr_color]));
      if (spill_offsets.size() == 1)
        ret.push_back(string("\tldr\t") + tmp_regname + ", [sp, #" + to_string(spill_offsets.front()) + "]");
      else {
        ret.push_back(string("\tadd\t") + tmp_regname + ", sp, #" + to_string(spill_offsets.front()));
        spill_offsets.pop_front();
        for (auto &i: spill_offsets)
          ret.push_back(string("\tadd\t") + tmp_regname + ", " + tmp_regname + ", #" + to_string(i));
        ret.push_back(string("\tldr\t") + tmp_regname + ", [" + tmp_regname + "]");
      }
    }
    // 如果属于局部数组
    // 那么就将局部数组地址存入count
    else if (frame.local_arr.count(addr_val)) {
      ret_reg = tmp_regname;
      // 防止爆立即数
      auto arr_offsets = split_to_arm_imm(4 * (frame.local_arr_offset() + frame.local_arr[addr_val]));
      ret.push_back(string("\tadd\t") + tmp_regname + ", sp, #" + to_string((arr_offsets.front())));
      arr_offsets.pop_front();
      for (auto &i: arr_offsets)
        ret.push_back(string("\tadd\t") + tmp_regname + ", " + tmp_regname + ", #" + to_string(i));
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
      // 防止爆立即数
      auto param_offsets = split_to_arm_imm(4 * (frame.frame_offset() + addr_val - 4));
      if (param_offsets.size() == 1)
        ret.push_back(string("\tldr\t") + tmp_regname + ", [sp, #" + to_string(param_offsets.front()) + "]");
      else {
        ret.push_back(string("\tadd\t") + tmp_regname + ", sp, #" + to_string(param_offsets.front()));
        param_offsets.pop_front();
        for (auto &i: param_offsets)
          ret.push_back(string("\tadd\t") + tmp_regname + ", " + tmp_regname + ", #" + to_string(i));
        ret.push_back(string("\tldr\t") + tmp_regname + ", [" + tmp_regname + "]");
      }
    }
  }
  // 全局名字有可能是单一变量或数组
  // 通过全局符号表（context.vartab_cur）确定
  else if (ir_addr->kind == IR::Addr::Kind::NAMED_LABEL) {
    ret_reg = tmp_regname;
    // 先获取全局变量信息
    auto vartab_ent = context.vartab_cur->get(ir_addr->name);
    // 先移动地址
    ret.push_back(string("\tmov32\t") + tmp_regname + ", " + ir_addr->name);
    // 如果是全局变量，则需要移动值
    if (vartab_ent==nullptr || !(vartab_ent->is_array())) 
      ret.push_back(string("\tldr\t") + tmp_regname + ", [" + tmp_regname + "]");
  }
  // 立即数
  else if (ir_addr->kind == IR::Addr::Kind::IMM) {
    ret_reg = tmp_regname;
    // 如果ARM可以表示这个立即数
    if (is_arm_imm(ir_addr->val)) {
      ret.push_back(string("\tmov\t") + tmp_regname + ", #" + std::to_string(ir_addr->val));
    }
    // 如果ARM可以表示这个立即数的反值
    else if (is_arm_imm(~(ir_addr->val))) {
      ret.push_back(string("\tmvn\t") + tmp_regname + ", #" + std::to_string(~(ir_addr->val)));
    }
    // 如果都不行，就需要动用mov32
    else {
      ret.push_back(string("\tmov32\t") + tmp_regname + ", #" + std::to_string(ir_addr->val));
    }
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
      // 防止爆立即数
      auto spill_offsets = split_to_arm_imm(4 * (frame.spill_offset() + frame.spill[addr_color]));
      if (spill_offsets.size() == 1)
        ret.push_back(string("\tstr\t") + result_regname + ", [sp, #" + to_string(spill_offsets.front()) + "]");
      else {
        ret.push_back(string("\tadd\t") + idle_regname + ", sp, #" + to_string(spill_offsets.front()));
        spill_offsets.pop_front();
        for (auto &i: spill_offsets)
          ret.push_back(string("\tadd\t") + idle_regname + ", " + idle_regname + ", #" + to_string(i));
        ret.push_back(string("\tstr\t") + result_regname + ", [" + idle_regname + "]");
      }
    }
    // 如果属于局部数组
    else if (frame.local_arr.count(addr_val)) {
      assert(false);
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

      // 防止爆立即数
      auto param_offsets = split_to_arm_imm(4 * (frame.frame_offset() + addr_val - 4));
      if (param_offsets.size() == 1)
        ret.push_back(string("\tstr\t") + result_regname + ", [sp, #" + to_string(param_offsets.front()) + "]");
      else {
        ret.push_back(string("\tadd\t") + idle_regname + ", sp, #" + to_string(param_offsets.front()));
        param_offsets.pop_front();
        for (auto &i: param_offsets)
          ret.push_back(string("\tadd\t") + idle_regname + ", " + idle_regname + ", #" + to_string(i));
        ret.push_back(string("\tstr\t") + result_regname + ", [" + idle_regname + "]");
      }
    }
  }
  // ir_armify保证全局变量一定是变量型的
  else if (ir_addr->kind == IR::Addr::Kind::NAMED_LABEL) {
    ret_reg = result_regname;
    ret.push_back(string("\tmov32\t") + idle_regname + ", " + ir_addr->name);
    ret.push_back(string("\tstr\t") + result_regname + ", [" + idle_regname + "]");
  }
  // 立即数
  else if (ir_addr->kind == IR::Addr::Kind::IMM) {
    ret_reg = result_regname;
    ret.push_back(string("\tmov\t") + result_regname + ", #" + std::to_string(ir_addr->val));
    // 如果ARM可以表示这个立即数
    if (is_arm_imm(ir_addr->val)) {
      ret.push_back(string("\tmov\t") + result_regname + ", #" + std::to_string(ir_addr->val));
    }
    // 如果ARM可以表示这个立即数的反值
    else if (is_arm_imm(~(ir_addr->val))) {
      ret.push_back(string("\tmvn\t") + result_regname + ", #" + std::to_string(~(ir_addr->val)));
    }
    // 如果都不行，就需要动用mov32
    else {
      ret.push_back(string("\tmov32\t") + result_regname + ", #" + std::to_string(ir_addr->val));
    }
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
      // 将a1取入寄存器
      ret.splice(ret.end(), a1_arm);
      // 压栈
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
    auto [a2_reg, a2_arm] = move_to_reg(frame, ir->a2, string("r10"));
    // 将a0与a1取入寄存器
    ret.splice(ret.end(), a0_arm);
    ret.splice(ret.end(), a1_arm);
    // 计算变址
    ret.push_back(string("\tadd\t") + "fp, " + a0_reg + ", " + a1_reg + ", lsl #2");
    // 将a2取入寄存器
    ret.splice(ret.end(), a2_arm);
    // 执行变址存数
    ret.push_back(string("\tstr\t") + a2_reg + ", [fp]");
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
  // allocated already in main.cc
  //register_allocate(l);
  auto color_assign = register_assign(l);

  // stack space calculation
  FrameInfo frame = FrameInfo::cal_frameinfo(l, color_assign);

  // generate function ir code
  string name = l.front()->a0->name;
  ret.push_back(string(".text"));
  ret.push_back(string(".global\t") + name);
  ret.push_back(string(".type\t") + name + ", %function");
  ret.push_back(string(".syntax unified"));
  ret.push_back(string(".thumb"));
  ret.push_back(string(".thumb_func"));

  ret.push_back(name + ":");

  ret.splice(ret.end(), frame.init_statements());

  // actual translate
  for (auto ir: l) {
    ret.splice(ret.end(), translate(frame, ir));
  }

  return ret;
}
