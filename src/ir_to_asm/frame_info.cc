#include "frame_info.h"
#include <unordered_map>

FrameInfo::FrameInfo()
  : param_size(0), local_arr_size(0), spill_size(0), register_save_size(0), reg_assign(), ret_stmt_buf() { }

FrameInfo
FrameInfo::cal_frameinfo(IR::List &l, std::unordered_map<int, int> reg_assign) {
  FrameInfo ret;

  ret.reg_assign = reg_assign;

  // 计算函数顶部需要的保存寄存器的空间
  // 需要用到的变量和r10, r11, lr
  ret.register_save_size = reg_assign.size() + 3;

  // 计算溢出颜色所需的空间和对应偏移
  auto normal_addr_proc = [&reg_assign, &ret](IR::Addr::Ptr ir_addr) {
    if (ir_addr != nullptr && ir_addr->get_color() != color_graph::none_color()) {
      auto color = ir_addr->get_color();
      if (color <= 4) ;
      else if (reg_assign.count(ir_addr->get_color())) ;
      else if (ret.spill.count(ir_addr->get_color())) ;
      else {
        ret.spill[ir_addr->get_color()] = ret.spill_size;
        ret.spill_size += 1;
      }
    }
  };

  // 计算局部数组与函数传参所需空间与偏移
  for (auto &ir: l) {
    if (ir->op_ == IR::Op::ALLOC_IN_STACK) {
      ret.local_arr_size += ir->a1->val;
      ret.local_arr[ir->a0->val] = ret.local_arr_size - 1;
    }
    else if (ir->op_ == IR::Op::PARAM) {
      ret.param_size = std::max(ret.param_size, ir->a0->val-3);
    }
    else {
      normal_addr_proc(ir->a0);
      normal_addr_proc(ir->a1);
      normal_addr_proc(ir->a2);
    }
  }

  return ret;
}

int
FrameInfo::spill_offset() const {
  return param_size + local_arr_size;
}

int
FrameInfo::local_arr_offset() const {
  return param_size;
}

int
FrameInfo::param_offset() const {
  return 0;
}

int
FrameInfo::frame_offset() const {
  return param_size + local_arr_size + spill_size + register_save_size;
}

std::list<std::string>
FrameInfo::init_statements() {
  std::list<std::string> ret;

  // 保留寄存器
  
  std::string need_to_save = "r10, fp, lr";

  if (reg_assign.size() == 1) need_to_save = string("r4") + ", " + need_to_save;
  else if (reg_assign.size()) need_to_save = std::string("r4-r") + std::to_string(3 + reg_assign.size()) + ", " + need_to_save;
  
  ret.push_back(std::string("\tpush\t{") + need_to_save + "}");

  // 分配栈帧
  
  ret.push_back(string("\tsub\tsp, sp, #") + to_string(4*(frame_offset()-register_save_size)));
  
  return ret;
}

std::list<std::string>
FrameInfo::ret_statements() {
  if (ret_stmt_buf.size()) return ret_stmt_buf;

  std::list<std::string> ret;

  // 还原栈帧

  ret.push_back(string("\tadd\tsp, sp, #") + to_string(4*(frame_offset()-register_save_size)));

  // 恢复寄存器

  std::string need_to_restore = "r10, fp, pc";
  if (reg_assign.size() == 1) need_to_restore = string("r4") + ", " + need_to_restore;
  else if (reg_assign.size()) need_to_restore = std::string("r4-r") + std::to_string(3 + reg_assign.size()) + ", " + need_to_restore;

  ret.push_back(std::string("\tpop\t{") + need_to_restore + "}");

  return ret_stmt_buf = ret;
}
