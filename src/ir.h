#pragma once

#include "frame.h"
#include "debug.h"

class IR : public Debug_impl{
public:
  using Ptr = std::shared_ptr<IR>;
  enum class Op {
    LABEL,
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    JMP,
    CALL,
    RET,
    CMOVE,
    // ...
  };
  IR(Op op): op_(op) {}
  virtual ~IR() {}
  virtual void internal_print() override;
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

class CallIR : public IR {
public:
    CallIR(FrameAccess dst) : dst_(dst), IR(Op::CALL) {}
    FrameAccess dst_;
};

class RetIR : public IR {
public:
    RetIR() : IR(Op::RET) {}
};
class LabelIR : public IR {
public:
  LabelIR(FrameAccess dst) : IR(Op::LABEL), dst_(dst) {}
    FrameAccess dst_;
};
// ... More 
