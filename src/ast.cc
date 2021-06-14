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

NumberExp::NumberExp(string *str) : Expression(Op::NUM, true), string_(*str) {
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
    : global_(false), type_(type), name_(*name), immutable_(immutable),
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

FunctionDecl::FunctionDecl(BType ret_type, string *name, Variable::List *params,
                           BlockStmt *block)
    : ret_type_(ret_type), name_(*name), params_(params), body_(block),
      frame_(std::make_shared<Frame>(false)),
      symtab_(std::make_shared<SymbolTable>()),
      ret_access_(frame_->newRetAccess()) {
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
