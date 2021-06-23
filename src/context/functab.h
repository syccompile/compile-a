#pragma once
#include <list>

#include "vartab.h"

class FuncTabEntry {
public:
  // 函数名
  std::string name;

  // 参数表
  std::list<VarTabEntry> params;
};