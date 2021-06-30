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

#define wrap_tie_arith(vec, access, stmt, symtab)                              \
  arith2logic = false;                                                         \
  wrap_tie(vec, access, stmt, symtab);

#define wrap_tie_logic(vec, access, stmt, symtab)                              \
  arith2logic = true;                                                          \
  wrap_tie(vec, access, stmt, symtab);

using std::vector;

struct IrTranslator_impl {
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) const = 0;
};
