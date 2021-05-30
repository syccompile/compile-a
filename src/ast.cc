#include "ast.h"

#include <string>

Expression::Expression(string ident)
    : op_(Op::IDENT), sub_exp1_(nullptr), sub_exp2_(nullptr), evaluable_(false),
      number_(0), name_(ident), params_(nullptr) {}

Expression::Expression(int value)
    : op_(Op::NUM), sub_exp1_(nullptr), sub_exp2_(nullptr), evaluable_(true),
      number_(value), name_(), params_(nullptr) {}

Expression::Expression(string function, vector<Expression *> *params)
    : Expression(function) {
  op_ = Op::CALL;
  params_ = params;
}
Expression::Expression(Op op, Expression *lhs, Expression *rhs)
    : Expression(0) {
  op_ = op;
  sub_exp1_ = lhs;
  sub_exp2_ = rhs;
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

Variable::Variable(BType type, string name, bool immutable)
    : type_(type), name_(name), immutable_(immutable), initialized_(false),
      initvals_(nullptr), size_(new Expression(1)) {}
Variable::Variable(BType type, string name, bool immutable, Expression *initval)
    : Variable(type, name, immutable) {
  initialized_ = true;
  initvals_ = new Expression::List;
  initvals_->push_back(initval);
}
Variable::Variable(BType type, string name, bool immutable, Expression *size,
                   Expression::List *initvals)
    : Variable(type, name, immutable) {
  delete size_;
  size_ = size;
  initialized_ = true;
  initvals_ = initvals;
}
Variable::~Variable() {
  delete size_;
  delete initvals_;
}
