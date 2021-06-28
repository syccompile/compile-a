#include "ast.h"
#include "context/context.h"

#include <cassert>
#include <string>

VarExp::VarExp(string *ident, Expression::List *dimens)
    : Expression(Op::VAR, false), ident_(*ident), dimens_(dimens) {}
VarExp::~VarExp() {
  if (dimens_) {
    for (Expression *exp : *dimens_) {
      delete exp;
    }
    delete dimens_;
  }
}

bool 
VarExp::is_evaluable() const {
  // 取得符号表里的对应表项，查看标识符是否为编译期常量
  auto entry = context.vartab_cur->get(this->ident_);
  assert(entry!=nullptr);
  if (!(entry->is_constant)) return false;

  // 再查看数组下标是否均为编译期常量
  if (this->dimens_) (Expression *exp: *(this->dimens_)) if (!(exp->is_evaluable())) return false;
  return true;
}

int
VarExp::eval() {
  auto entry = context.vartab_cur->get(this->ident_);
  assert(entry!=nullptr);

  const std::vector<int> &shape = entry->type.arr_shape;
  std::vector<int> dimension;

  if (this->dimens_) {
    // 数组
    for (Expression *exp: this->dimens_) dimension.push_back(exp->eval());
    assert(dimension.size()==entry.size());

    int offset = dimension.back();
    int acc = shape.back();

    for (int i=dimension.size()-2 ; i>0 ; i--) {
      offset += dimension[i] * acc;
      acc *= shape[i];
    }

    return entry->init_val[offset];
  } else {
    // 单变量
    return entry->init_val[0];
  }
}

// FIX: 将string 转换为int
NumberExp::NumberExp(string *str)
    : Expression(Op::NUM, true), string_(*str), value_(stoi(string_)) {
  assert(str);
}
NumberExp::~NumberExp() {}

FuncCallExp::FuncCallExp(string *func_name, vector<Expression *> *params)
    : Expression(Op::CALL, false), name_(*func_name), params_(params) {
  assert(func_name);
}
FuncCallExp::~FuncCallExp() {
  if (params_) {
    for (Expression *exp : *params_) {
      delete exp;
    }
  }
}

BinaryExp::BinaryExp(Op op, Expression *lhs, Expression *rhs)
    : Expression(op, false), left_(lhs), right_(rhs) {
  assert(lhs);
  assert(rhs);
}
BinaryExp::~BinaryExp() {
  delete left_;
  delete right_;
}
UnaryExp::UnaryExp(Op op, Expression *exp) : Expression(op, false), exp_(exp) {
  assert(exp);
}
UnaryExp::~UnaryExp() { delete exp_; }

Variable::Variable(BType type, string *name, bool immutable)
    : type_(type), name_(*name), immutable_(immutable),
      initialized_(false), initval_(nullptr) {
  assert(name);
}
Variable::Variable(BType type, string *name, bool immutable,
                   Expression *initval)
    : Variable(type, name, immutable) {
  assert(name);
  assert(initval);
  initialized_ = true;
  initval_ = initval;
}
Variable::~Variable() { delete initval_; }

Array::InitValExp::InitValExp(Expression *exp) : exp_(exp) { assert(exp); }
Array::InitValExp::~InitValExp() { delete exp_; }
Array::InitValContainer::InitValContainer() {}
Array::InitValContainer::~InitValContainer() {
  for (InitVal *p : initval_container_) {
    delete p;
  }
}

Array::Array(BType type, string *name, bool immutable, Expression::List *size)
    : Variable(type, name, immutable), dimens_(size),
      initval_container_(nullptr) {
  assert(name);
  assert(size);
}

Array::Array(BType type, string *name, bool immutable, Expression::List *size,
             InitVal *container)
    : Variable(type, name, immutable), dimens_(size),

      initval_container_(dynamic_cast<InitValContainer *>(container)) {
  assert(size);
  assert(container);
  initialized_ = true;
}

Array::~Array() {
  delete dimens_;
  delete initval_container_;
}

VarDeclStmt::VarDeclStmt() {}
VarDeclStmt::~VarDeclStmt() {
  for (Variable *var : vars_) {
    delete var;
  }
}

BlockStmt::BlockStmt() : symtab_(std::make_shared<SymbolTable>()) {}
BlockStmt::~BlockStmt() {
  for (Stmt *stmt : stmts_) {
    delete stmt;
  }
}

IfStmt::IfStmt(Expression *condition, BlockStmt *yes, BlockStmt *no)
    : condition_(condition), yes_(yes), no_(no) {
  assert(condition);
  assert(yes);
}
IfStmt::IfStmt(Expression *condition, BlockStmt *yes)
    : IfStmt(condition, yes, nullptr) {}
IfStmt::~IfStmt() {
  delete condition_;
  delete yes_;
  delete no_;
}

WhileStmt::WhileStmt(Expression *condition, BlockStmt *body)
    : condition_(condition), body_(body),
      break_access_(GlobFrame->newLabelAccess(GlobFrame)),
      continue_access_(GlobFrame->newLabelAccess(GlobFrame)) {
  assert(condition);
  assert(body);
}
WhileStmt::~WhileStmt() {
  delete condition_;
  delete body_;
}

ExpStmt::ExpStmt(Expression *exp) : exp_(exp) { assert(exp); }
ExpStmt::~ExpStmt() { delete exp_; }

ReturnStmt::ReturnStmt(Expression *ret) : ret_exp_(ret) {}
ReturnStmt::~ReturnStmt() { delete ret_exp_; }

AssignmentStmt::AssignmentStmt(string *name, Expression::List *dimens,
                               Expression *rval)
    : name_(*name), dimens_(dimens), rval_(rval) {
  assert(name);
  assert(rval);
}
AssignmentStmt::~AssignmentStmt() {
  delete dimens_;
  delete rval_;
}

FunctionDecl::FunctionDecl(BType ret_type, string *name, Variable::List *params,
                           BlockStmt *block)
    : ret_type_(ret_type), name_(*name), params_(params), body_(block),
      frame_(std::make_shared<Frame>(false)),
      ret_access_(frame_->newRetAccess(frame_)) {
  assert(name);
  assert(block);
}
FunctionDecl::~FunctionDecl() {
  delete params_;
  delete body_;
}

void BlockStmt::push_back(Stmt *stmt) {
  stmts_.push_back(stmt);
  // TODO
}

int Expression::eval() { return 0; }

int VarExp::eval() { return 0; }
int FuncCallExp::eval() { return 0; }
/* int LogicExp::eval() {
  switch (op_) {
  case Op::AND:
    return left_->eval() && right_->eval();
  case Op::OR:
    return left_->eval() || right_->eval();
  case Op::LT:
    return left_->eval() < right_->eval() ? 1 : 0;
  case Op::LE:
    return left_->eval() <= right_->eval() ? 1 : 0;
  case Op::GT:
    return left_->eval() > right_->eval() ? 1 : 0;
  case Op::GE:
    return left_->eval() >= right_->eval() ? 1 : 0;
  case Op::EQ:
    return left_->eval() == right_->eval() ? 1 : 0;
  case Op::NEQ:
    return left_->eval() != right_->eval() ? 1 : 0;
  default:
    return 0;
  }
}*/
int BinaryExp::eval() {
  switch (op_) {
  case Op::ADD:
    return left_->eval() + right_->eval();
  case Op::SUB:
    return left_->eval() - right_->eval();
  case Op::MUL:
    return left_->eval() * right_->eval();
  case Op::DIV:
    return left_->eval() / right_->eval();
  case Op::MOD:
    return left_->eval() % right_->eval();
  default:
    return 0;
  }
}
int UnaryExp::eval() {
  switch (op_) {
  case Op::NOT:
    return !exp_->eval();
  case Op::ADD:
    return exp_->eval();
  case Op::SUB:
    return -exp_->eval();
  default:
    return 0;
  }
}
int NumberExp::eval() { return value_; }
