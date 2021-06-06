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
      initval_(nullptr){}
Variable::Variable(BType type, string name, bool immutable, Expression *initval)
    : Variable(type, name, immutable) {
  initialized_ = true;
  initval_ = initval;
}
Variable::~Variable() { delete initval_; }

Array::InitVals::InitVals(vector<Expression *> *initvals)
  : type_(Type::EXP)
{
  content_.exps = initvals;
}
Array::InitVals::InitVals(vector<Array::InitVals *> *initvals) 
 :type_(Type::CONTAINER)
{
  content_.container = initvals;
}
Array::InitVals::InitVals(Expression *exp) {
  content_.exps = new vector<Expression*>();
  content_.exps->push_back(exp);
}
Array::InitVals::InitVals(InitVals* initvals){
  content_.container = new vector<InitVals*>();
  content_.container->push_back(initvals);
}
Array::InitVals::~InitVals() {
  switch (type_) {
    case Type::EXP:
      delete content_.exps;
      break;
    case Type::CONTAINER:
      delete content_.container;
      break;
  }
}
bool Array::InitVals::push_back(Expression *exp) {
  if (type_ != Type::EXP) {
    return false;
  }
  content_.exps->push_back(exp);
  return true;
}

bool Array::InitVals::push_back(Array::InitVals *initvals) {
  if(type_ != Type::CONTAINER){
    return false;
  }
  content_.container->push_back(initvals);
  return true;
}
Array::Array(BType type, string name, bool immutable, Expression::List *size)
    : Variable(type, name, immutable), size_(size), initvals_(nullptr) {}

Array::Array(BType type, string name, bool immutable, Expression::List *size,
             InitVals *initvals)
    : Variable(type, name, immutable), size_(size), initvals_(initvals) {}

Array::~Array() {
  delete size_;
  delete initvals_;
}
