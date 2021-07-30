#ifndef COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_
#define COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_

#include "../ir.h"

#include <memory>
#include <list>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <utility>

bool is_mov_op(IR::Op op);
bool is_algo_op(IR::Op op);
bool is_swappable_op(IR::Op op);

bool operator==(const IR::Addr &lhs, const IR::Addr &rhs);
bool operator<(const IR::Addr &lhs, const IR::Addr &rhs);
std::ostream &operator<<(std::ostream &os, IR::Addr a);
inline bool is_var_or_param(const IR::Addr::Ptr &a);

// 用于可用表达式分析的“表达式”类
class Exp {
 public:
  using Ptr = std::shared_ptr<Exp>;
  IR::Op op_;
  IR::Addr::Ptr a0_;
  IR::Addr::Ptr a1_;
  Exp(IR::Op op, IR::Addr::Ptr a0, IR::Addr::Ptr a1) : op_(op), a0_(std::move(a0)), a1_(std::move(a1)) {}
  bool operator==(const Exp &rhs) const;
  bool related_to(const IR::Addr::Ptr &a) const;  // 当前表达式用到了a
  bool be_used_by(const IR::Ptr &ir) const; // ir中用到了当前表达式
  bool copy_be_used_by(const IR::Addr::Ptr &a) const;
  bool copy_related_to(const IR::Addr::Ptr &a) const;
  bool operator<(const Exp &exp) const;
  friend std::ostream &operator<<(std::ostream &os, const Exp &exp);
};

inline Exp make_algo_exp(const IR::Ptr &ir) {
  return Exp(ir->op_, ir->a1, ir->a2);
}

inline Exp make_mov_exp(const IR::Ptr &ir) {
  return Exp(ir->op_, ir->a0, ir->a1);
}

static int cur_num = 100;
inline int alloc_num() { return ++cur_num; }
inline IR::Addr::Ptr alloc_var() { return std::make_shared<IR::Addr>(IR::Addr::Kind::VAR, alloc_num()); }
inline IR::Addr::Ptr make_imm(int val) { return std::make_shared<IR::Addr>(IR::Addr::Kind::IMM, val); }
inline IR::Addr::Ptr make_var(int val) { return std::make_shared<IR::Addr>(IR::Addr::Kind::VAR, val); }
inline IR::Addr::Ptr make_param(int val) { return std::make_shared<IR::Addr>(IR::Addr::Kind::PARAM, val); }
inline IR::Ptr make_tmp_assign_exp_ir(const Exp &exp) {
  return std::make_shared<IR>(exp.op_, alloc_var(), exp.a0_, exp.a1_);
}

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

  std::list<int> dominate_IN_;  // 支配结点
  std::list<int> dominate_OUT_;

  explicit BasicBlock(const std::list<IR::Ptr> &ir_list)
      : ir_list_(ir_list) {}
  std::list<std::string> translate_to_arm();  // 不确定是否需要
  void delete_local_common_expression();
  void constant_folding();
  void algebraic_simplification();
  void local_copy_propagation(std::set<Exp> &available_copy_exps);
  void remove_dead_code();
  void ir_specify_optimization(); // 针对IR的特定优化
  iterator begin() { return ir_list_.begin(); }
  iterator end() { return ir_list_.end(); }
  reverse_iterator rbegin() { return ir_list_.rbegin(); }
  reverse_iterator rend() { return ir_list_.rend(); }
  void debug();
};

inline std::shared_ptr<BasicBlock> make_basic_block(const std::list<IR::Ptr> &ir_list) {
  return std::make_shared<BasicBlock>(ir_list);
}

inline std::shared_ptr<BasicBlock> make_empty_basic_block() {
  return make_basic_block(std::list<IR::Ptr>());
}

#endif //COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_
