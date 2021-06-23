#pragma once
#include "ir.h"
#include "symtab.h"

#include <memory>
#include <list>
#include <vector>

#define wrap_tie(vec, access, stmt, symtab)                                    \
  vector<IR::Ptr> vec;                                                         \
  FrameAccess access;                                                          \
  std::tie(vec, access) = stmt->translate(symtab);

using std::vector;

struct IrTranslator_impl {
  virtual std::list<IR> translate() {
    return std::list<IR>();
  }
};
