#pragma once
#include "debug.h"
#include "reg_allocate/flow_graph.h"
#include <string>
#include <memory>
#include <unordered_map>

using namespace color_graph;

class IR;

struct IR_Addr: public Debug_impl, public var{
public:
  using Ptr = std::shared_ptr<IR_Addr>;

  enum Kind: int { VAR, PARAM, IMM, BRANCH_LABEL, NAMED_LABEL, RET } kind;
  int val;
  std::string name;
  std::shared_ptr<IR> _label_decl_addr;

  IR_Addr(Kind kind) : kind(kind), val(-1), name(), var() { }
  IR_Addr(Kind kind, int val): kind(kind), name(), val(val), var() { }
  IR_Addr(Kind kind, std::string name): kind(kind), name(name), val(-1), var() { }
  virtual void internal_print() override;
  
  static Ptr make_var(int v)   { return std::make_shared<IR_Addr>(Kind::VAR, v); }
  static Ptr make_param(int v) { return std::make_shared<IR_Addr>(Kind::PARAM, v); }
  static Ptr make_imm(int v)   { return std::make_shared<IR_Addr>(Kind::IMM, v); }
  static Ptr make_label(int v) { return std::make_shared<IR_Addr>(Kind::BRANCH_LABEL, v); }
  static Ptr make_named_label(std::string s) { return std::make_shared<IR_Addr>(Kind::NAMED_LABEL, s); }
  static Ptr make_ret() { return std::make_shared<IR_Addr>(RET, 0); }
};
