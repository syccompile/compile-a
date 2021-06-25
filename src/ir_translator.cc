#include "ir_translator.h"
#include "ast.h"
#include "ir.h"

#include <cassert>
#include <iostream>

#define vec_push_all1(vec, first)                                              \
  vec.insert(vec.end(), first.begin(), first.end())

#define vec_push_all2(vec, first, second)                                      \
  vec_push_all1(vec, first);                                                   \
  vec_push_all1(vec, second);

#define vec_push_ir(vec, type, op, ...)                                        \
  vec.push_back(std::make_shared<type>(IR::Op::op, ##__VA_ARGS__));

static FunctionDecl *now_func;
static WhileStmt *now_while;
/**
 *  @Variable jmp_revert
 *  对于一般的条件表达式，例如a > 3，使用jmp_revert控制跳转
 *  条件，例如if(a > 3)，可以翻译成
 *      CMP a , 3
 *      JGT .L1
 *      # no
 *  .L1:
 *      # yes
 *  也可以翻译成
 *      CMP a , 3
 *      JLE .L1
 *      # yes
 *  .L1:
 *      # no
 *  如果jmp_revert == true, 使用下一种翻译方法，默认使用上一种
 */
static bool jmp_revert = false;

/** 
 * @Variable arich2logic
 * 将算术表达式视为逻辑表达式生成中间代码
 * 只影响最高层运算符，见函数BinaryExp::translate
 */
static bool arith2logic = false;

std::tuple<vector<IR::Ptr>, FrameAccess>
VarExp::translate(SymbolTable::Ptr symtab) {
  // TODO: 考虑数组
  SymbolTable::SymTabEntry entry = symtab->find(ident_);
  Frame::Ptr frame = symtab->frame();
  if (arith2logic) {
    vector<IR::Ptr> ret;
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), entry.access_, frame->newImmAccess(frame, 0));
    if (jmp_revert) {
      FrameAccess jmp = frame->newLabelAccess(frame);
      vec_push_ir(ret, SingalOpIR, JE, jmp);
      return std::make_tuple(ret, jmp);
    } else {
      FrameAccess jmp = frame->newLabelAccess(frame);
      vec_push_ir(ret, SingalOpIR, JNE, jmp);
      return std::make_tuple(ret, jmp);
    }
  }
  if (entry.pointer_.var_ptr->immutable()) {
    return std::make_tuple(vector<IR::Ptr>(), entry.access_);
  }
  return std::make_tuple(vector<IR::Ptr>(), entry.access_);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
FuncCallExp::translate(SymbolTable::Ptr symtab) {
  // TODO: arith2logic
  vector<IR::Ptr> ret;
  SymbolTable::SymTabEntry entry = symtab->find(name_);
  if (params_) {
    size_t i = 0;
    for (Expression *exp : *params_) {
      wrap_tie(vec, access, exp, symtab);
      vec_push_all1(ret, vec);
      vec_push_ir(ret, UnaryOpIR, MOV, entry.pointer_.func_ptr->get_params_access(i), access);
      i++;
    }
  }
  vec_push_ir(ret, SingalOpIR, CALL, entry.access_);
  return std::make_tuple(ret, entry.pointer_.func_ptr->get_return_access());
}

std::tuple<vector<IR::Ptr>, FrameAccess>
logic_translate(BinaryExp *exp, SymbolTable::Ptr symtab) {
  // TODO: 考虑evaluable() == true
  vector<IR::Ptr> ret;
  Frame::Ptr frame = symtab->frame();
  FrameAccess result;
  if (exp->op_ == Expression::Op::AND) {
    if (jmp_revert) {
      jmp_revert = true;
      wrap_tie_logic(lhs_vec, lhs_access, exp->left_, symtab);
      wrap_tie_logic(rhs_vec, rhs_access, exp->right_, symtab);
      vec_push_all2(ret, lhs_vec, rhs_vec);
      rhs_access->copy(lhs_access);
      return std::make_tuple(ret, lhs_access);
    } else {
      jmp_revert = true;
      wrap_tie_logic(lhs_vec, lhs_access, exp->left_, symtab);
      jmp_revert = false;
      wrap_tie_logic(rhs_vec, rhs_access, exp->right_, symtab);
      vec_push_all2(ret, lhs_vec, rhs_vec);
      vec_push_ir(ret, SingalOpIR, LABEL, lhs_access);
      return std::make_tuple(ret, rhs_access);
    }
  } else if (exp->op_ == Expression::Op::OR) {
    if (jmp_revert) {
      jmp_revert = true;
      wrap_tie_logic(lhs_vec, lhs_access, exp->left_, symtab);
      jmp_revert = false;
      wrap_tie_logic(rhs_vec, rhs_access, exp->right_, symtab);
      vec_push_all2(ret, lhs_vec, rhs_vec);
      vec_push_ir(ret, SingalOpIR, LABEL, lhs_access);
      return std::make_tuple(ret, rhs_access);
    } else {
      jmp_revert = false;
      wrap_tie_logic(lhs_vec, lhs_access, exp->left_, symtab);
      wrap_tie_logic(rhs_vec, rhs_access, exp->right_, symtab);
      vec_push_all2(ret, lhs_vec, rhs_vec);
      rhs_access->copy(lhs_access);
      return std::make_tuple(ret, lhs_access);
    }
  }
  wrap_tie_arith(lhs_vec, lhs_access, exp->left_, symtab);
  wrap_tie_arith(rhs_vec, rhs_access, exp->right_, symtab);
  switch (exp->op_) {
  case Expression::Op::LE:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access, rhs_access);
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JGT, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JLE, result);
    }
    break;
  case Expression::Op::GE:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access, rhs_access);
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JLT, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JGE, result);
    }
    break;
  case Expression::Op::LT:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access, rhs_access);
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JGE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JLT, result);
    }
  case Expression::Op::GT:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access, rhs_access);
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JLE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JGT, result);
    }
    break;
  case Expression::Op::EQ:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access, rhs_access);
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JNE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JE, result);
    }
    break;
  case Expression::Op::NEQ:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access, rhs_access);
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JNE, result);
    }
  case Expression::Op::ADD:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    {
      FrameAccess tmp = frame->newTempAccess(frame);
      vec_push_ir(ret, BinOpIR, ADD, tmp, lhs_access, rhs_access);
      vec_push_ir(ret, BinOpIR, TEST, frame->newTempAccess(frame), tmp, tmp);
      result = frame->newLabelAccess(frame);
    }
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JNE, result);
    }
    break;
  case Expression::Op::SUB:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    vec_push_ir(ret, BinOpIR, CMP, frame->newLabelAccess(frame), lhs_access, rhs_access);
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JNE, result);
    }
    break;
  case Expression::Op::MUL:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    {
      FrameAccess tmp = frame->newTempAccess(frame);
      vec_push_ir(ret, BinOpIR, MUL, tmp, lhs_access, rhs_access);
      vec_push_ir(ret, BinOpIR, TEST, frame->newTempAccess(frame), tmp, tmp);
    }
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JNE, result);
    }
    break;
  case Expression::Op::DIV:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    {
      FrameAccess tmp = frame->newTempAccess(frame);
      vec_push_ir(ret, BinOpIR, DIV, tmp, lhs_access, rhs_access);
      vec_push_ir(ret, BinOpIR, TEST, frame->newTempAccess(frame), tmp, tmp);
    }
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JNE, result);
    }
    break;
  case Expression::Op::MOD:
    vec_push_all2(ret, lhs_vec, rhs_vec);
    {
      FrameAccess tmp = frame->newTempAccess(frame);
      vec_push_ir(ret, BinOpIR, MOD, tmp, lhs_access, rhs_access);
      vec_push_ir(ret, BinOpIR, TEST, frame->newTempAccess(frame), tmp, tmp);
    }
    result = frame->newLabelAccess(frame);
    if (jmp_revert) {
      vec_push_ir(ret, SingalOpIR, JE, result);
    } else {
      vec_push_ir(ret, SingalOpIR, JNE, result);
    }
    break;
  default:
    break;
  }
  return std::make_tuple(ret, result);
}
std::tuple<vector<IR::Ptr>, FrameAccess>
arithmetic_translate(BinaryExp *exp, SymbolTable::Ptr symtab) {
  // TODO : 考虑evaluable() == true;
  vector<IR::Ptr> ret;
  Frame::Ptr frame = symtab->frame();
  // FIX: 现在的计算还有些问题
  if (exp->evaluable()) {
    return std::make_tuple(
        vector<IR::Ptr>(),
        frame->newImmAccess(frame, exp->eval()));
  }
  FrameAccess result = frame->newTempAccess(frame);

  wrap_tie(lhs_vec, lhs_access, exp->left_, symtab);
  wrap_tie(rhs_vec, rhs_access, exp->right_, symtab);
  vec_push_all2(ret, lhs_vec, rhs_vec);
  IR::Ptr ir;
  switch (exp->op_) {
  case Expression::Op::ADD:
    vec_push_ir(ret, BinOpIR, ADD, result, lhs_access, rhs_access);
    break;
  case Expression::Op::SUB:
    vec_push_ir(ret, BinOpIR, SUB, result, lhs_access, rhs_access);
    break;
  case Expression::Op::MUL:
    vec_push_ir(ret, BinOpIR, MUL, result, lhs_access, rhs_access);
    break;
  case Expression::Op::DIV:
    vec_push_ir(ret, BinOpIR, DIV, result, lhs_access, rhs_access);
    break;
  case Expression::Op::MOD:
    vec_push_ir(ret, BinOpIR, MOD, result, lhs_access, rhs_access);
    break;
  case Expression::Op::LT:
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access,
                rhs_access);
    vec_push_ir(ret, SingalOpIR, SETLT, result);
    break;
  case Expression::Op::LE:
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access,
                rhs_access);
    vec_push_ir(ret, SingalOpIR, SETLE, result);
    break;
  case Expression::Op::GT:
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access,
                rhs_access);
    vec_push_ir(ret, SingalOpIR, SETGT, result);
    break;
  case Expression::Op::GE:
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access,
                rhs_access);
    vec_push_ir(ret, SingalOpIR, SETGE, result);
    break;
  case Expression::Op::EQ:
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access,
                rhs_access);
    vec_push_ir(ret, SingalOpIR, SETE, result);
    break;
  case Expression::Op::NEQ:
    vec_push_ir(ret, BinOpIR, CMP, frame->newTempAccess(frame), lhs_access,
                rhs_access);
    vec_push_ir(ret, SingalOpIR, SETNE, result);
    break;

  
  default:
    assert(false);
    break;
  }
  return std::make_tuple(ret, result);
}
std::tuple<vector<IR::Ptr>, FrameAccess>
BinaryExp::translate(SymbolTable::Ptr symtab) {
  if (arith2logic) {
    arith2logic = false;
    return logic_translate(this, symtab);
  } else {
    return arithmetic_translate(this, symtab);
  }
}

std::tuple<vector<IR::Ptr>, FrameAccess>
UnaryExp::translate(SymbolTable::Ptr symtab) {
  vector<IR::Ptr> ret;
  Frame::Ptr frame = symtab->frame();
  // FIX
  if (evaluable()) {
    return std::make_tuple(vector<IR::Ptr>(), frame->newImmAccess(
                                                  frame, eval()));
  }
  FrameAccess result = frame->newTempAccess(frame);

  IR::Ptr ir;
  switch (op_) {
  case Op::ADD: 
    if(arith2logic){
    }else{
      if(evaluable()){
        return std::make_tuple(vector<IR::Ptr>(), frame->newImmAccess(
                                                    frame, eval()));
      }
      wrap_tie(vec, access, exp_, symtab);
      result = access;
    } 
    break;
  case Op::SUB: {
    wrap_tie(vec, access, exp_, symtab);
    // TODO: 优化多个减号, int a = -----3;
    vec_push_all1(ret, vec);
    vec_push_ir(ret, BinOpIR, SUB, result, frame->newImmAccess(frame, 0), access);
  } break;
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
NumberExp::translate(SymbolTable::Ptr symtab) {
  return std::make_tuple(vector<IR::Ptr>(), symtab->frame()->newImmAccess(
                                                symtab->frame(), value_));
}

std::tuple<vector<IR::Ptr>, FrameAccess>
VarDeclStmt::translate(SymbolTable::Ptr symtab) {
  vector<IR::Ptr> ret;
  for (Variable *var : vars_) {
    FrameAccess access = symtab->push(var);
    if (var->initialized() && !var->global()) {
      if (var->is_array()) {
        // TODO 数组声明
      } else {
        wrap_tie(rhs_vec, rhs_access, var->initval_, symtab);
        vec_push_all1(ret, rhs_vec);
        vec_push_ir(ret, UnaryOpIR, MOV, access, rhs_access);
      }
    } else if (var->initialized() && var->global()) {
      // TODO 全局变量 
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
    vec_push_all1(ret, vec);
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
  FrameAccess next_label = symtab->frame()->newLabelAccess(symtab->frame());

  jmp_revert = false;
  wrap_tie_logic(condition_vec, yes_label, condition_, symtab);
  vec_push_all1(ret, condition_vec);
  if (no_) {
    wrap_tie(no_vec, access, no_, no_->symtab_);
    vec_push_all1(ret, no_vec);
  }
  vec_push_ir(ret, SingalOpIR, JMP, next_label);

  vec_push_ir(ret, SingalOpIR, LABEL, yes_label);
  wrap_tie(yes_vec, yes_access, yes_, yes_->symtab_);
  vec_push_all1(ret, yes_vec);
  vec_push_ir(ret, SingalOpIR, LABEL, next_label);
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
WhileStmt::translate(SymbolTable::Ptr symtab) {
  body_->symtab_->set_parent(symtab);
  WhileStmt *temp = now_while;
  now_while = this;
  // FIX: 使用其他的栈帧结构
  vector<IR::Ptr> ret;
  jmp_revert = false;
  wrap_tie_logic(condition_vec, body_access, condition_, symtab);
  wrap_tie(body_vec, tmp_access, body_, body_->symtab_);

  vec_push_ir(ret, SingalOpIR, JMP, continue_access_);
  vec_push_ir(ret, SingalOpIR, LABEL, body_access);
  vec_push_all1(ret, body_vec);
  vec_push_ir(ret, SingalOpIR, LABEL, continue_access_);
  vec_push_all1(ret, condition_vec);
  vec_push_ir(ret, SingalOpIR, LABEL, break_access_);

  now_while = temp;
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ReturnStmt::translate(SymbolTable::Ptr symtab) {
  assert(now_func);
  parent_ = now_func;

  vector<IR::Ptr> ret;
  if (ret_exp_) {
    wrap_tie(vec, access, ret_exp_, symtab);
    vec_push_all1(ret, vec);
    vec_push_ir(ret, UnaryOpIR, MOV, parent_->get_return_access(), access);
  }
  vec_push_ir(ret, NoOpIR, RET);
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
BreakStmt::translate(SymbolTable::Ptr symtab) {
  assert(now_while);
  parent_ = now_while;
  vector<IR::Ptr> ret;
  vec_push_ir(ret, SingalOpIR, JMP, parent_->break_access_);
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ContinueStmt::translate(SymbolTable::Ptr symtab) {
  assert(now_while);
  parent_ = now_while;
  vector<IR::Ptr> ret;
  vec_push_ir(ret, SingalOpIR, JMP, parent_->continue_access_);
  return std::make_tuple(ret, nullptr);
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
    vec_push_all1(ret, vec);
    vec_push_ir(ret, UnaryOpIR, MOV, entry.access_, access);
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
  vec_push_ir(ret, SingalOpIR, LABEL, frame_->newLabelAccess(frame_, name_));
  wrap_tie(vec, access, body_, body_->symtab_);
  vec_push_all1(ret, vec);

  now_func = temp;
  return std::make_tuple(ret, nullptr);
}
