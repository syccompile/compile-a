#pragma once

#include "asm_translator.h"
#include "ir.h"

#include <string>
#include <vector>

class Arm_asm : public AsmTranslator_impl {
public:
  virtual std::vector<std::string>
      translate_asm(Frame::Ptr, std::vector<IR>) const override;
};
