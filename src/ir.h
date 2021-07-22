#pragma once

#include "debug.h"
#include "ir_addr.h"
#include "reg_allocate/flow_graph.h"

#include <memory>
#include <vector>
#include <list>

class IR: public Debug_impl, public varUse{
public:
  using Ptr = std::shared_ptr<IR>;
  using List = std::list<Ptr>;

  enum Op: int {
    ADD = 0,
    SUB,
    MUL,
    DIV,
    MOD,
    AND,
    OR,
    NOT,
    XOR,

    MOV,
    MVN,
    MOVLE,
    MOVLT,
    MOVGE,
    MOVGT,
    MOVEQ,
    MOVNE,

    CMP,

    LABEL,

    JMP,
    JLE,
    JLT,
    JGE,
    JGT,
    JE,
    JNE,

    FUNCDEF,
    FUNCEND,
    PARAM,
    CALL,
    RET,

    VARDEF,
    DATA,
    ZERO,
    LABVAL,
    VAREND,
    ALLOC_IN_STACK,

    LOAD,
    STORE,

    NOP
    // ...
  };

  using Addr = IR_Addr;

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

  // operand type
  bool is_al()  const { return this->op_<=XOR; }
  bool is_mov() const { return MOV<=this->op_ && this->op_<=MOVNE; }
  bool is_jmp() const { return JMP<=this->op_ && this->op_<=JNE; }

  Op op_;

  Addr::Ptr a0;
  Addr::Ptr a1;
  Addr::Ptr a2;

  bool in_inner_loop;
};
