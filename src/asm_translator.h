#pragma once
#include "frame.h"
#include "ir.h"

#include <vector>
#include <string>

class AsmTranslator_impl {
public:
  virtual std::vector<std::string> translate_asm(Frame::Ptr, std::vector<IR>) const = 0;
};
