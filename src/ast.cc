#include "ast.h"

#include <string>

Expression::Expression(Variable* var)
    : op_(Op::IDENT), sub_exp1_(nullptr), sub_exp2_(nullptr), evaluable_(false),
      number_(), var_(var), func_name_(), func_params_(nullptr) {
      }

Expression::Expression(string *number)
    : op_(Op::NUM), sub_exp1_(nullptr), sub_exp2_(nullptr), evaluable_(true),
      number_(*number), var_(nullptr), func_name_(),
      func_params_(nullptr) {
}

Expression::Expression(string *function, vector<Expression *> *params)
    : op_(Op::CALL), sub_exp1_(nullptr), sub_exp2_(nullptr), evaluable_(false),
      number_(), var_(nullptr), func_name_(*function),
      func_params_(params) {
}

Expression::Expression(Op op, Expression *lhs, Expression *rhs)
    : op_(op), sub_exp1_(lhs), sub_exp2_(rhs), evaluable_(false),
      number_(), var_(nullptr), func_name_(),
      func_params_(nullptr) {
  if (lhs == nullptr && rhs == nullptr) {
    evaluable_ = false;
  } else if (lhs != nullptr && rhs != nullptr && lhs->evaluable_ &&
             rhs->evaluable_) {
    evaluable_ = true;
  } else if (lhs != nullptr && lhs->evaluable_) {
    evaluable_ = true;
  } else if (rhs != nullptr && rhs->evaluable_) {
    evaluable_ = true;
  }
}
Expression::Expression(Op op, Expression *exp) : Expression(op, exp, nullptr) {}

Expression::~Expression() {
  delete sub_exp1_;
  delete sub_exp2_;
}

Variable::Variable(BType type, string *name, bool immutable)
    : type_(type), name_(*name), immutable_(immutable), initialized_(false),
      initval_(nullptr) {
      }

Variable::Variable(BType type, string *name, bool immutable,
                   Expression *initval)
    : Variable(type, name, immutable) {
  initialized_ = true;
  initval_ = initval;
}
Variable::~Variable() { delete initval_; }

Array::InitValExp::InitValExp(Expression* exp) : exp_(exp) {}
Array::InitValExp::~InitValExp() { delete exp_; }
Array::InitValContainer::InitValContainer() {}
Array::InitValContainer::~InitValContainer() {
  for (InitVal *p : initval_container_) {
    delete p;
  }
}

Array::Array(BType type, string *name, bool immutable, Expression::List *size)
    : Variable(type, name, immutable), dimens_(size), container_(nullptr) {
    }

Array::Array(BType type, string* name, bool immutable, Expression::List *size, InitVal* container)
    : Variable(type, name, immutable), dimens_(size), container_(dynamic_cast<InitValContainer*>(container)) {
      initialized_ = true;
    }

Array::~Array() {
  delete dimens_;
  delete container_;
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
  : condition_(condition), yes_(yes), no_(no){
}
IfStmt::IfStmt(Expression *condition, BlockStmt *yes)
    : IfStmt(condition, yes, nullptr) {}

IfStmt::~IfStmt() {
  delete condition_;
  delete yes_;
  delete no_;
}

WhileStmt::WhileStmt(Expression *condition, BlockStmt *body)
    : condition_(condition), body_(body) {}
WhileStmt::~WhileStmt() {
  delete condition_;
  delete body_;
}

ExpStmt::ExpStmt(Expression *exp) : exp_(exp) {}
ExpStmt::~ExpStmt() { delete exp_; }

ReturnStmt::ReturnStmt(Expression *ret) : ret_exp_(ret) {}
ReturnStmt::~ReturnStmt() { delete ret_exp_; }

AssignmentStmt::AssignmentStmt(string *name, Expression::List *dimens,
                               Expression *rval)
    : name_(*name), dimens_(dimens), rval_(rval) {}
AssignmentStmt::~AssignmentStmt() {
  delete dimens_;
  delete rval_;
}

FunctionDecl::FunctionDecl(BType ret_type, string *name, FParam::List *params,
                   BlockStmt *block)
    : ret_type_(ret_type), func_name_(*name), params_(params), body_(block) {
}

FunctionDecl::~FunctionDecl() {
  delete params_;
  delete body_;
}
