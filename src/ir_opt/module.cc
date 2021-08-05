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

void Module::make_inline_function() {
  // TODO
}

std::list<std::list<IR::Ptr>> Module::merge() {
  std::list<std::list<IR::Ptr>> ret;
  for (auto &function: function_list_) {
    std::list<IR::Ptr> tmp = function->merge();
    tmp.push_front(function->header_);
    tmp.push_back(function->footer_);
    ret.push_back(tmp);
  }
  return ret;
}

void Module::optimize(int optimize_level) {
  for (auto &function: function_list_) {
    function->optimize(optimize_level);
  }
}
