#ifndef COMPILER_SRC_IR_OPT_FUNCTION_HPP_
#define COMPILER_SRC_IR_OPT_FUNCTION_HPP_

#include "basicblock.h"

class Function {
 private:
  map<int, int> symbol_table_;
  multimap<int, int> gen_kill_help_map_;
  map<int, IR::Ptr> lineno_ir_map_; // 从lineno到IR指令的映射
  map<int, list<int>> gen_map_; // 从lineno到gen的映射
  map<int, list<int>> kill_map_;  // 从lineno到kill的映射
  list<Exp> all_exp_list_;
  vector<bool> searched_; // _find_sources的辅助表，记录block_num是否被搜索过
  vector<bool> inst_invariant_vec_;  // 从lineno到指令是否为循环不变量的映射
  vector<IR_Addr::Ptr> lineno_rd_vec_;  // 从lineno到目的操作数的映射
  std::vector<bool> reachable_;  // blocknum到reachable的映射

  void _build_lineno_ir_map();  // 建立lineno到ir的映射表，并更新basic_block的block_num和first_lineno,last_lineno等信息，同时更新ir_num_
  void _update_blocknum();
  void _build_lineno_rd_vec();

  void _build_gen_kill_map();
  void _add_to_gen_kill_help_map(const IR::Ptr &ir, int lineno);
  void _add_to_gen_map(const IR::Ptr &ir, int lineno);
  void _add_to_kill_map(const IR::Ptr &ir, int lineno);
  void _calc_gen_kill();
  void _calc_reach_define_IN_OUT();

  void _fill_all_exp_list();
  void _calc_egen_ekill();
  void _calc_available_expression_IN_OUT();
  using source = pair<shared_ptr<BasicBlock>, BasicBlock::iterator>;
  void _real_find_sources(const Exp &exp,
                          const shared_ptr<BasicBlock> &cur_block,
                          list<source> &sources);
  list<source> _find_sources(const Exp &exp, const shared_ptr<BasicBlock> &cur_block);

  void _calc_use_def();
  void _calc_live_variable_IN_OUT();

  void _calc_dominate_IN_OUT();
  using edge = pair<BasicBlock::Ptr, BasicBlock::Ptr>;
  static bool back_edge_less(const edge &lhs, const edge &rhs) {
    int range1 = lhs.first->block_num_ - lhs.second->block_num_;
    int range2 = rhs.first->block_num_ - rhs.second->block_num_;
    assert(range1 > 0);
    assert(range2 > 0);
    if (range1 != range2) {
      return range1 < range2;
    }
    return lhs.second->block_num_ < rhs.second->block_num_;
  }
  set<edge, decltype(back_edge_less) *> back_edges_;
  void _find_back_edges();
  using loop = vector<BasicBlock::Ptr>;
  static loop _get_loop(const edge &e);
  vector<pair<BasicBlock::Ptr, BasicBlock::iterator>> _mark_loop_invariant(loop &l);

  void _explore(const BasicBlock::Ptr &block);
  void _delete_block(int i);

  void _merge_block(const BasicBlock::Ptr &block1, const BasicBlock::Ptr &block2);

  void _divide_basic_block(list<IR::Ptr> &ir_list);
  void _link_basic_block();
  void _rebuild_basic_block();
 public:
  using Ptr = shared_ptr<Function>;
  using iterator = vector<BasicBlock::Ptr>::iterator;

  vector<BasicBlock::Ptr> basic_block_vector_;
  string func_name_;
  int arg_num_;
  int ir_num_;

  explicit Function(list<IR::Ptr> &ir_list);
  list<string> translate_to_arm();
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
  void ir_specify_optimization(); // 针对IR的特定优化
  void delete_unreachable_code();
  void staighten();
  void if_simplify();
  void tail_merging();
  void label_simplify();
  void strength_reduction();
  void induction_variable_elimination();
  void optimize(int optimize_level);
  iterator begin() { return basic_block_vector_.begin(); }
  iterator end() { return basic_block_vector_.end(); }
  list<IR::Ptr> merge();
  void debug();
};

/* 通过ir_list新建一个FunctionBlock, 返回指向它的shared_ptr */
inline std::shared_ptr<Function>
make_function_block(std::list<IR::Ptr> &ir_list) {
  return std::make_shared<Function>(ir_list);
}

template<typename T>
inline auto len_of_list(const std::list<T> &l) {
  return std::distance(l.begin(), l.end());
}

inline bool erase_pred_succ_list(std::list<BasicBlock::Ptr_weak> &l, const BasicBlock::Ptr &block) {
  auto result = std::find_if(l.begin(), l.end(), [&block](const BasicBlock::Ptr_weak &b) {
    return b.lock()->block_num_ == block->block_num_;
  });
  if (result != l.end()) {
    l.erase(result);
    return true;
  }
  return false;
}

inline void remove_useless_label(std::list<IR::Ptr> &ir_list) {
  // TODO: 不确保正确性，建立在所有相同label的地址相同
  for (auto iter = ir_list.begin(); iter != ir_list.end(); ++iter) {
    auto next_iter = std::next(iter);
    if (next_iter == ir_list.end()) break;
    auto cur_ir = *iter;
    auto next_ir = *next_iter;
    if (cur_ir->op_ == IR::Op::LABEL && next_ir->op_ == IR::Op::LABEL) {
      *(cur_ir->a0) = *(next_ir->a0);
      iter = ir_list.erase(iter);
    }
  }
}

inline void remove_unnecessary_jmp(std::list<IR::Ptr> &ir_list) {
  for (auto iter = ir_list.begin(); iter != ir_list.end(); ++iter) {
    auto next_iter = std::next(iter);
    if (next_iter == ir_list.end()) break;
    auto cur_ir = *iter;
    auto next_ir = *next_iter;
    if (is_jmp_op(cur_ir->op_) && is_jmp_op(next_ir->op_)) {
      assert(next_ir->op_ == IR::Op::JMP);  // 可能会断言失败
      if (*cur_ir->a0 == *next_ir->a0) {  // 要跳转到相同的标号
        iter = ir_list.erase(iter);
      }
    }
  }
}

inline void remove_unnecessary_cmp(std::list<IR::Ptr> &ir_list) {
  // TODO: 优化不一定正确，以下代码建立在CMP之后必须马上使用条件跳转或者条件传送的基础上
  for (auto iter = ir_list.begin(); iter != ir_list.end(); ++iter) {
    auto next_iter = std::next(iter);
    if (next_iter == ir_list.end()) break;
    auto cur_ir = *iter;
    auto next_ir = *next_iter;
    if (cur_ir->op_ == IR::Op::CMP) {
      if ((is_mov_op(next_ir->op_) && next_ir->op_ != IR::Op::MOV) ||
          (is_jmp_op(next_ir->op_) && next_ir->op_ != IR::Op::JMP)) {
        continue;
      }
      iter = ir_list.erase(iter);
    }
  }
}

#endif //COMPILER_SRC_IR_OPT_FUNCTION_HPP_
