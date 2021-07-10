#ifndef COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_
#define COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_

#include "../ir.h"

#include <memory>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <algorithm>


bool is_mov_op(IR::Op op);
bool is_algo_op(IR::Op op);

// 用于可用表达式分析的“表达式”类
class Exp {
 public:
  using Ptr = std::shared_ptr<Exp>;
  IR::Op op_;
  IR::Addr::Ptr a0_;
  IR::Addr::Ptr a1_;
  Exp(IR::Op op, const IR::Addr::Ptr &a0, const IR::Addr::Ptr &a1) : op_(op), a0_(a0), a1_(a1) {}
//  explicit Exp(const IR::Ptr &ir) : op_(ir->op_), a0_(ir->a1), a1_(ir->a2) {}
  bool operator==(const Exp &rhs) const {
//    if (a1_ != nullptr) {
      return op_ == rhs.op_ &&
          a0_->kind == rhs.a0_->kind && a0_->val == rhs.a0_->val &&
          a1_->kind == rhs.a1_->kind && a1_->val == rhs.a1_->val;
//    } else {
//      return op_ == rhs.op_ &&
//          a0_->kind == rhs.a0_->kind && a0_->val == rhs.a0_->val;
//    }
  }
  bool related_to(const IR::Addr::Ptr &a) const {
//    if (a1_ != nullptr) {
    if (is_algo_op(op_)) {
      return (a0_->kind == a->kind && a0_->val == a->val) ||
          (a1_->kind == a->kind && a1_->val == a->val);
    } else {  // MOV
      return a1_->kind == a->kind && a1_->val == a->val;
    }
//    } else {
//      return (a0_->kind == a->kind && a0_->val == a->val);
//    }
  }
  bool operator<(const Exp &exp) const {
    if (op_ != exp.op_) return op_ < exp.op_;
    if (a0_->kind != exp.a0_->kind) return a0_->kind < exp.a0_->kind;
    if (a0_->val != exp.a0_->val) return a0_->val < exp.a0_->val;
//    if (a1_ != nullptr) {
      if (a1_->kind != exp.a1_->kind) return a1_->kind < exp.a1_->kind;
      return a0_->val < exp.a0_->val;
//    } else {
//      return a0_->val < exp.a0_->val;
//    }
  }
  friend std::ostream& operator<<(std::ostream& os, const Exp &exp) {
    exp.a0_->internal_print();
    switch (exp.op_) {
      case IR::Op::ADD: os << "+"; break;
      case IR::Op::SUB: os << "-"; break;
      case IR::Op::MUL: os << "*"; break;
      case IR::Op::DIV: os << "/"; break;
      case IR::Op::MOD: os << "%"; break;
      default: os << "=";
    }
//    if (exp.a1_ != nullptr) {
      exp.a1_->internal_print();
//    }
    return os;
  }
};

class BasicBlock {
 private:
 public:
  using Ptr = std::shared_ptr<BasicBlock>;
  using Ptr_weak = std::weak_ptr<BasicBlock>;
  using iterator = std::list<IR::Ptr>::iterator;
  using reverse_iterator = std::list<IR::Ptr>::reverse_iterator;
  int first_lineno_;
  int last_lineno_;
  int block_num_;
  std::list<IR::Ptr> ir_list_;
  std::list<Ptr_weak> predecessor_list_;  // 前驱基本块
  std::list<Ptr_weak> successor_list_;    // 后继基本块

  std::list<int> gen_;
  std::list<int> kill_;
  std::list<int> reach_define_IN_;
  std::list<int> reach_define_OUT_;

  std::list<Exp> egen_;
  std::list<Exp> ekill_;
  std::list<Exp> available_expression_IN_;
  std::list<Exp> available_expression_OUT_;
  void calc_egen_ekill(const std::list<Exp> &all_exp_list);

  std::list<IR::Addr> use_;
  std::list<IR::Addr> def_;
  std::list<IR::Addr> live_variable_IN_;
  std::list<IR::Addr> live_variable_OUT_;
  void calc_use_def();

  explicit BasicBlock(const std::list<IR::Ptr> &ir_list)
      : ir_list_(ir_list) {}
  std::list<std::string> translate_to_arm();  // 不确定是否需要
  void delete_local_common_expression();
  iterator begin() { return ir_list_.begin(); }
  iterator end() { return ir_list_.end(); }
  reverse_iterator rbegin() { return ir_list_.rbegin(); }
  reverse_iterator rend() { return ir_list_.rend(); }
  void debug();
};

class Function {
 private:
  std::map<int, int> symbol_table_;
  std::multimap<int, int> gen_kill_help_map_;
  std::map<int, IR::Ptr> lineno_ir_map_; // 从lineno到IR指令的映射
  std::map<int, std::list<int>> gen_map_; // 从lineno到gen的映射
  std::map<int, std::list<int>> kill_map_;  // 从lineno到kill的映射
  std::list<Exp> all_exp_list_;

  void _build_lineno_ir_map();  // 建立lineno到ir的映射表，并更新basic_block的block_num和first_lineno,last_lineno等信息

  void _build_gen_kill_map();
  void _add_to_gen_kill_help_map(const IR::Ptr &ir, int lineno);
  void _add_to_gen_map(const IR::Ptr &ir, int lineno);
  void _add_to_kill_map(const IR::Ptr &ir, int lineno);
  void _calc_gen_kill();
  void _calc_reach_define_IN_OUT();

  void _fill_all_exp_list();
  void _calc_egen_ekill();
  void _calc_available_expression_IN_OUT();

  void _calc_use_def();
  void _calc_live_variable_IN_OUT();
 public:
  using Ptr = std::shared_ptr<Function>;
  using iterator = std::list<BasicBlock::Ptr>::iterator;

  std::list<BasicBlock::Ptr> basic_block_list_;
  std::string func_name_;
  int arg_num_;

  explicit Function(std::list<IR::Ptr> &ir_list);
  std::list<std::string> translate_to_arm();
  void reach_define_analysis();   // 到达定值分析
  void live_variable_analysis();  // 活跃变量分析
  void available_expression_analysis(); // 可用表达式分析
  void delete_local_common_expression();
  void delete_global_common_expression();
  iterator begin() { return basic_block_list_.begin(); }
  iterator end() { return basic_block_list_.end(); }
  void debug();
};

class Module {
 public:
  using Ptr = std::shared_ptr<Module>;
  using iterator = std::list<Function::Ptr>::iterator;
//  using const_iterator = std::list<Function::Ptr>::const_iterator;

  std::list<Function::Ptr> function_list_;

  explicit Module(std::list<IR::Ptr> &ir_list);
  std::list<std::string> translate_to_arm();
  void reach_define_analysis();
  void live_variable_analysis();
  void available_expression_analysis();
  iterator begin() { return function_list_.begin(); }
  iterator end() { return function_list_.end(); }
//  const_iterator cbegin() const { return function_list_.cbegin(); }
//  const_iterator cend() const { return function_list_.cend(); }
  void debug();
};

#endif //COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_
