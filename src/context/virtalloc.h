#pragma once

#include <string>

class VirtualAllocator {
public:
  int allocate_addr()  { return acc_addr++; }
  int allocate_label() { return acc_label++; }

  VirtualAllocator();

private:
  int acc_addr;
  int acc_label;
};
