#pragma once
#include "ir.h"
#include "symtab.h"

#include <memory>
#include <tuple>
#include <vector>

#define wrap_tie(vec, access, stmt, symtab)                                    \
  vector<IR::Ptr> vec;                                                         \
  FrameAccess access;                                                          \
  std::tie(vec, access) = stmt->translate(symtab);

using std::vector;

struct IrTranslator_impl {
  virtual std::tuple<vector<IR::Ptr>, FrameAccess> translate(SymbolTable::Ptr) {
    return std::make_tuple(vector<IR::Ptr>(), nullptr);
  }
};
