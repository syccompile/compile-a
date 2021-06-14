#pragma once

#include "frame.h"

class IR {
public:
  using Ptr = std::shared_ptr<IR>;
  enum class Op {
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    JMP,
    CMOVE,
    // ...
  };
  IR(Op op): op_(op) {}
  virtual ~IR() {}
protected:
  Op op_;
};

class BinOpIR : public IR{
public:
  BinOpIR(Op op, FrameAccess dst, FrameAccess src1, FrameAccess src2): src1_(src1), src2_(src2), dst_(dst), IR(op) {}
  FrameAccess src1_;
  FrameAccess src2_;
  FrameAccess dst_;
};
class UnaryOpIR : public IR {
public:
  UnaryOpIR(Op op, FrameAccess dst, FrameAccess src) : src_(src), dst_(dst), IR(op){}
  FrameAccess src_;
  FrameAccess dst_;
};

class JmpIR : public IR {
public:
    JmpIR(FrameAccess dst) : dst_(dst), IR(Op::JMP) {}
    FrameAccess dst_;
};
// ... More 
