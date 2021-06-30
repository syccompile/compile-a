#include "asm_translator.h"
#include "ir.h"

std::vector<std::string> IR::translate_arm(Frame::Ptr frame) {
  return std::vector<std::string>();
}

std::vector<std::string> BinSrcIR::translate_arm(Frame::Ptr frame) {
  std::vector<std::string> ret;
  switch (op_) {
    case Op::ADD :
      break;
    default:
      break;
    }
  return ret;
}


std::vector<std::string> UnarySrcIR::translate_arm(Frame::Ptr frame) {
  return std::vector<std::string>();
}


std::vector<std::string> DstIR::translate_arm(Frame::Ptr frame) {
  return std::vector<std::string>();
}


std::vector<std::string> NoOpIR::translate_arm(Frame::Ptr frame){
  return std::vector<std::string>();
}
