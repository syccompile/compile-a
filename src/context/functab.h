#pragma once
#include <vector>
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
  std::vector<IR::Addr::Ptr> param_list;

  FuncTabEntry(std::string _name, std::vector<IR::Addr::Ptr> _param_list) : name(_name), param_list(_param_list) {
    label = IR_Addr::make_named_label(_name);
  }
  IR::Addr::Ptr get_param_addr(int v);
};

class FuncTab {
private:
  using EntPtr = std::shared_ptr<FuncTabEntry>;
  using EntPtr_const = std::shared_ptr<const FuncTabEntry>;

  std::unordered_map<std::string, EntPtr> functab;
  EntPtr curtab;
public:
  void put(std::string name, std::vector<IR::Addr::Ptr> params);
  EntPtr get(std::string name);
  EntPtr get_curtab() const;
};
