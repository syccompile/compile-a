#pragma once

#include "asm_translator.h"
#include "debug.h"
#include "frame.h"

class IR : public Debug_impl, public AsmTranslator_impl {
public:
  using Ptr = std::shared_ptr<IR>;
  enum class Op {
    // BinSrcIR
    TEST,
    CMP,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,

    // UnarySrcIR
    MOV,

    // DstIR
    LABEL,
    JMP,
    JLE,
    JLT,
    JGE,
    JGT,
    JE,
    JNE,
    SETE,
    SETNE,
    SETGT,
    SETGE,
    SETLT,
    SETLE,

    CALL,

    // NoOpIR
    RET,
    CMOVE,

    // Instruction IR
    WORD,
    DWORD,
    ZERO,
    // ...
  };
  IR(Op op) : op_(op) {}
  virtual ~IR() {}
  virtual void internal_print() const override;
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;

protected:
  Op op_;
};

class BinSrcIR : public IR {
public:
  BinSrcIR(Op op, FrameAccess dst, FrameAccess src1, FrameAccess src2)
      : src1_(src1), src2_(src2), dst_(dst), IR(op) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
  FrameAccess src1_;
  FrameAccess src2_;
  FrameAccess dst_;
};
class UnarySrcIR : public IR {
public:
  UnarySrcIR(Op op, FrameAccess dst, FrameAccess src)
      : src_(src), dst_(dst), IR(op) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
  FrameAccess src_;
  FrameAccess dst_;
};

class DstIR : public IR {
public:
  DstIR(Op op, FrameAccess dst) : dst_(dst), IR(op) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
  FrameAccess dst_;
};

class NoOpIR : public IR {
public:
  NoOpIR(Op op) : IR(op) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
};

class InstructionIR : public IR {
public:
  InstructionIR(Op op, FrameAccess arg) : IR(op), arg_(arg) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
  FrameAccess arg_;
};
