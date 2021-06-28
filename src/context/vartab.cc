#include "vartab.h"

VarTabEntry::VarTabEntry(std::string name, std::vector<int> &&shape, IR::Addr::Ptr addr, std::vector<int> &&init_val, bool is_const, int param_order)
  : is_constant(is_const), param_order(param_order), addr(addr), name(name), type(std::move(shape)), init_val(std::move(init_val)) { }

void VarTab::put(std::shared_ptr<VarTabEntry> ent) {
  this->symtab.emplace(std::make_pair(ent->name, ent));
}

VarTab::EntPtr_const VarTab::get(std::string name) const {
  // 先在本作用域内找
  auto ent = this->symtab.at(name);

  // 在本作用域里找到了变量
  if (ent!=nullptr) return ent;

  // 否则，去祖先作用域里找
  auto ancestor = fa;
  while (ancestor!=nullptr && ((ent=ancestor->symtab.at(name))==nullptr)) ancestor = ancestor->fa;

  // 如果找到，则返回变量信息
  // 如果没有，则返回空指针
  // 两种情况编码上统一
  return ent;
}
