#include "functab.h"

void FuncTab::put(std::string name, std::vector<IR::Addr::Ptr> params) {
  auto new_ent = std::make_shared<FuncTabEntry>(name, params);
  this->curtab = new_ent;
  functab.emplace(name, new_ent);
}

FuncTab::EntPtr FuncTab::get(std::string name) {
  if (this->functab.count(name))
    return this->functab.at(name);
  return nullptr;
}

FuncTab::EntPtr FuncTab::get_curtab() const {
  return this->curtab;
}

IR::Addr::Ptr FuncTabEntry::get_param_addr(int v) {
  if (v>=param_list.size())   param_list.resize(v+1);
  if (param_list[v]==nullptr) param_list[v] = IR::Addr::make_param(v);
  return param_list[v];
}
