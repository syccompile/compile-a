#pragma once

#include "debug.h"

#include <memory>
#include <vector>

class IR: public Debug_impl {
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

    FUNCDEF,
    FUNCEND,
    PARAM,
    CALL,
    RET,

    LOAD,
    STORE,

    NOP
    // ...
  };

  struct Addr: public Debug_impl {
  public:
    using Ptr = std::shared_ptr<Addr>;

    enum Kind: int { VAR, PARAM, IMM, BRANCH_LABEL, NAMED_LABEL } kind;
    int val;
    std::string name;

    Addr(Kind kind, int val): kind(kind), name(), val(val) { }
    Addr(Kind kind, std::string name): kind(kind), name(name), val(-1) { }
    virtual void internal_print() override;
    
    static Ptr make_var(int v)   { return std::make_shared<Addr>(Kind::VAR, v); }
    static Ptr make_param(int v) { return std::make_shared<Addr>(Kind::PARAM, v); }
    static Ptr make_imm(int v)   { return std::make_shared<Addr>(Kind::IMM, v); }
    static Ptr make_label(int v) { return std::make_shared<Addr>(Kind::BRANCH_LABEL, v); }
    static Ptr make_named_label(std::string s) { return std::make_shared<Addr>(Kind::NAMED_LABEL, s); }
  };

  IR(Op op, Addr::Ptr a0, Addr::Ptr a1, Addr::Ptr a2): op_(op), a0(a0), a1(a1), a2(a2) { }
  ~IR() { }

  virtual void internal_print();
  std::vector<std::string> translate_arm();

  static std::shared_ptr<IR>
  make_triple(Op op, Addr::Ptr a0, Addr::Ptr a1, Addr::Ptr a2) { return std::make_shared<IR>(op, a0, a1, a2); }
  static std::shared_ptr<IR>
  make_binary(Op op, Addr::Ptr a0, Addr::Ptr a1) { return make_triple(op, a0, a1, nullptr); }
  static std::shared_ptr<IR>
  make_unary(Op op, Addr::Ptr a0) { return make_binary(op, a0, nullptr); }
  static std::shared_ptr<IR>
  make_no_operand(Op op) { return make_unary(op, nullptr); }

private:
  Op op_;

  Addr::Ptr a0;
  Addr::Ptr a1;
  Addr::Ptr a2;
};
