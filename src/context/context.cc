#include "context.h"

Context context;

Context::Context(): allocator() {
  vartab_cur = std::make_shared<VarTab>();
  functab = std::make_shared<FuncTab>();
} 

void Context::new_scope() {
  auto new_vartab  = std::make_shared<VarTab>();
  new_vartab->fa   = this->vartab_cur;
  this->vartab_cur = new_vartab;
}

void Context::end_scope() {
  this->vartab_cur = this->vartab_cur->fa;
}
