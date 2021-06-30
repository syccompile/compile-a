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

void UnaryExp::internal_print() const {
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
void VarExp::internal_print() const {
  std::cout << red << ident_;
  if (dimens_) {
    for (Expression *exp : *dimens_) {
      std::cout << white << "[";
      exp->internal_print();
      std::cout << white << "]";
    }
  }
}

void LogicExp::internal_print() const {
  switch (op_) {
  case Op::AND:
    left_->internal_print();
    printer << yellow << "&&";
    right_->internal_print();
    break;
  case Op::OR:
    left_->internal_print();
    printer << yellow << "||";
    right_->internal_print();
    break;
  case Op::NEQ:
    left_->internal_print();
    printer << yellow << "!=";
    right_->internal_print();
    break;
  case Op::EQ:
    left_->internal_print();
    printer << yellow << "==";
    right_->internal_print();
    break;
  case Op::GE:
    left_->internal_print();
    printer << yellow << ">=";
    right_->internal_print();
    break;
  case Op::LE:
    left_->internal_print();
    printer << yellow << "<=";
    right_->internal_print();
    break;
  case Op::GT:
    left_->internal_print();
    printer << yellow << ">";
    right_->internal_print();
    break;
  case Op::LT:
    left_->internal_print();
    printer << yellow << "<";
    right_->internal_print();
    break;
  default:
    break;
  }
}
void BinaryExp::internal_print() const {
  switch (op_) {
  case Op::ADD:
    left_->internal_print();
    printer << yellow << "+";
    right_->internal_print();
    break;
  case Op::SUB:
    left_->internal_print();
    printer << yellow << "-";
    right_->internal_print();
    break;
  case Op::MUL:
    left_->internal_print();
    printer << yellow << "*";
    right_->internal_print();
    break;
  case Op::DIV:
    left_->internal_print();
    printer << yellow << "/";
    right_->internal_print();
    break;
  case Op::MOD:
    left_->internal_print();
    printer << yellow << "%";
    right_->internal_print();
    break;
  default:
    break;
  }
}

void FuncCallExp::internal_print() const {
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

void NumberExp::internal_print() const { printer << green << string_; }
void Variable::internal_print() const {
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

void Array::InitValExp::internal_print() const { exp_->internal_print(); }

void Array::InitValContainer::internal_print() const {
  printer << white << "{";
  for (InitVal *initval : initval_container_) {
    initval->internal_print();
    if (*(initval_container_.rbegin()) != initval) {
      printer << white << ", ";
    }
  }
  printer << white << "}";
}

void Array::internal_print() const {
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

void VarDeclStmt::internal_print() const {
  for (Variable *var : vars_) {
    var->internal_print();
    if (*(vars_.rbegin()) != var) {
      printer << white << ", ";
    }
  }
  printer << white << ";" << IndentPrinter::endl;
}

void BreakStmt::internal_print() const {
  printer << white << "break;" << IndentPrinter::endl;
}
void ReturnStmt::internal_print() const {
  printer << white << "return ";
  if (ret_exp_) {
    ret_exp_->internal_print();
  }
  printer << white << ";" << IndentPrinter::endl;
}
void ContinueStmt::internal_print() const {
  printer << white << "continue ;" << IndentPrinter::endl;
}

void BlockStmt::internal_print() const {
  printer << white << "{" << IndentPrinter::endl;
  printer.add_level();
  for (Stmt *stmt : stmts_) {
    stmt->internal_print();
  }
  printer.sub_level();
  printer << white << "}" << IndentPrinter::endl;
}

void IfStmt::internal_print() const {
  printer << blue << "if" << white << " (";
  condition_->internal_print();
  printer << white << ")" << IndentPrinter::endl;
  yes_->internal_print();
  if (!no_)
    return;
  printer << blue << "else" << IndentPrinter::endl;
  no_->internal_print();
}

void WhileStmt::internal_print() const {
  printer << blue << "while" << white << " (";
  condition_->internal_print();
  printer << white << ")" << IndentPrinter::endl;
  body_->internal_print();
}

void AssignmentStmt::internal_print() const {
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

void FunctionDecl::internal_print() const {
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
void IR::internal_print() const {
  printer.add_level();
  switch (op_) {
  case IR::Op::LABEL:
    printer.sub_level();
    dynamic_cast<const DstIR *>(this)->dst_->internal_print();
    printer << ":" << IndentPrinter::endl;
    printer.add_level();
    break;
  case IR::Op::MOV:
    printer << "MOV\t";
    dynamic_cast<const UnarySrcIR *>(this)->src_->internal_print();
    printer << white << "->";
    dynamic_cast<const UnarySrcIR *>(this)->dst_->internal_print();
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
  A:
    dynamic_cast<const BinSrcIR *>(this)->src1_->internal_print();
    printer << white << ", ";
    dynamic_cast<const BinSrcIR *>(this)->src2_->internal_print();
    printer << white << "->";
    dynamic_cast<const BinSrcIR *>(this)->dst_->internal_print();
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
    dynamic_cast<const DstIR *>(this)->dst_->internal_print();
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

void _FrameAccess::internal_print() const {
  switch (kind_) {
  case Kind::TEMP:
  case Kind::REG:
  case Kind::LABEL:
    printer << yellow << name_;
    break;
  case Kind::IMM:
    printer << green << std::to_string(locate_.offset);
    break;
  default:
    printer << "$$";
    break;
  }
}
