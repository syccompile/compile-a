/**
 * 目前只对局部普通的变量声明，普通的变量赋值，return语句，函数声明等
 * 尝试了中间代码的生成
 */

#include "ir_translator.h"
#include "ast.h"
#include "ir.h"

#include <cassert>
static FunctionDecl *now_func;
static WhileStmt *now_while;

std::tuple<vector<IR::Ptr>, FrameAccess>
VarExp::translate(SymbolTable::Ptr symtab) {
  SymbolTable::SymTabEntry entry = symtab->find(ident_);
  if (entry.pointer_.var_ptr->immutable()) {
    return std::make_tuple(vector<IR::Ptr>(), entry.access_);
  }
  return std::make_tuple(vector<IR::Ptr>(), entry.access_);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
FuncCallExp::translate(SymbolTable::Ptr symtab) {
  vector<IR::Ptr> ret;
  SymbolTable::SymTabEntry entry = symtab->find(name_);
  if (params_) {
    size_t i = 0;
    for (Expression *exp : *params_) {
      wrap_tie(vec, access, exp, symtab);
      ret.insert(ret.end(), vec.begin(), vec.end());
      ret.push_back(std::make_shared<UnaryOpIR>(
          IR::Op::MOV, entry.pointer_.func_ptr->get_params_access(i), access));
      i++;
    }
  }
  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::CALL, entry.access_));
  return std::make_tuple(ret, entry.pointer_.func_ptr->get_return_access());
}

std::tuple<vector<IR::Ptr>, FrameAccess>
BinaryExp::translate(SymbolTable::Ptr symtab) {
  vector<IR::Ptr> ret;
  if (evaluable()) {
    return std::make_tuple(vector<IR::Ptr>(),
                           symtab->frame()->newImmAccess(symtab->frame(), eval()));
  }
  FrameAccess result = symtab->frame()->newTempAccess(symtab->frame());

  wrap_tie(lhs_vec, lhs_access, left_, symtab);
  wrap_tie(rhs_vec, rhs_access, right_, symtab);
  ret.insert(ret.end(), lhs_vec.begin(), lhs_vec.end());
  ret.insert(ret.end(), rhs_vec.begin(), rhs_vec.end());
  IR::Ptr ir;
  switch (op_) {
  case Op::ADD:
    ir = std::make_shared<BinOpIR>(IR::Op::ADD, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::SUB:
    ir = std::make_shared<BinOpIR>(IR::Op::SUB, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::MUL:
    ir = std::make_shared<BinOpIR>(IR::Op::MUL, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::DIV:
    ir = std::make_shared<BinOpIR>(IR::Op::DIV, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::MOD:
    ir = std::make_shared<BinOpIR>(IR::Op::MOD, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::LE:
    ret.push_back(
        std::make_shared<BinOpIR>(IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()), lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JLE, result));
    break;
  case Op::GE:
    ret.push_back(
        std::make_shared<BinOpIR>(IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()), lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JGE, result));
    break;
  case Op::LT:
    ret.push_back(
        std::make_shared<BinOpIR>(IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()), lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JLT, result));
    break;
  case Op::GT:
    ret.push_back(
        std::make_shared<BinOpIR>(IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()), lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JGT, result));
    break;
  case Op::EQ:
    ret.push_back(
        std::make_shared<BinOpIR>(IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()), lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JE, result));
    break;
  case Op::NEQ:
    ret.push_back(
        std::make_shared<BinOpIR>(IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()), lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JNE, result));
    break;
  default:
    break;
  }
  return std::make_tuple(ret, result);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
UnaryExp::translate(SymbolTable::Ptr symtab) {
  vector<IR::Ptr> ret;
  if (evaluable()) {
    return std::make_tuple(vector<IR::Ptr>(),
                           symtab->frame()->newImmAccess(symtab->frame(), eval()));
  }
  FrameAccess result = symtab->frame()->newTempAccess(symtab->frame());

  wrap_tie(vec, access, exp_, symtab);

  IR::Ptr ir;
  switch (op_) {
  case Op::ADD:
    result = access;
    break;
  case Op::SUB:
    ir = std::make_shared<BinOpIR>(IR::Op::SUB, result,
                                   symtab->frame()->newImmAccess(symtab->frame(), 0), access);
    ret.push_back(ir);
    break;
  default:
    break;
  }
  return std::make_tuple(ret, result);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
NumberExp::translate(SymbolTable::Ptr symtab) {
  return std::make_tuple(vector<IR::Ptr>(),
                         symtab->frame()->newImmAccess(symtab->frame(), value_));
}

std::tuple<vector<IR::Ptr>, FrameAccess>
VarDeclStmt::translate(SymbolTable::Ptr symtab) {
  vector<IR::Ptr> ret;
  for (Variable *var : vars_) {
    FrameAccess access = symtab->push(var);
    if (var->initialized() && !var->global()) {
      if (var->is_array()) {
        // TODO
      } else {
        wrap_tie(rhs_vec, rhs_access, var->initval_, symtab);
        ret.insert(ret.end(), rhs_vec.begin(), rhs_vec.end());
        ret.push_back(
            std::make_shared<UnaryOpIR>(IR::Op::MOV, access, rhs_access));
      }
    } else if (var->initialized() && var->global()) {
      // TODO
    }
  }
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ExpStmt::translate(SymbolTable::Ptr symtab) {
  return std::make_tuple(vector<IR::Ptr>(), nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
BlockStmt::translate(SymbolTable::Ptr symtab) {
  vector<IR::Ptr> ret;
  for (Stmt *stmt : stmts_) {
    wrap_tie(vec, access, stmt, symtab_);
    ret.insert(ret.end(), vec.begin(), vec.end());
  }
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
IfStmt::translate(SymbolTable::Ptr symtab) {
  yes_->symtab_->set_parent(symtab);
  if (no_) {
    no_->symtab_->set_parent(symtab);
  }

  vector<IR::Ptr> ret;
  FrameAccess next_label =
      symtab->frame()->newLabelAccess(symtab->frame());

  wrap_tie(conditon_vec, yes_label, condition_, symtab);
  ret.insert(ret.end(), conditon_vec.begin(), conditon_vec.end());
  if (no_) {
    wrap_tie(no_vec, access, no_, no_->symtab_);
    ret.insert(ret.end(), no_vec.begin(), no_vec.end());
  }
  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JMP, next_label));

  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::LABEL, yes_label));
  wrap_tie(yes_vec, yes_access, yes_, yes_->symtab_);
  ret.insert(ret.end(), yes_vec.begin(), yes_vec.end());
  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::LABEL, next_label));
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
WhileStmt::translate(SymbolTable::Ptr symtab) {
  body_->symtab_->set_parent(symtab);
  WhileStmt *temp = now_while;
  now_while = this;
  // TODO
  now_while = temp;
  return std::make_tuple(vector<IR::Ptr>(), nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ReturnStmt::translate(SymbolTable::Ptr symtab) {
  assert(now_func);
  parent_ = now_func;

  vector<IR::Ptr> ret;
  if (ret_exp_) {
    wrap_tie(vec, access, ret_exp_, symtab);
    ret.insert(ret.end(), vec.begin(), vec.end());
    ret.push_back(std::make_shared<UnaryOpIR>(
        IR::Op::MOV, parent_->get_return_access(), access));
  }
  ret.push_back(std::make_shared<NoOpIR>(IR::Op::RET));
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
BreakStmt::translate(SymbolTable::Ptr symtab) {
  assert(now_while);
  parent_ = now_while;
  // TODO
  return std::make_tuple(vector<IR::Ptr>(), nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ContinueStmt::translate(SymbolTable::Ptr symtab) {
  assert(now_while);
  parent_ = now_while;
  // TODO
  return std::make_tuple(vector<IR::Ptr>(), nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
AssignmentStmt::translate(SymbolTable::Ptr symtab) {
  // TODO
  vector<IR::Ptr> ret;
  auto entry = symtab->find(name_);
  auto var_ptr = entry.pointer_.var_ptr;
  assert(entry.type_ == SymbolTable::SymTabEntry::SymType::VARIABLE);
  assert(!var_ptr->immutable());
  if (var_ptr->is_array()) {

  } else {
    wrap_tie(vec, access, rval_, symtab);
    ret.insert(ret.end(), vec.begin(), vec.end());
    ret.push_back(
        std::make_shared<UnaryOpIR>(IR::Op::MOV, entry.access_, access));
  }
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
FunctionDecl::translate(SymbolTable::Ptr symtab) {
  symtab_->set_parent(symtab);
  body_->symtab_->set_parent(symtab_);
  FunctionDecl *temp = now_func;
  now_func = this;

  vector<IR::Ptr> ret;
  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::LABEL, frame_->newLabelAccess(frame_, name_)));
  wrap_tie(vec, access, body_, body_->symtab_);
  ret.insert(ret.end(), vec.begin(), vec.end());

  now_func = temp;
  return std::make_tuple(ret, nullptr);
}
