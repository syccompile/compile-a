#include "symtab.h"
#include "ast.h"

#include <cassert>
#include <memory>

FrameAccess SymbolTable::push(Variable *var) {
  assert(var);
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::VARIABLE;
  entry.name_ = var->name();
  entry.pointer_.var_ptr = var;
  entry.access_ = frame_->newVarAccess(frame_, var->name());
  entries_.push_back(entry);
  return entry.access_;
}

FrameAccess SymbolTable::push(FunctionDecl *func) {
  assert(func);
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::FUNCTION;
  entry.name_ = func->name();
  entry.pointer_.func_ptr = func;
  entry.access_ = frame_->newLabelAccess(frame_, func->name());
  entries_.push_back(entry);
  return entry.access_;
}

void SymbolTable::set_parent(SymbolTable::Ptr parent) { parent_ = parent; }
void SymbolTable::set_frame(Frame::Ptr frame) { frame_ = frame; }
#include <iostream>
SymbolTable::SymTabEntry SymbolTable::find(std::string str) {
  for (SymTabEntry entry : entries_) {
    if (entry.name_ == str) {
      return entry;
    }
  }
  if (parent_) {
    return parent_->find(str);
  }
  // FIX
  std::cout << "Can't found " << str << std::endl;
  exit(1);
  return SymTabEntry();
}

SymbolTable::SymTabEntry SymbolTable::find(Variable *var) {
  for (auto entry : entries_) {
    if (entry.type_ == SymTabEntry::SymType::VARIABLE &&
        entry.pointer_.var_ptr == var) {
      return entry;
    }
  }
  if (parent_) {
    return parent_->find(var);
  }
  // FIX
  std::cout << "Can't found " << var->name() << std::endl;
  exit(1);
  return SymTabEntry();
}

SymbolTable::SymTabEntry SymbolTable::find(FunctionDecl *func) {
  for (auto entry : entries_) {
    if (entry.type_ == SymTabEntry::SymType::FUNCTION &&
        entry.pointer_.func_ptr == func) {
      return entry;
    }
  }
  if (parent_) {
    return parent_->find(func);
  }
  // FIX
  std::cout << "Can't found " << func->name() << std::endl;
  exit(1);
  return SymTabEntry();
}
