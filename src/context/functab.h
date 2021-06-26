#pragma once
#include <list>
#include <string>
#include <unordered_map>
#include <memory>

#include "types.h"

class FuncTabEntry {
public:
  // 函数名
  std::string name;

  // 参数表
  std::list<Type> param_types;
};

class FuncTab {
private:
  using EntPtr = std::shared_ptr<FuncTabEntry>;
  using EntPtr_const = std::shared_ptr<const FuncTabEntry>;

  std::unordered_map<std::string, EntPtr> functab;
public:
  void put(std::string name, std::list<Type> param_types);
  EntPtr_const get(std::string name);
};
