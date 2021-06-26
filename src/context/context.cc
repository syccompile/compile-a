#include "context.h"

Context::Context(): allocator() {
  vartab_cur = std::make_shared<VarTab>();
} 

void Context::new_scope() {
  auto new_vartab  = std::make_shared<VarTab>();
  new_vartab->fa   = this->vartab_cur;
  this->vartab_cur = new_vartab;
}

void Context::end_scope() {
  if (this->vartab_cur->fa == nullptr) return;
  this->vartab_cur = this->vartab_cur->fa;
}
