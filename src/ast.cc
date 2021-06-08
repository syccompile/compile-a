#include "ast.h"

#include <cassert>
#include <string>

VarExp::VarExp(Variable *var) : Expression(Op::VAR, false), var_(var) {
  assert(var);
}
VarExp::~VarExp() { delete var_; }

NumberExp::NumberExp(string *str)
    : Expression(Op::NUM, true), string_(*str) {
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
  set_evaluable(left_->evaluable() && right_->evaluable());
}

BinaryExp::~BinaryExp() {
  delete left_;
  delete right_;
}
UnaryExp::UnaryExp(Op op, Expression *exp) : Expression(op, false), exp_(exp) {
  assert(exp);
  set_evaluable(exp->evaluable());
}
UnaryExp::~UnaryExp() { delete exp_; }

Variable::Variable(BType type, string *name, bool immutable)
    : type_(type), name_(*name), immutable_(immutable), initialized_(false),
      initval_(nullptr) {
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
    : Variable(type, name, immutable), dimens_(size), initval_container_(nullptr) {
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

BlockStmt::BlockStmt() {}
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
    : condition_(condition), body_(body) {
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

FunctionDecl::FunctionDecl(BType ret_type, string *name, FParam::List *params,
                           BlockStmt *block)
    : ret_type_(ret_type), name_(*name), params_(params), body_(block) {
  assert(name);
  assert(block);
}

FunctionDecl::~FunctionDecl() {
  delete params_;
  delete body_;
}
