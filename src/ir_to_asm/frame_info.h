#pragma once

#include "../ir.h"
#include <unordered_map>
#include <list>
#include <string>

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
class FrameInfo {
public:
  FrameInfo();

  // calculate frameinfo
  static FrameInfo cal_frameinfo(IR::List &l, std::unordered_map<int, int> reg_assign);

  

  // stack related

  // stack area size  
  int param_size;
  int local_arr_size;
  int spill_size;
  int register_save_size;
  
  // color -> stack_frame
  std::unordered_map<int, int> spill;
  // ir_addr.val -> stack_frame
  std::unordered_map<int, int> local_arr;

  // stack offsets
  int spill_offset() const;
  int local_arr_offset() const;
  int param_offset() const;
  int frame_offset() const;


  
  // register related
  
  // register assign
  // color -> register
  std::unordered_map<int, int> reg_assign;



  // function initialization and return statements
  
  std::list<std::string> init_statements();
  std::list<std::string> ret_statements();

  // avoid redunctant calculations for return statements
  private: std::list<std::string> ret_stmt_buf; public:
};
