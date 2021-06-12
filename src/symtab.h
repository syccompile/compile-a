#pragma once

#include "frame.h"

#include <string>
#include <memory>
#include <vector>

using std::string;
using std::vector;

/* Forward declartion */
class Variable;
class FunctionDecl;

class SymbolTable {
public:
  using Ptr = std::shared_ptr<SymbolTable>;
  class SymTabEntry {
  public:
    friend class SymbolTable;
    enum class SymType { VARIABLE, FUNCTION };
    ~SymTabEntry() {}

  private:
    SymTabEntry() {}
    SymType type_;
    string name_;
    union {
      Variable *var_ptr;
      FunctionDecl *func_ptr;
    } pointer_;
  };
  SymbolTable() {}
  ~SymbolTable() {}
  static Ptr &&newSymTab();
  void set_parent(Ptr parent);
  void push_back_variable(Variable *var);
  void push_back_function(FunctionDecl *func);

private:
  /** 禁止直接使用构造函数构造 **/

  vector<SymTabEntry> entries_;
  /** 上一级符号表 **/
  Ptr parent_;
  Frame::Ptr frame_;
};

/** 全局符号表 **/
extern std::shared_ptr<SymbolTable> GlobSymTab;
