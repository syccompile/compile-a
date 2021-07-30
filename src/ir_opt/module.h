#ifndef COMPILER_SRC_IR_OPT_MODULE_HPP_
#define COMPILER_SRC_IR_OPT_MODULE_HPP_

#include "function.h"

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

#endif //COMPILER_SRC_IR_OPT_MODULE_HPP_
