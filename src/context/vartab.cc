#include "vartab.h"

VarTabEntry::VarTabEntry(std::string name, std::vector<int> &&shape, int addr, bool is_const)
  : is_constant(is_const), address_number(addr), name(name), type(std::move(shape)) { }

void VarTab::put(std::string name, std::vector<int> shape, int addr, bool is_const) {
  this->symtab.emplace(std::make_pair(name, std::make_shared<VarTabEntry>(
    name, std::move(shape), addr, is_const
  )));
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
