#pragma once
#include <map>
#include <memory>

#include "vartab.h"
#include "virtalloc.h"

class Context {
public:
  // 负责分配标号、栈帧
  VirtualAllocator allocator;

  // 当前作用域的符号表
  std::shared_ptr<VarTab> vartab_cur;

  // 开始作用域
  // 建立新的符号表作为当前符号表，将父级符号表放入其指针域
  void new_scope();
  // 结束作用域
  // 从当前符号表的指针域中取出父级符号表，作为当前符号表
  void end_scope();

  Context();
};

Context context;
