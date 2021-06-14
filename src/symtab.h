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

/**
 * 符号表，保存了变量、函数的FrameAccess等信息，
 * 在生成中间代码时，查找符号表找到对应的FrameAccess，
 * 利用FrameAccess生成中间代码
 */
class SymbolTable {
public:
  using Ptr = std::shared_ptr<SymbolTable>;
  class SymTabEntry {
  public:
    friend class SymbolTable;
    enum class SymType { VARIABLE, FUNCTION};
    ~SymTabEntry() {}

    /**
     * @member type_
     * 符号表项目的类型，变量或函数
     */
    SymType type_;
    /**
     * @member name_
     * 符号表项目的名称，一般为变量或函数的名称，
     */
    string name_;
    /** 
     * @member pointer_
     * 变量或函数对应的指针
     */
    union {
      Variable *var_ptr;
      FunctionDecl *func_ptr;
    } pointer_;
    /**
     * @member access_
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

  Frame::Ptr frame() { return frame_; }
  void set_frame(Frame::Ptr frame);

  /**
   * 将变量或函数添加进符号表中
   */
  FrameAccess push(Variable *var);
  FrameAccess push(FunctionDecl *func);

  /**
   * 符号表中查找，若不在当前符号表中，
   * 则向上一级符号表查找
   */
  SymTabEntry find(string str);
  SymTabEntry find(Variable* var);
  SymTabEntry find(FunctionDecl *func);

private:
  /**
   * @member entries_
   * 符号表项目
   */
  vector<SymTabEntry> entries_;

  /** 
   * @member parent_
   * 上一级符号表 
   **/
  Ptr parent_;
  /**
   * @member frame_
   * 所属栈帧
   */
  Frame::Ptr frame_;
};

/** 全局符号表 **/
extern std::shared_ptr<SymbolTable> GlobSymTab;
