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
    /**
     * 若为变量，access_应为MEM或REG类型，
     * 若为函数，access_应为LABEL类型
     */
    FrameAccess access_;
  };
  SymbolTable(): parent_(nullptr) {}
  ~SymbolTable() {}
  void set_parent(Ptr parent);
  void push_variable(Variable *var);
  void push_function(FunctionDecl *func);
  SymTabEntry find();
private:

  vector<SymTabEntry> entries_;

  /** 上一级符号表 **/
  Ptr parent_;
};

/** 全局符号表 **/
extern std::shared_ptr<SymbolTable> GlobSymTab;
