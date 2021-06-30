/**
 * 目前只对局部普通的变量声明，普通的变量赋值，return语句，函数声明等
 * 尝试了中间代码的生成
 */

#include "ir_translator.h"
#include "ast.h"
#include "ir.h"

#include <cassert>
const static FunctionDecl *now_func;
const static WhileStmt *now_while;
static bool jmp_revert = false;

std::tuple<vector<IR::Ptr>, FrameAccess>
VarExp::translate(SymbolTable::Ptr symtab) const {
  SymbolTable::SymTabEntry entry = symtab->find(ident_);
  if (entry.pointer_.var_ptr->immutable()) {
    return std::make_tuple(vector<IR::Ptr>(), entry.access_);
  }
  return std::make_tuple(vector<IR::Ptr>(), entry.access_);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
FuncCallExp::translate(SymbolTable::Ptr symtab) const {
  vector<IR::Ptr> ret;
  SymbolTable::SymTabEntry entry = symtab->find(name_);
  if (params_) {
    size_t i = 0;
    for (Expression *exp : *params_) {
      wrap_tie(vec, access, exp, symtab);
      ret.insert(ret.end(), vec.begin(), vec.end());
      ret.push_back(std::make_shared<UnarySrcIR>(
          IR::Op::MOV, entry.pointer_.func_ptr->get_params_access(i), access));
      i++;
    }
  }
  ret.push_back(std::make_shared<DstIR>(IR::Op::CALL, entry.access_));
  return std::make_tuple(ret, entry.pointer_.func_ptr->get_return_access());
}

std::tuple<vector<IR::Ptr>, FrameAccess>
LogicExp::translate(SymbolTable::Ptr symtab) const {
  vector<IR::Ptr> ret;
  FrameAccess result;
  if (op_ == Op::AND) {
    if (jmp_revert) {
      jmp_revert = true;
      wrap_tie(lhs_vec, lhs_access, left_, symtab);
      wrap_tie(rhs_vec, rhs_access, right_, symtab);
      ret.insert(ret.end(), lhs_vec.begin(), lhs_vec.end());
      ret.insert(ret.end(), rhs_vec.begin(), rhs_vec.end());
      rhs_access->copy(lhs_access);
      return std::make_tuple(ret, lhs_access);
    } else {
      jmp_revert = true;
      wrap_tie(lhs_vec, lhs_access, left_, symtab);
      jmp_revert = false;
      wrap_tie(rhs_vec, rhs_access, right_, symtab);
      ret.insert(ret.end(), lhs_vec.begin(), lhs_vec.end());
      ret.insert(ret.end(), rhs_vec.begin(), rhs_vec.end());
      ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL, lhs_access));
      return std::make_tuple(ret, rhs_access);
    }
  } else if (op_ == Op::OR) {
    if (jmp_revert) {
      jmp_revert = true;
      wrap_tie(lhs_vec, lhs_access, left_, symtab);
      jmp_revert = false;
      wrap_tie(rhs_vec, rhs_access, right_, symtab);
      ret.insert(ret.end(), lhs_vec.begin(), lhs_vec.end());
      ret.insert(ret.end(), rhs_vec.begin(), rhs_vec.end());
      ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL, lhs_access));
      return std::make_tuple(ret, rhs_access);
    } else {
      jmp_revert = false;
      wrap_tie(lhs_vec, lhs_access, left_, symtab);
      wrap_tie(rhs_vec, rhs_access, right_, symtab);
      ret.insert(ret.end(), lhs_vec.begin(), lhs_vec.end());
      ret.insert(ret.end(), rhs_vec.begin(), rhs_vec.end());
      rhs_access->copy(lhs_access);
      return std::make_tuple(ret, lhs_access);
    }
  }
  wrap_tie(lhs_vec, lhs_access, left_, symtab);
  wrap_tie(rhs_vec, rhs_access, right_, symtab);
  switch (op_) {
  case Op::LE:
    ret.push_back(std::make_shared<BinSrcIR>(
        IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()),
        lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    if (jmp_revert) {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JGT, result));
    } else {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JLE, result));
    }
    break;
  case Op::GE:
    ret.push_back(std::make_shared<BinSrcIR>(
        IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()),
        lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    if (jmp_revert) {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JLT, result));
    } else {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JGE, result));
    }
    break;
  case Op::LT:
    ret.push_back(std::make_shared<BinSrcIR>(
        IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()),
        lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    if (jmp_revert) {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JGE, result));
    } else {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JLT, result));
    }
    break;
  case Op::GT:
    ret.push_back(std::make_shared<BinSrcIR>(
        IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()),
        lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    if (jmp_revert) {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JLE, result));
    } else {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JGT, result));
    }
    break;
  case Op::EQ:
    ret.push_back(std::make_shared<BinSrcIR>(
        IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()),
        lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    if (jmp_revert) {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JNE, result));
    } else {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JE, result));
    }
    break;
  case Op::NEQ:
    ret.push_back(std::make_shared<BinSrcIR>(
        IR::Op::CMP, symtab->frame()->newTempAccess(symtab->frame()),
        lhs_access, rhs_access));
    result = symtab->frame()->newLabelAccess(symtab->frame());
    if (jmp_revert) {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JE, result));
    } else {
      ret.push_back(std::make_shared<DstIR>(IR::Op::JNE, result));
    }
    break;
  default:
    break;
  }
  return std::make_tuple(ret, result);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
BinaryExp::translate(SymbolTable::Ptr symtab) const {
  vector<IR::Ptr> ret;
  // FIX: 现在的计算还有些问题
  if (evaluable()) {
    return std::make_tuple(vector<IR::Ptr>(), symtab->frame()->newImmAccess(
                                                  symtab->frame(), eval()));
  }
  FrameAccess result = symtab->frame()->newTempAccess(symtab->frame());

  wrap_tie(lhs_vec, lhs_access, left_, symtab);
  wrap_tie(rhs_vec, rhs_access, right_, symtab);
  ret.insert(ret.end(), lhs_vec.begin(), lhs_vec.end());
  ret.insert(ret.end(), rhs_vec.begin(), rhs_vec.end());
  IR::Ptr ir;
  switch (op_) {
  case Op::ADD:
    ir =
        std::make_shared<BinSrcIR>(IR::Op::ADD, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::SUB:
    ir =
        std::make_shared<BinSrcIR>(IR::Op::SUB, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::MUL:
    ir =
        std::make_shared<BinSrcIR>(IR::Op::MUL, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::DIV:
    ir =
        std::make_shared<BinSrcIR>(IR::Op::DIV, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  case Op::MOD:
    ir =
        std::make_shared<BinSrcIR>(IR::Op::MOD, result, lhs_access, rhs_access);
    ret.push_back(ir);
    break;
  default:
    break;
  }
  return std::make_tuple(ret, result);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
UnaryExp::translate(SymbolTable::Ptr symtab) const {
  vector<IR::Ptr> ret;
  // FIX
  if (evaluable()) {
    return std::make_tuple(vector<IR::Ptr>(), symtab->frame()->newImmAccess(
                                                  symtab->frame(), eval()));
  }
  FrameAccess result = symtab->frame()->newTempAccess(symtab->frame());

  wrap_tie(vec, access, exp_, symtab);

  IR::Ptr ir;
  switch (op_) {
  case Op::ADD:
    result = access;
    break;
  case Op::SUB:
    ir = std::make_shared<BinSrcIR>(
        IR::Op::SUB, result, symtab->frame()->newImmAccess(symtab->frame(), 0),
        access);
    ret.push_back(ir);
    break;
  case Op::NOT:
    // FIX
    jmp_revert = !jmp_revert;
    return exp_->translate(symtab);
  default:
    break;
  }
  return std::make_tuple(ret, result);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
NumberExp::translate(SymbolTable::Ptr symtab) const {
  return std::make_tuple(vector<IR::Ptr>(), symtab->frame()->newImmAccess(
                                                symtab->frame(), value_));
}

std::tuple<vector<IR::Ptr>, FrameAccess>
VarDeclStmt::translate(SymbolTable::Ptr symtab) const {
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
            std::make_shared<UnarySrcIR>(IR::Op::MOV, access, rhs_access));
      }
    } else if (var->initialized() && var->global()) {
      // TODO
    }
  }
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ExpStmt::translate(SymbolTable::Ptr symtab) const {
  return std::make_tuple(vector<IR::Ptr>(), nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
BlockStmt::translate(SymbolTable::Ptr symtab) const {
  vector<IR::Ptr> ret;
  for (Stmt *stmt : stmts_) {
    wrap_tie(vec, access, stmt, symtab_);
    ret.insert(ret.end(), vec.begin(), vec.end());
  }
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
IfStmt::translate(SymbolTable::Ptr symtab) const {
  yes_->symtab_->set_parent(symtab);
  if (no_) {
    no_->symtab_->set_parent(symtab);
  }

  vector<IR::Ptr> ret;
  FrameAccess next_label = symtab->frame()->newLabelAccess(symtab->frame());

  jmp_revert = false;
  wrap_tie(condition_vec, yes_label, condition_, symtab);
  ret.insert(ret.end(), condition_vec.begin(), condition_vec.end());
  if (no_) {
    wrap_tie(no_vec, access, no_, no_->symtab_);
    ret.insert(ret.end(), no_vec.begin(), no_vec.end());
  }
  ret.push_back(std::make_shared<DstIR>(IR::Op::JMP, next_label));

  ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL, yes_label));
  wrap_tie(yes_vec, yes_access, yes_, yes_->symtab_);
  ret.insert(ret.end(), yes_vec.begin(), yes_vec.end());
  ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL, next_label));
  return std::make_tuple(ret, nullptr);
}
/**
 * while语句翻译为中间代码的模式如下
 *          JMP  .L1
 *      .L2:
 *          #循环体代码
 *          # ...
 *          # ...
 *      .L1:
 *          # 判断代码
 *          # ...
 *          CMP   ax, bx # 比较
 *          JLE   .L2
 *      .L3:
 *          # ...
 */
std::tuple<vector<IR::Ptr>, FrameAccess>
WhileStmt::translate(SymbolTable::Ptr symtab) const {
  body_->symtab_->set_parent(symtab);
  const WhileStmt *temp = now_while;
  now_while = this;
  // TODO
  vector<IR::Ptr> ret;
  jmp_revert = false;
  wrap_tie(condition_vec, body_access, condition_, symtab);
  wrap_tie(body_vec, tmp_access, body_, body_->symtab_);

  ret.push_back(std::make_shared<DstIR>(IR::Op::JMP, continue_access_));
  ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL, body_access));
  ret.insert(ret.end(), body_vec.begin(), body_vec.end());
  ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL, continue_access_));
  ret.insert(ret.end(), condition_vec.begin(), condition_vec.end());
  ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL, break_access_));

  now_while = temp;
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ReturnStmt::translate(SymbolTable::Ptr symtab) const {
  assert(now_func);

  vector<IR::Ptr> ret;
  if (ret_exp_) {
    wrap_tie(vec, access, ret_exp_, symtab);
    ret.insert(ret.end(), vec.begin(), vec.end());
    ret.push_back(std::make_shared<UnarySrcIR>(
        IR::Op::MOV, now_func->get_return_access(), access));
  }
  ret.push_back(std::make_shared<NoOpIR>(IR::Op::RET));
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
BreakStmt::translate(SymbolTable::Ptr symtab) const {
  assert(now_while);
  vector<IR::Ptr> ret;
  ret.push_back(std::make_shared<DstIR>(IR::Op::JMP, now_while->break_access_));
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ContinueStmt::translate(SymbolTable::Ptr symtab) const {
  assert(now_while);
  vector<IR::Ptr> ret;
  ret.push_back(
      std::make_shared<DstIR>(IR::Op::JMP, now_while->continue_access_));
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
AssignmentStmt::translate(SymbolTable::Ptr symtab) const {
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
        std::make_shared<UnarySrcIR>(IR::Op::MOV, entry.access_, access));
  }
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
FunctionDecl::translate(SymbolTable::Ptr symtab) const {
  symtab_->set_parent(symtab);
  body_->symtab_->set_parent(symtab_);
  const FunctionDecl *temp = now_func;
  now_func = this;

  vector<IR::Ptr> ret;
  ret.push_back(std::make_shared<DstIR>(IR::Op::LABEL,
                                        frame_->newLabelAccess(frame_, name_)));
  wrap_tie(vec, access, body_, body_->symtab_);
  ret.insert(ret.end(), vec.begin(), vec.end());

  now_func = temp;
  return std::make_tuple(ret, nullptr);
}
