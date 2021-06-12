#pragma once

#include "frame.h"

class IR {
public:
  enum class Op {
    MOV,
    ADD,
    DIV,
    JMP,
    CMOVE,
    // ...
  };
  virtual ~IR() {}
private:
  Op op_;
};

class BinOpIR : public IR{
public:
  FrameAccess src1_;
  FrameAccess src2_;
  FrameAccess dst_;
};

class JmpIR : public IR {
  public:
    FrameAccess dst;
};
// ... More 
