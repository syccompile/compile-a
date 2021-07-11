#pragma once
#include <list>
#include <string>
#include <unordered_map>
#include <memory>

#include "types.h"
#include "vartab.h"
#include "../ir_addr.h"

class FuncTabEntry {
public:
  // 函数名
  std::string name;

  // 函数label
  IR_Addr::Ptr label;

  // 参数表
  std::list<Type> param_list;

  FuncTabEntry(std::string _name, std::list<Type> _param_list) : name(_name), param_list(_param_list) {}
};

class FuncTab {
private:
  using EntPtr = std::shared_ptr<FuncTabEntry>;
  using EntPtr_const = std::shared_ptr<const FuncTabEntry>;

  std::unordered_map<std::string, EntPtr> functab;
public:
  void put(std::string name, std::list<Type> param_types);
  EntPtr get(std::string name);
};
