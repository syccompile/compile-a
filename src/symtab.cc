#include "ast.h"
#include "symtab.h"

#include <memory>
#include <cassert>

/** 全局符号表 **/
auto GlobSymTab = std::make_shared<SymbolTable>();

FrameAccess SymbolTable::push_variable(Variable *var) {
  assert(var);
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::VARIABLE;
  entry.name_ = var->name();
  entry.pointer_.var_ptr = var;
  entry.access_ = frame_->newVarAccess();
  entries_.push_back(entry);
  return entry.access_;
}

FrameAccess SymbolTable::push_function(FunctionDecl *func) {
  assert(func);
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::FUNCTION;
  entry.name_ = func->name();
  entry.pointer_.func_ptr = func;
  entry.access_ = frame_->newLabelAccess();
  entries_.push_back(entry);
  return entry.access_;
}


void SymbolTable::set_parent(SymbolTable::Ptr parent) {
  parent_ = parent; 
}
void SymbolTable::set_frame(Frame::Ptr frame) {
  frame_ = frame;
}
#include <iostream>
SymbolTable::SymTabEntry SymbolTable::find(std::string str) {
  std::cout << "finding " << str << std::endl;
  for (SymTabEntry entry : entries_) {
    if (entry.name_ == str) {
      return entry;
    }
  }
  // FIX
  std::cout << "Can't found " << str << std::endl;
  exit(1);
  return SymTabEntry();
}

FrameAccess SymbolTable::push_return() {
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::RETURN;
entry.name_ = "return";
  entry.access_ = frame_->newTempAccess();
  entry.pointer_.var_ptr = nullptr;
  entries_.push_back(entry);
  return entry.access_;
}
SymbolTable::SymTabEntry SymbolTable::get_return() {
return find("return");
}
FrameAccess SymbolTable::push_param(FParam *param) {
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::PARAM;
  entry.name_ = param->name_;
  entry.access_ = frame_->newTempAccess();
  entry.pointer_.param_ptr = param;
  entries_.push_back(entry);
  return entry.access_;
}
