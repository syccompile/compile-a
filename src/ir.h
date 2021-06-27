#pragma once

#include "frame.h"
#include "asm_translator.h"
#include "debug.h"

class IR : public Debug_impl, public AsmTranslator_impl{
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

    CMP,

    CALL,
    TEST,
    RET,
    CMOVE,

    WORD,
    DWORD,
    ZERO,
    // ...
  };
  IR(Op op): op_(op) {}
  virtual ~IR() {}
  virtual void internal_print() const override;
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
protected:
  Op op_;
};

class BinOpIR : public IR{
public:
  BinOpIR(Op op, FrameAccess dst, FrameAccess src1, FrameAccess src2)
      : src1_(src1), src2_(src2), dst_(dst), IR(op) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
  FrameAccess src1_;
  FrameAccess src2_;
  FrameAccess dst_;
};
class UnaryOpIR : public IR {
public:
  UnaryOpIR(Op op, FrameAccess dst, FrameAccess src) : src_(src), dst_(dst), IR(op){}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
  FrameAccess src_;
  FrameAccess dst_;
};
 
class SingalOpIR : public IR {
public :
  SingalOpIR(Op op, FrameAccess dst) : dst_(dst), IR(op) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
  FrameAccess dst_;
};

class NoOpIR : public IR {
public :
  NoOpIR(Op op) : IR(op) {}
  virtual std::vector<std::string> translate_arm(Frame::Ptr) override;
};
