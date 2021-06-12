#pragma once
#include "ir.h"

#include <vector>

using std::vector;

class IrTranslator {
  virtual vector<IR>  translatate();
};
