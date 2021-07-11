#include "functab.h"

void FuncTab::put(std::string name, std::list<Type> param_types) {
  functab.emplace(name, std::make_shared<FuncTabEntry>(name, param_types));
}

FuncTab::EntPtr FuncTab::get(std::string name) {
  if (this->functab.count(name))
    return this->functab.at(name);
  return nullptr;
}

