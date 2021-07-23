#pragma once

#include <string>
#include "../ir_addr.h"
#include "../ir.h"

class VirtualAllocator {
public:
  IR::Addr::Ptr allocate_addr()  { return IR::Addr::make_var(acc_addr++); }
  IR::Addr::Ptr allocate_label() { return IR::Addr::make_label(acc_label++); }

  VirtualAllocator(): acc_addr(0), acc_label(0) { }

private:
  int acc_addr;
  int acc_label;
};
