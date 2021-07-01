#pragma once

#include <string>

class VirtualAllocator {
public:
  int allocate_addr()  { return acc_addr++; }
  int allocate_label() { return acc_label++; }

  VirtualAllocator(): acc_addr(0), acc_label(0) { }

private:
  int acc_addr;
  int acc_label;
};
