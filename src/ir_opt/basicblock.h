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

bool is_mov_op(IR::Op op);
bool is_algo_op(IR::Op op);

bool operator==(const IR::Addr &lhs, const IR::Addr &rhs);
bool operator<(const IR::Addr &lhs, const IR::Addr &rhs);
inline bool is_var_or_param(const IR::Addr::Ptr &a);

// 用于可用表达式分析的“表达式”类
class Exp {
 public:
  using Ptr = std::shared_ptr<Exp>;
  IR::Op op_;
  IR::Addr::Ptr a0_;
  IR::Addr::Ptr a1_;
  Exp(IR::Op op, const IR::Addr::Ptr &a0, const IR::Addr::Ptr &a1) : op_(op), a0_(a0), a1_(a1) {}
  bool operator==(const Exp &rhs) const;
  bool related_to(const IR::Addr::Ptr &a) const;  // 当前表达式用到了a
  bool be_used_by(const IR::Ptr &ir) const; // ir中用到了当前表达式
  bool copy_be_used_by(const IR::Addr::Ptr &a) const;
  bool copy_related_to(const IR::Addr::Ptr &a) const;
  bool operator<(const Exp &exp) const;
  friend std::ostream &operator<<(std::ostream &os, const Exp &exp);
};

static int cur_num = 100;
inline int alloc_num();
inline IR::Addr::Ptr alloc_var();
inline IR::Ptr make_tmp_assign_exp_ir(const Exp &exp);

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
  std::vector<bool> searched_; // _find_sources的辅助表，记录block_num是否被搜索过
  std::map<int, BasicBlock::Ptr> blocknum_block_map_;  // 从blocknum到block的映射

  void _build_lineno_ir_map();  // 建立lineno到ir的映射表，并更新basic_block的block_num和first_lineno,last_lineno等信息
  void _build_blocknum_block_map();

  void _build_gen_kill_map();
  void _add_to_gen_kill_help_map(const IR::Ptr &ir, int lineno);
  void _add_to_gen_map(const IR::Ptr &ir, int lineno);
  void _add_to_kill_map(const IR::Ptr &ir, int lineno);
  void _calc_gen_kill();
  void _calc_reach_define_IN_OUT();

  void _fill_all_exp_list();
  void _calc_egen_ekill();
  void _calc_available_expression_IN_OUT();
  using source = std::pair<std::shared_ptr<BasicBlock>, BasicBlock::iterator>;
  void _real_find_sources(const Exp &exp,
                          const std::shared_ptr<BasicBlock> &cur_block,
                          std::list<source> &sources);
  std::list<source> _find_sources(const Exp &exp, const std::shared_ptr<BasicBlock> &cur_block);

  void _calc_use_def();
  void _calc_live_variable_IN_OUT();

  void _calc_dominate_IN_OUT();
  using edge = std::pair<int, int>;
  std::set<edge> back_edges_;
  void _find_back_edges();
  using loop = std::vector<int>;
  loop _get_loop(const edge& e);
  std::vector<loop> loops_;
  void _get_all_loops();

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
  void delete_global_common_expression(); // 请先调用delete_local_common_expression
  void constant_folding();
  void algebraic_simplification();
  void local_copy_propagation();
  void global_copy_propagation();
  void remove_dead_code();
  void loop_invariant_code_motion();
  iterator begin() { return basic_block_list_.begin(); }
  iterator end() { return basic_block_list_.end(); }
  std::list<IR::Ptr> merge();
  void debug();
};

class Module {
 public:
  using Ptr = std::shared_ptr<Module>;
  using iterator = std::list<Function::Ptr>::iterator;
//  using const_iterator = std::list<Function::Ptr>::const_iterator;

  std::list<Function::Ptr> function_list_;

  explicit Module(std::list<IR::Ptr> &ir_list);
  explicit Module(std::list<std::list<IR::Ptr>> &ir_lists);
  std::list<std::string> translate_to_arm();
  void reach_define_analysis();
  void live_variable_analysis();
  void available_expression_analysis();
  void delete_local_common_expression();
  void delete_global_common_expression();
  void constant_folding();
  void algebraic_simplification();
  void local_copy_propagation();
  void global_copy_propagation();
  void remove_dead_code();
  iterator begin() { return function_list_.begin(); }
  iterator end() { return function_list_.end(); }
//  const_iterator cbegin() const { return function_list_.cbegin(); }
//  const_iterator cend() const { return function_list_.cend(); }
  void optimize(int optimize_level);
  std::list<std::list<IR::Ptr>> merge();
  void debug();
};

#endif //COMPILER_SRC_IR_OPT_BASICBLOCK_HPP_
