#pragma once

#include <memory>

struct IR_Addr {
public:
  using Ptr = std::shared_ptr<IR_Addr>;

  enum Kind: int { VAR, IMM, BRANCH_LABEL, NAMED_LABEL } kind;
  int val;
  std::string name;

  IR_Addr(Kind kind, int val): kind(kind), name(), val(val) { }
  IR_Addr(Kind kind, std::string name): kind(kind), name(name), val(-1) { }
  
  static Ptr make_var(int v)   { return std::make_shared<IR_Addr>(Kind::VAR, v); }
  static Ptr make_imm(int v)   { return std::make_shared<IR_Addr>(Kind::IMM, v); }
  static Ptr make_label(int v) { return std::make_shared<IR_Addr>(Kind::BRANCH_LABEL, v); }
  static Ptr make_named_label(std::string s) { return std::make_shared<IR_Addr>(Kind::BRANCH_LABEL, s); }
};

class IR {
public:
  using Ptr = std::shared_ptr<IR>;
  enum class Op {
    LABEL,

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

    MOV,
    MOVLE,
    MOVLT,
    MOVGE,
    MOVGT,
    MOVEQ,
    MOVNE,

    CMP,

    PARAM,
    CALL,
    RET,
    CMOVE,

    NOP
    // ...
  };
  IR(Op op): op_(op) {}
  virtual ~IR() {}
  virtual void internal_print();
  virtual std::vector<std::string> translate_arm();
protected:
  Op op_;
};

class BinOpIR : public IR{
public:
  BinOpIR(Op op, IR_Addr::Ptr dst, IR_Addr::Ptr src1, IR_Addr::Ptr src2)
      : src1_(src1), src2_(src2), dst_(dst), IR(op) {}
  virtual std::vector<std::string> translate_arm() override;
  IR_Addr::Ptr dst_;
  IR_Addr::Ptr src1_;
  IR_Addr::Ptr src2_;
};

class UnaryOpIR : public IR {
public:
  UnaryOpIR(Op op, IR_Addr::Ptr dst, IR_Addr::Ptr src) : src_(src), dst_(dst), IR(op){}
  virtual std::vector<std::string> translate_arm() override;
  IR_Addr::Ptr src_;
  IR_Addr::Ptr dst_;
};
 
class SingalOpIR : public IR {
public :
  SingalOpIR(Op op, IR_Addr::Ptr dst) : dst_(dst), IR(op) {}
  virtual std::vector<std::string> translate_arm() override;
  IR_Addr::Ptr dst_;
};

class NoOpIR : public IR {
public :
  NoOpIR(Op op) : IR(op) {}
  virtual std::vector<std::string> translate_arm() override;
};
