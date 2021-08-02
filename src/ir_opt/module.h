#ifndef COMPILER_SRC_IR_OPT_MODULE_HPP_
#define COMPILER_SRC_IR_OPT_MODULE_HPP_

#include "function.h"

class Module {
 public:
  using Ptr = std::shared_ptr<Module>;
  using iterator = std::list<Function::Ptr>::iterator;

  std::list<Function::Ptr> function_list_;

  explicit Module(std::list<IR::Ptr> &ir_list);
  explicit Module(std::list<std::list<IR::Ptr>> &ir_lists);
  std::list<std::string> translate_to_arm();
  iterator begin() { return function_list_.begin(); }
  iterator end() { return function_list_.end(); }
  void make_inline_function();
  void optimize(int optimize_level);
  std::list<std::list<IR::Ptr>> merge();
  void debug();
};

#endif //COMPILER_SRC_IR_OPT_MODULE_HPP_
