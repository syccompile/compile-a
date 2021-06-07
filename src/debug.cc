#include "debug.h"
#include "ast.h"

#include <iostream>

// Colors
const string normal = "\033[0m";
const string black = "\033[0;30m";
const string red = "\033[0;31m";
const string green = "\033[0;32m";
const string yellow = "\033[0;33m";
const string blue = "\033[0;34m";
const string white = "\033[0;37m";

void Expression::internal_print() {
  std::cout << yellow;
  //if(evaluable_) std::cout << "(*)" ;
  switch (op_) {
  case Op::AND:
    sub_exp1_->internal_print();
    std::cout << "&&";
    sub_exp2_->internal_print();
    break;
  case Op::OR:
    sub_exp1_->internal_print();
    std::cout << "||";
    sub_exp2_->internal_print();
    break;
  case Op::NEQ:
    sub_exp1_->internal_print();
    std::cout << "!=";
    sub_exp2_->internal_print();
    break;
  case Op::EQ:
    sub_exp1_->internal_print();
    std::cout << "==";
    sub_exp2_->internal_print();
    break;
  case Op::NOT:
    std::cout << "!";
    sub_exp1_->internal_print();
    break;
  case Op::ADD:
    sub_exp1_->internal_print();
    std::cout << "+";
    sub_exp2_->internal_print();
    break;
  case Op::SUB:
    sub_exp1_->internal_print();
    std::cout << "-";
    sub_exp2_->internal_print();
    break;
  case Op::MUL:
    sub_exp1_->internal_print();
    std::cout << "*";
    sub_exp2_->internal_print();
    break;
  case Op::DIV:
    sub_exp1_->internal_print();
    std::cout << "/";
    sub_exp2_->internal_print();
    break;
  case Op::MOD:
    sub_exp1_->internal_print();
    std::cout << "%";
    sub_exp2_->internal_print();
    break;
  case Op::GE:
    sub_exp1_->internal_print();
    std::cout << ">=";
    sub_exp2_->internal_print();
    break;
  case Op::LE:
    sub_exp1_->internal_print();
    std::cout << "<=";
    sub_exp2_->internal_print();
    break;
  case Op::GT:
    sub_exp1_->internal_print();
    std::cout << ">";
    sub_exp2_->internal_print();
    break;
  case Op::LT:
    sub_exp1_->internal_print();
    std::cout << "<";
    sub_exp2_->internal_print();
    break;
  case Op::IDENT:
    var_->internal_print();
    break;
  case Op::NUM:
    std::cout << number_;
    break;
  case Op::CALL:
    std::cout << func_name_ << "(";
    if (func_params_) {
      for (Expression *e : *func_params_) {
        e->internal_print();
        if(*(func_params_->rbegin()) != e){
          std::cout << ", ";
        }
      }
    }
    std::cout << ")";
    break;
  default:
    break;
  }
  std::cout << normal;
}

void Variable::internal_print() {
  std::cout << red;
  if (immutable_) {
    std::cout << "const ";
  }
  switch (type_) {
    case BType::INT:
      std::cout << "int ";
      break;
    case BType::VOID:
      std::cout << "void ";
      break;
    default:
      break;
  }
  std::cout << name_;
  if (initialized_ && initval_) {
    std::cout << " = ";
    initval_->internal_print();

  }
}

void Array::InitValExp::internal_print() { exp_->internal_print(); }

void Array::InitValContainer::internal_print() {
  std::cout << red;
  std::cout << "{";
  for (InitVal *initval : initval_container_) {
    initval->internal_print();
    if(*(initval_container_.rbegin()) != initval){
      std::cout << ", ";
    }
  }
  std::cout << "}";
}

void Array::internal_print() {
  Variable::internal_print();
  for (Expression *e : *dimens_) {
    std::cout << "[";
    e->internal_print();
    std::cout << "]";
  }
  if (initialized_) {
    std::cout << " = ";
    container_->internal_print();
  }
}

void VarDeclStmt::internal_print() {
  for (Variable *var : vars_) {
    var->internal_print();
    if (*(vars_.rbegin()) != var) {
      std::cout << ", " ;
    }
  }
  std::cout << ";"  << std::endl;
}

void BreakStmt::internal_print() { std::cout << "break;" << std::endl; }
void ReturnStmt::internal_print() {
  std::cout << "return";
  if (ret_exp_) {
    ret_exp_->internal_print();
  }
  std::cout << ";" << std::endl;
}
void ContinueStmt::internal_print() {
  std::cout << "continue ;" << std::endl;
}

void BlockStmt::internal_print() {
  std::cout << "{" << std::endl;
  for (Stmt *stmt : stmts_) {
    stmt->internal_print();
  }
  std::cout << "}" << std::endl;
}

void IfStmt::internal_print() {
  std::cout << "if (" ;
  condition_->internal_print();
  std::cout << ")" << std::endl;
  yes_->internal_print();
  if(!no_) return;
  std::cout << "else" << std::endl;
  no_->internal_print();
}

void WhileStmt::internal_print() {
  std::cout << "while (";
  condition_->internal_print();
  std::cout << ")" << std::endl;
  body_->internal_print();
}

void AssignmentStmt::internal_print() {
  std::cout << yellow;
  std::cout << name_;
  if(dimens_){
    for (Expression *e : *dimens_) {
      std::cout << "[";
      e->internal_print();
      std::cout << "]";
    }
  }
  std::cout << " = ";
  rval_->internal_print();
  std::cout << ";" << std::endl;
  std::cout << normal;
}

void FunctionDecl::internal_print() {
  std::cout << green;
  switch (ret_type_) {
    case BType::INT:
      std::cout << "int ";
      break;
    case BType::VOID:
      std::cout << "void ";
      break;
    default:
      break;
    }
    std::cout << func_name_ << "(";
    for(FParam* param : *params_){
      std::cout << blue;
      switch (param->type_) {
        case BType::INT:
          std::cout << "int ";
          break;
        case BType::VOID:
          std::cout << "void ";
          break;
        default:
          break;
      }
      std::cout << param->name_;
      if(!param->dimens_) break;
      for (Expression *e : *param->dimens_) {
        std::cout << "[";
        if(e) e->internal_print();
        std::cout << "]";
      }
      if (*(params_->rbegin()) != param) {
        std::cout << ", ";
      }
      std::cout << normal;
    }
    std::cout << ")" << std::endl;
    body_->internal_print();
    std::cout << normal;
}
