#include "vartab.h"

VarTabEntry::VarTabEntry(std::string name, std::vector<int> shape, IR::Addr::Ptr addr, std::vector<int> init_val, bool is_const)
  : is_constant(is_const), addr(addr), name(name), type(std::move(shape)), init_val(std::move(init_val)) { }

void VarTab::put(VarTabEntry::Ptr ent) {
  this->symtab.emplace(std::make_pair(ent->name, ent));
}

VarTabEntry::Ptr_const VarTab::get(std::string name) const {
  // 先在本作用域内找
  if (this->symtab.count(name)) return this->symtab.at(name);

  // 否则，去祖先作用域里找
  auto ancestor = fa;
  while (ancestor!=nullptr) {
    if (ancestor->symtab.count(name)) return ancestor->symtab.at(name);
    else ancestor = ancestor->fa;
  }

  // 如果没有，则返回空指针
  return nullptr;
}
