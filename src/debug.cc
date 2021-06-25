#include "debug.h"
#include "ast.h"

#include <iostream>

class IndentPrinter {
public:
  const static char endl = '\0';
  IndentPrinter &operator<<(string str) {
    if (newline_) {
      for (int i = 0; i < level_; ++i) {
        std::cout << "\t";
      }
      newline_ = false;
    }
    std::cout << str;
    return *this;
  }
  IndentPrinter &operator<<(char ch) {
    if (ch == endl) {
      std::cout << std::endl;
      newline_ = true;
    } else {
      std::cout << ch;
    }
    return *this;
  }
  void set_level(int level) { level_ = level; }
  void add_level() { level_++; }
  void sub_level() { level_--; }

private:
  int level_;
  bool newline_;
};

IndentPrinter printer;

// Colors
const string normal = "\033[0m";
const string black = "\033[0;30m";
const string red = "\033[0;31m";
const string green = "\033[0;32m";
const string yellow = "\033[0;33m";
const string blue = "\033[0;34m";
const string white = "\033[0;37m";

void UnaryExp::internal_print() {
  switch (op_) {
  case Op::NOT:
    printer << yellow << "!";
    exp_->internal_print();
    break;
  case Op::ADD:
    printer << yellow << "+";
    exp_->internal_print();
    break;
  case Op::SUB:
    printer << yellow << "-";
    exp_->internal_print();
    break;
  default:
    break;
  }
}
void VarExp::internal_print() {
  std::cout << red << ident_;
  if (dimens_) {
    for (Expression *exp : *dimens_) {
      std::cout << white << "[";
      exp->internal_print();
      std::cout << white << "]";
    }
  }
}

void BinaryExp::internal_print() {
  switch (op_) {
  case Op::ADD:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "+";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::SUB:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "-";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::MUL:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "*";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::DIV:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "/";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::MOD:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "%";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::AND:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "&&";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::OR:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "||";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::NEQ:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "!=";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::EQ:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "==";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::GE:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << ">=";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::LE:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "<=";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::GT:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << ">";
    right_->internal_print();
    printer << yellow << ")";
    break;
  case Op::LT:
    printer << yellow << "(";
    left_->internal_print();
    printer << yellow << "<";
    right_->internal_print();
    printer << yellow << ")";
  default:
    break;
  }
}

void FuncCallExp::internal_print() {
  printer << red << name_ << white << "(";
  if (params_) {
    for (Expression *e : *params_) {
      e->internal_print();
      if (*(params_->rbegin()) != e) {
        printer << white << ", ";
      }
    }
  }
  printer << white << ")";
}

void NumberExp::internal_print() { printer << green << string_; }
void Variable::internal_print() {
  if (immutable_) {
    printer << blue << "const ";
  }
  switch (type_) {
  case BType::INT:
    printer << blue << "int ";
    break;
  case BType::VOID:
    printer << blue << "void ";
    break;
  default:
    break;
  }
  printer << red << name_;
  if (initialized_ && initval_) {
    printer << white << " = ";
    initval_->internal_print();
  }
}

void Array::InitValExp::internal_print() { exp_->internal_print(); }

void Array::InitValContainer::internal_print() {
  printer << white << "{";
  for (InitVal *initval : initval_container_) {
    initval->internal_print();
    if (*(initval_container_.rbegin()) != initval) {
      printer << white << ", ";
    }
  }
  printer << white << "}";
}

void Array::internal_print() {
  Variable::internal_print();
  for (Expression *e : *dimens_) {
    printer << white << "[";
    if (e) {
      e->internal_print();
    }
    printer << white << "]";
  }
  if (initialized_) {
    printer << white << " = ";
    initval_container_->internal_print();
  }
}

void VarDeclStmt::internal_print() {
  for (Variable *var : vars_) {
    var->internal_print();
    if (*(vars_.rbegin()) != var) {
      printer << white << ", ";
    }
  }
  printer << white << ";" << IndentPrinter::endl;
}

void BreakStmt::internal_print() {
  printer << white << "break;" << IndentPrinter::endl;
}
void ReturnStmt::internal_print() {
  printer << white << "return ";
  if (ret_exp_) {
    ret_exp_->internal_print();
  }
  printer << white << ";" << IndentPrinter::endl;
}
void ContinueStmt::internal_print() {
  printer << white << "continue ;" << IndentPrinter::endl;
}

void BlockStmt::internal_print() {
  printer << white << "{" << IndentPrinter::endl;
  printer.add_level();
  for (Stmt *stmt : stmts_) {
    stmt->internal_print();
  }
  printer.sub_level();
  printer << white << "}" << IndentPrinter::endl;
}

void IfStmt::internal_print() {
  printer << blue << "if" << white << " (";
  condition_->internal_print();
  printer << white << ")" << IndentPrinter::endl;
  yes_->internal_print();
  if (!no_)
    return;
  printer << blue << "else" << IndentPrinter::endl;
  no_->internal_print();
}

void WhileStmt::internal_print() {
  printer << blue << "while" << white << " (";
  condition_->internal_print();
  printer << white << ")" << IndentPrinter::endl;
  body_->internal_print();
}

void AssignmentStmt::internal_print() {
  printer << green << name_;
  if (dimens_) {
    for (Expression *e : *dimens_) {
      printer << white << "[";
      e->internal_print();
      printer << white << "]";
    }
  }
  printer << white << " = ";
  rval_->internal_print();
  printer << white << ";" << IndentPrinter::endl;
}

void FunctionDecl::internal_print() {
  switch (ret_type_) {
  case BType::INT:
    printer << blue << "int ";
    break;
  case BType::VOID:
    printer << blue << "void ";
    break;
  default:
    break;
  }
  printer << red << name_ << white << "(";
  if (params_) {
    for (Variable *param : *params_) {
      param->internal_print();
      if (*(params_->rbegin()) != param) {
        printer << white << ", ";
      }
      printer << normal;
    }
  }
  printer << white << ")" << IndentPrinter::endl;
  body_->internal_print();
}
void IR::internal_print() {
  printer.add_level();
  switch (op_) {
  case IR::Op::LABEL:
    printer.sub_level();
    dynamic_cast<SingalOpIR *>(this)->dst_->internal_print();
    printer << ":" << IndentPrinter::endl;
    printer.add_level();
    break;
  case IR::Op::MOV:
    printer << "MOV\t";
    dynamic_cast<UnaryOpIR *>(this)->src_->internal_print();
    printer << white << "\t\t->\t";
    dynamic_cast<UnaryOpIR *>(this)->dst_->internal_print();
    printer << IndentPrinter::endl;
    break;
  case IR::Op::SUB:
    printer << "SUB\t";
    goto A;
  case IR::Op::ADD:
    printer << "ADD\t";
    goto A;
  case IR::Op::MUL:
    printer << "MUL\t";
    goto A;
  case IR::Op::DIV:
    printer << "DIV\t";
    goto A;
  case IR::Op::MOD:
    printer << "MOD\t";
    goto A;
  case IR::Op::CMP:
    printer << "CMP\t";
    goto A;
  case IR::Op::TEST:
    printer << "TEST\t";
  A:
    dynamic_cast<BinOpIR *>(this)->src1_->internal_print();
    printer << white << ",\t";
    dynamic_cast<BinOpIR *>(this)->src2_->internal_print();
    printer << white << "\t->\t";
    dynamic_cast<BinOpIR *>(this)->dst_->internal_print();
    printer << IndentPrinter::endl;
    break;
  case IR::Op::CALL:
    printer << "CALL\t";
    goto B;
  case IR::Op::JMP:
    printer << "JMP\t";
    goto B;
  case IR::Op::JE:
    printer << "JE\t";
    goto B;
  case IR::Op::JNE:
    printer << "JNE\t";
    goto B;
  case IR::Op::JLE:
    printer << "JLE\t";
    goto B;
  case IR::Op::JGE:
    printer << "JGE\t";
    goto B;
  case IR::Op::JLT:
    printer << "JLT\t";
    goto B;
  case IR::Op::JGT:
    printer << "JGT\t";
  B:
    dynamic_cast<SingalOpIR *>(this)->dst_->internal_print();
    printer << IndentPrinter::endl;
    break;
  case IR::Op::RET:
    printer << "RET" << IndentPrinter::endl;
    break;
  default:
    printer << "NOP" << IndentPrinter::endl;
    break;
  }
  printer.sub_level();
}

void _FrameAccess::internal_print() {
  switch (kind_) {
  case Kind::TEMP:
  case Kind::REG:
  case Kind::LABEL:
    printer << yellow;
    printer << name_;
    break;
  case Kind::IMM:
    printer << green << std::to_string(locate_.offset);
    break;
  default:
    printer << "$$";
    break;
  }
}
