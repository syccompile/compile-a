#pragma once
#include "frame.h"

#include <vector>
#include <string>

class AsmTranslator_impl {
public:
  virtual std::vector<std::string> translate_arm(Frame::Ptr frame) {
    return std::vector<std::string>();
  }

};
