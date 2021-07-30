#include "module.h"

Module::Module(std::list<IR::Ptr> &ir_list) {
  while (!ir_list.empty()) {
    while (!ir_list.empty() && (ir_list.front()->op_ != IR::Op::FUNCDEF)) {
      ir_list.pop_front();  // 忽略全局声明语句
    }
    if (ir_list.empty()) return;

    auto iter = ir_list.begin();
    while ((*iter)->op_ != IR::Op::FUNCEND) ++iter;
    std::list<IR::Ptr> function_ir_code;
    function_ir_code.splice(function_ir_code.end(),
                            ir_list,
                            ir_list.begin(), iter);
    function_list_.push_back(make_function_block(function_ir_code));
  }
}

Module::Module(list<std::list<IR::Ptr>> &ir_lists) {
  for (auto &func_ir_list : ir_lists) {
    func_ir_list.pop_back();  // POP FUNCEND
    function_list_.push_back(make_function_block(func_ir_list));
  }
}

std::list<std::string> Module::translate_to_arm() {
  std::list<std::string> ret;
  for (const auto &function : function_list_) {
    ret.splice(ret.end(), function->translate_to_arm());
  }
  return ret;
}
void Module::debug() {
//  testExp();
  for (auto &function : function_list_) {
    function->debug();
  }
}
void Module::reach_define_analysis() {
  for (auto &function: function_list_) {
    function->reach_define_analysis();
  }
}
void Module::available_expression_analysis() {
  for (auto &function: function_list_) {
    function->available_expression_analysis();
  }
}
void Module::live_variable_analysis() {
  for (auto &function: function_list_) {
    function->live_variable_analysis();
  }
}
void Module::delete_local_common_expression() {
  for (auto &function: function_list_) {
    function->delete_local_common_expression();
  }
}
void Module::delete_global_common_expression() {
  for (auto &function: function_list_) {
    function->delete_global_common_expression();
  }
}
void Module::constant_folding() {
  for (auto &function: function_list_) {
    function->constant_folding();
  }
}
void Module::algebraic_simplification() {
  for (auto &function: function_list_) {
    function->algebraic_simplification();
  }
}
void Module::local_copy_propagation() {
  for (auto &function: function_list_) {
    function->local_copy_propagation();
  }
}
void Module::global_copy_propagation() {
  for (auto &function: function_list_) {
    function->global_copy_propagation();
  }
}
void Module::remove_dead_code() {
  for (auto &function: function_list_) {
    function->remove_dead_code();
  }
}
std::list<std::list<IR::Ptr>> Module::merge() {
  std::list<std::list<IR::Ptr>> ret;
  for (auto &function: function_list_) {
    ret.push_back(function->merge());
  }
  return ret;
}
void Module::optimize(int optimize_level) {
  if (optimize_level == 0) return;  // no optimization
  if (optimize_level >= 1) {
    for (int i = 0; i < 2; ++i) {
      constant_folding();
      algebraic_simplification();
      delete_local_common_expression();
      delete_global_common_expression();
      local_copy_propagation();
      global_copy_propagation();
      remove_dead_code();
    }
    return;
  }
}
