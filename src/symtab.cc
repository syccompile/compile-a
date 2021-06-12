#include "ast.h"
#include "symtab.h"

#include <memory>
#include <cassert>

/** 全局符号表 **/
auto GlobSymTab = std::make_shared<SymbolTable>();

void SymbolTable::push_variable(Variable *var) {
  assert(var);
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::VARIABLE;
  entry.name_ = var->name();
  entry.pointer_.var_ptr = var;
  entries_.push_back(entry);
}

void SymbolTable::push_function(FunctionDecl *func) {
  assert(func);
  SymTabEntry entry;
  entry.type_ = SymTabEntry::SymType::FUNCTION;
  entry.name_ = func->name();
  entry.pointer_.func_ptr = func;
  entries_.push_back(entry);
}


void SymbolTable::set_parent(SymbolTable::Ptr parent) {
  parent_ = parent; 
}

SymbolTable::SymTabEntry SymbolTable::find() {
  // TODO
  SymbolTable::SymTabEntry entry;
  return entry;
}
