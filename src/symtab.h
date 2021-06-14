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
class FParam;

class SymbolTable {
public:
  using Ptr = std::shared_ptr<SymbolTable>;
  class SymTabEntry {
  public:
    friend class SymbolTable;
    enum class SymType { VARIABLE, FUNCTION, RETURN, PARAM};
    ~SymTabEntry() {}

    SymType type_;
    string name_;
    union {
      Variable *var_ptr;
      FunctionDecl *func_ptr;
      FParam *param_ptr;
    } pointer_;
    /**
     * 若为变量，access_应为MEM或REG类型，
     * 若为函数，access_应为LABEL类型
     */
    FrameAccess access_;

  private:
    SymTabEntry() {}
  };
  SymbolTable() : parent_(nullptr), frame_(nullptr) {}
  SymbolTable(SymbolTable::Ptr parent, Frame::Ptr frame) : parent_(parent), frame_(frame) {}
  ~SymbolTable() {}
  void set_parent(Ptr parent);
  void set_frame(Frame::Ptr frame);
  Frame::Ptr frame() { return frame_; }
  FrameAccess push_variable(Variable *var);
  FrameAccess push_function(FunctionDecl *func);
  FrameAccess push_param(FParam* param);
  FrameAccess push_return();
  SymTabEntry find(string str);
  SymTabEntry find(Variable* var);
  SymTabEntry find(FunctionDecl *func);
  SymTabEntry get_return();

private:
  vector<SymTabEntry> entries_;

  /** 上一级符号表 **/
  Ptr parent_;
  Frame::Ptr frame_;
};

/** 全局符号表 **/
extern std::shared_ptr<SymbolTable> GlobSymTab;
