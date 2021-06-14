#pragma once
#include "ir.h"
#include "symtab.h"

#include <vector>
#include <tuple>
#include <memory>

using std::vector;

struct IrTranslator_impl {
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) {
        return std::make_tuple(vector<IR::Ptr>(), nullptr);
      }
};
