#ifndef COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_
#define COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_

#include "../ir.h"

#include <memory>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>

class BasicBlock {
 public:
  using Ptr = std::shared_ptr<BasicBlock>;
  using Ptr_weak = std::weak_ptr<BasicBlock>;
  std::list<IR::Ptr> ir_list_;
  std::list<Ptr_weak> predecessor_list_;  // 前驱基本块
  std::list<Ptr_weak> successor_list_;    // 后继基本块
  std::list<int> gen_;
  std::list<int> kill_;
  std::list<int> reach_define_IN_;
  std::list<int> reach_define_OUT_;

  int first_lineno_;
  int last_lineno_;
  int block_num_;
  explicit BasicBlock(const std::list<IR::Ptr> &ir_list)
      : ir_list_(ir_list) {}
  std::list<std::string> translate_to_arm();  // 不确定是否需要
  void debug();
};

class FunctionBlock {
 private:
  std::map<int, int> symbol_table_;
  std::multimap<int, int> gen_kill_help_map_;
  std::map<int, IR::Ptr> lineno_ir_map_; // 从lineno到IR指令的映射
  std::map<int, std::list<int>> gen_map_; // 从lineno到gen的映射
  std::map<int, std::list<int>> kill_map_;  // 从lineno到kill的映射
//  std::map<int, std::list<int>> define_map_;
//  std::map<int, std::list<int>> use_map_;
  void _build_gen_kill_map();
  void _build_define_map();
  void _build_lineno_ir_map();  // 建立lineno到ir的映射表，并更新basic_block的block_num和first_lineno,last_lineno等信息
  void _add_to_gen_kill_help_map(const IR::Ptr &ir, int lineno);
  void _add_to_gen_map(const IR::Ptr &ir, int lineno);
  void _add_to_kill_map(const IR::Ptr &ir, int lineno);
  void _calc_gen_kill();
  void _calc_reach_define_IN_OUT();
 public:
  std::list<BasicBlock::Ptr> basic_block_list_;
  std::string func_name_;
  int arg_num_;
  using Ptr = std::shared_ptr<FunctionBlock>;
  explicit FunctionBlock(std::list<IR::Ptr> &ir_list);
  std::list<std::string> translate_to_arm();
  void reach_define_analysis();   // 到达定值分析
  void live_variable_analysis();  // 活跃变量分析
  void available_expression_analysis(); // 可用表达式分析
  void debug();
};

class FunctionBlocks {
 public:
  std::list<FunctionBlock::Ptr> function_block_list_;
  using Ptr = std::shared_ptr<FunctionBlocks>;
  explicit FunctionBlocks(std::list<IR::Ptr> &ir_list);
  std::list<std::string> translate_to_arm();
  void reach_define_analysis();
  void live_variable_analysis();
  void available_expression_analysis();
  void debug();
};

#endif //COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_
