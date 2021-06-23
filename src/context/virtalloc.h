#pragma once

#include <string>

class VirtualAllocator {
public:
  int allocate_addr()      { return acc_addr++; }
  int allocate_tag_if()    { return acc_tag_if++; }
  int allocate_tag_while() { return acc_tag_while++; }

  VirtualAllocator();

private:
  int acc_addr;
  int acc_tag_if;
  int acc_tag_while;
};
