#include "translate_to_asm.h"
#include "../reg_allocate/liveness_analyze.h"

std::list<std::string> translate_function(IR::List &ir_list) {
  std::list<std::string> ret;
  liveness_analyze(ir_list);
  return ret;
}
