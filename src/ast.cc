#include "ast.h"

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

// FIX: 将string 转换为int
NumberExp::NumberExp(const string *str)
    : Expression(Op::NUM, true), string_(*str), value_(stoi(string_)) {
  assert(str);
}
NumberExp::~NumberExp() {}

FuncCallExp::FuncCallExp(const string *func_name,
                         const vector<Expression *> *params)
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
BinaryExp::BinaryExp(const Op op, const Expression *lhs, const Expression *rhs)
    : Expression(op, false), left_(lhs), right_(rhs) {
  assert(lhs);
  assert(rhs);
  evaluable_ = left_->evaluable() && right_->evaluable();
}

BinaryExp::~BinaryExp() {
  delete left_;
  delete right_;
}
UnaryExp::UnaryExp(const Op op, const Expression *exp)
    : Expression(op, false), exp_(exp) {
  assert(exp);
  evaluable_ = exp->evaluable();
}
UnaryExp::~UnaryExp() { delete exp_; }

Variable::Variable(const BType type, const string *name, bool immutable)
    : type_(type), name_(*name), immutable_(immutable), initialized_(false),
      initval_(nullptr) {
  assert(name);
}

Variable::Variable(const BType type, const string *name, bool immutable,
                   const Expression *initval)
    : type_(type), name_(*name), immutable_(immutable), initialized_(true),
      initval_(initval) {
  assert(name);
  assert(initval);
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

Array::Array(const BType type, const string *name, bool immutable,
             const Expression::List *size)
    : Variable(type, name, immutable), dimens_(size),
      initval_container_(nullptr) {
  assert(name);
  assert(size);
}

Array::Array(const BType type, const string *name, bool immutable,
             const Expression::List *size, InitVal *container)
    : Variable(type, name, immutable), dimens_(size),

      initval_container_(dynamic_cast<InitValContainer *>(container)) {
  assert(size);
  assert(container);
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

IfStmt::IfStmt(const Expression *condition, const BlockStmt *yes,
               const BlockStmt *no)
    : condition_(condition), yes_(yes), no_(no) {
  assert(condition);
  assert(yes);
}
IfStmt::IfStmt(const Expression *condition, const BlockStmt *yes)
    : IfStmt(condition, yes, nullptr) {}

IfStmt::~IfStmt() {
  delete condition_;
  delete yes_;
  delete no_;
}

WhileStmt::WhileStmt(const Expression *condition, const BlockStmt *body)
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

ReturnStmt::ReturnStmt(const Expression *ret) : ret_exp_(ret) {}
ReturnStmt::~ReturnStmt() { delete ret_exp_; }

AssignmentStmt::AssignmentStmt(const string *name,
                               const Expression::List *dimens,
                               const Expression *rval)
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
      symtab_(std::make_shared<SymbolTable>()),
      ret_access_(frame_->newRetAccess(frame_)) {
  assert(name);
  assert(block);

  GlobSymTab->push(this);

  if (params_) {
    for (Variable *param : *params_) {
      symtab_->push(param);
    }
  }
}

FunctionDecl::~FunctionDecl() {
  delete params_;
  delete body_;
}

void VarDeclStmt::set_global() {
  for (Variable *var : vars_) {
    var->set_global(true);
  }
}

void BlockStmt::push_back(Stmt *stmt) {
  stmts_.push_back(stmt);
  // TODO
}

int VarExp::eval() const { return 0; }
int FuncCallExp::eval() const { return 0; }
int BinaryExp::eval() const {
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
}
int UnaryExp::eval() const {
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
int NumberExp::eval() const { return value_; }
