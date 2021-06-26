#include "ast.h"
#include "ir.h"
#include "../context/context.h"

#include <cassert>
static FunctionDecl *now_func;
static WhileStmt *now_while;
static bool jmp_revert = false;

// helper: get correspond IR::Op of Expr::Op
IR::Op get_ir_jmp_op(Expression::Op expr_op) {

#define CASE(EXPR_OP, IR_OP) case Expression::Op:: EXPR_OP : return IR::Op:: IR_OP

  switch(expr_op) {
    CASE(GT,  JGT);
    CASE(LT,  JLT);
    CASE(GE,  JGE);
    CASE(LE,  JLE);
    CASE(EQ,  JE);
    CASE(NEQ, JNE);
    default: return IR::Op::NOP;
  }

#undef CASE

}

// helper: get correspond IR::Op of Expr::Op
IR::Op get_ir_mov_op(Expression::Op expr_op) {

#define CASE(EXPR_OP, IR_OP) case Expression::Op:: EXPR_OP : return IR::Op:: IR_OP

  switch(expr_op) {
    CASE(GT,  MOVGT);
    CASE(LT,  JLT);
    CASE(GE,  JGE);
    CASE(LE,  JLE);
    default: return IR::Op::NOP;
  }

#undef CASE

}

// helper: get correspond IR::Op of Expr::Op
IR::Op get_ir_regular_op(Expression::Op expr_op) {

#define CASE(EXPR_OP, IR_OP) case Expression::Op:: EXPR_OP : return IR::Op:: IR_OP

  switch(expr_op) {
    CASE(ADD, ADD);
    CASE(SUB, SUB);
    CASE(MUL, MUL);
    CASE(DIV, DIV);
    CASE(MOD, MOD);
    default: return IR::Op::NOP;
  }

#undef CASE

}

// helper: get correspond IR::Op of Expr::Op

// helper: reverse Expr::Op
Expression::Op reverse_op(Expression::Op op) {

#define CASE(OP, REV_OP) case Expression::Op:: OP : return Expression::Op:: REV_OP

  switch(op) {
    CASE(GT, LE);
    CASE(GE, LT);
    CASE(LE, GT);
    CASE(LT, GE);
    CASE(EQ, NEQ);
    CASE(NEQ, EQ);
    default: return op;
  }

#undef CASE

}

std::list<IR::Ptr>
VarExp::translate() {
  // 先从符号表取出对应表项
  auto entry = context.vartab_cur->get(this->ident_);
  // 然后从表项中取出虚地址，作为该语法单位的地址
  this->addr = entry->address_number;
}

std::list<IR::Ptr>
FuncCallExp::translate() {
  std::list<IR::Ptr> ret;

  if (params_) {
    int i = 0;
    
    // 首先计算各个函数参数的值
    for (Expression *exp : *params_) {
      exp->cast_to_regular = true;
      ret.splice(ret.end(), exp->translate());
      ++i;
    }

    // 然后生成传参中间代码
    for (Expression *exp : *params_) {
      auto param_spec_ir = std::make_shared<SingalOpIR>(IR::Op::PARAM);
      // TODO p_s_i.setAddress(exp.address);
      ret.emplace_back(std::move(param_spec_ir));
    }
  }

  // 最后生成函数调用的代码
  ret.emplace_back(std::make_shared<SingalOpIR>(IR::Op::CALL));

  return ret;
}

std::list<IR::Ptr>
BinaryExp::_translate_logical() {
  std::list<IR::Ptr> ret;

  // 要求子表达式都转为逻辑表达式
  this->left_->cast_to_logical = this->right_->cast_to_logical = true;
  this->left_->cast_to_regular = this->right_->cast_to_regular = false;

  // 如果上级表达式没有分配标号，则自己分配标号
  // 上级表达式要求转为算术表达式时，不会分配标号
  if (this->label_fail==-1) this->label_fail = context.allocator.allocate_label();

  // 根据表达式类型，选择不同翻译模式
  switch(this->op()) {
    case Expression::Op::AND:
      // 逻辑“与”

      // 常量优化部分
      if (this->left_->is_evaluable() && this->left_->eval()) {
        // 左表达式求值为“1”，右表达式不是编译期常量
        // 此时可看作右表达式的单元(Unary)表达式
        this->right_->label_fail = this->label_fail;
        ret.splice(ret.end(), this->right_->translate());
      }
      else if (this->right_->is_evaluable() && this->right_->eval()) {
        // 右表达式求值为“1”，左表达式不是编译期常量
        // 此时可看作左表达式的单元(Unary)表达式
        this->left_->label_fail = this->label_fail;
        ret.splice(ret.end(), this->left_->translate());
      }
      // 常量优化部分结束

      else {
        // if (lhs) <do nothing>; else goto fail;
        // if (rhs) <do nothing>; else goto fail;
        // ... <codes that execute on success>
        // fail:
        // ... <codes that execute on fail>
        this->left_->label_fail  = this->label_fail;
        this->right_->label_fail = this->label_fail;

        ret.splice(ret.end(), this->left_->translate());
        ret.splice(ret.end(), this->right_->translate());
      }

      break;
    case Expression::Op::OR:
      // 逻辑“或”：

      // 常量优化部分
      if (this->left_->is_evaluable() && !(this->left_->eval())) {
        // 左表达式求值为“0”，右表达式不是编译期常量
        // 此时可看作右表达式的单元表达式
        this->right_->label_fail = this->label_fail;
        ret.splice(ret.end(), this->right_->translate());
      }  
      else if (this->right_->is_evaluable() && !(this->right_->eval())) {
        // 右表达式求值为“0”，左表达式不是编译期常量
        // 此时可看作左表达式的单元表达式
        this->left_->label_fail = this->label_fail;
        ret.splice(ret.end(), this->left_->translate());
      }
      // 常量优化部分结束

      else {
        //   if (lhs) <do nothing>; else goto lhs_fail_label;
        //   goto lhs_success_label;
        // lhs_fail_label:
        //   if (rhs) <do nothing>; else goto fail;
        // lhs_success_label:
        // ... <codes that execute on success>
        // fail:
        // ... <codes that execute on fail>
        int lhs_fail_label    = context.allocator.allocate_label();
        int lhs_success_label = context.allocator.allocate_label();

        this->left_->label_fail  = context.allocator.allocate_label();
        this->right_->label_fail = this->label_fail;

        ret.splice(ret.end(), this->left_->translate());

        ret.emplace_back(
          std::make_shared<SingalOpIR>(
            IR::Op::JMP,
            IR_Addr::make_label(lhs_success_label)
          )
        );
        ret.emplace_back(
          std::make_shared<SingalOpIR>(
            IR::Op::LABEL,
            IR_Addr::make_label(lhs_fail_label)
          )
        );

        ret.splice(ret.end(), this->right_->translate());
        
        ret.emplace_back(
          std::make_shared<SingalOpIR>(
            IR::Op::LABEL,
            IR_Addr::make_label(lhs_success_label)
          )
        );
      }
      break;
    default:
      this->left_->label_fail  = -1;
      this->right_->label_fail = -1;
      break;
  }

  // 如果表达式被要求转为算术表达式，则加入以下代码
  if (this->cast_to_regular) {
    // mov <this address> , imm(1)
    // jmp end
    // fail:
    // mov <this address>, imm(0)
    // end:
    int end_label = context.allocator.allocate_label();

    ret.emplace_back(std::make_shared<UnaryOpIR>(
        IR::Op::MOV, IR_Addr::make_var(this->addr), IR_Addr::make_imm(1)
    ));
    ret.emplace_back(std::make_shared<SingalOpIR>(
        IR::Op::JMP, IR_Addr::make_label(end_label)
    ));
    ret.emplace_back(std::make_shared<SingalOpIR>(
        IR::Op::LABEL, IR_Addr::make_label(this->label_fail)
    ));
    ret.emplace_back(std::make_shared<UnaryOpIR>(
        IR::Op::MOV, IR_Addr::make_var(this->addr), IR_Addr::make_imm(0)
    ));
    ret.emplace_back(std::make_shared<SingalOpIR>(
        IR::Op::LABEL, IR_Addr::make_label(end_label)
    ));
  }

  return ret;
}

std::list<IR::Ptr>
BinaryExp::_translate_rel() {
  std::list<IR::Ptr> ret;

  // 要求左右子表达式都转为算术表达式
  this->left_->cast_to_logical = this->right_->cast_to_logical = false;
  this->left_->cast_to_regular = this->right_->cast_to_regular = true;

  // 如果左/右子表达式不可编译期求值，就给左右表达式分配变量地址，并生成计算左/右子表达式的代码
  if (!(this->left_->is_evaluable()))  {
    this->left_->addr = (this->left_->addr==-1) ? context.allocator.allocate_addr() : this->left_->addr;
    ret.splice(ret.end(), this->left_->translate());
  }
  if (!(this->right_->is_evaluable())) {
    this->right_->addr = (this->right_->addr==-1) ? context.allocator.allocate_addr() : this->right_->addr;
    ret.splice(ret.end(), this->right_->translate());
  }

  // 比较子表达式
  ret.emplace_back(
    std::make_shared<BinOpIR>(
      IR::Op::CMP,
      // 地址生成思路：如果表达式可以求值，就直接翻译为立即数地址，否则翻译为表达式的变量地址
      this->left_->is_evaluable()  ? IR_Addr::make_imm(this->left_->eval())  : IR_Addr::make_var(this->left_->addr),
      this->right_->is_evaluable() ? IR_Addr::make_imm(this->right_->eval()) : IR_Addr::make_var(this->right_->addr)
    )
  );

  if (this->translate_to_logical()) {
    // 要求翻译为逻辑表达式
    ret.emplace_back(
      std::make_shared<SingalOpIR>(
        // 因为是失败时跳转，所以需要反义操作符
        get_ir_jmp_op(reverse_op(this->op())),
        IR_Addr::make_label(this->label_fail)
      )
    );
  }
  else {
    // 要求翻译为算术表达式

    // 为假，就移0进来
    ret.emplace_back(
      std::make_shared<UnaryOpIR>(
        // 此处加了个reserve，表示为假时的情况
        get_ir_mov_op(reverse_op(this->op())),
        IR_Addr::make_label(this->addr),
        IR_Addr::make_imm(0)
      )
    );

    // 为真，就移1进来
    ret.emplace_back(
      std::make_shared<UnaryOpIR>(
        get_ir_mov_op(this->op()),
        IR_Addr::make_label(this->addr),
        IR_Addr::make_imm(1)
      )
    );
  }

  return ret;
}

std::list<IR::Ptr>
BinaryExp::_translate_regular() {
  std::list<IR::Ptr> ret;

  if (this->translate_to_logical()) {
    // 要求生成一个逻辑表达式
    // 那么就直接生成一个临时性的关系表达式，利用它生成中间代码
    BinaryExp *temp = new BinaryExp(Expression::Op::EQ, this, new NumberExp(0));
    temp->label_fail = this->label_fail;

    ret.splice(ret.end(), temp->translate());

    temp->left_ = nullptr;
    delete temp;
  }
  else {
    // 要求生成一个算术表达式
    // 如果左/右子表达式不可编译期求值，就给左右表达式分配变量地址，并生成计算左/右子表达式的代码
    if (!(this->left_->is_evaluable()))  {
      this->left_->addr = (this->left_->addr==-1) ? context.allocator.allocate_addr() : this->left_->addr;
      ret.splice(ret.end(), this->left_->translate());
    }
    if (!(this->right_->is_evaluable())) {
      this->right_->addr = (this->right_->addr==-1) ? context.allocator.allocate_addr() : this->right_->addr;
      ret.splice(ret.end(), this->right_->translate());
    }

    // 综合计算左右表达式
    ret.emplace_back(
      std::make_shared<BinOpIR>(
        get_ir_regular_op(this->op()),
        // 地址生成思路：如果表达式可以求值，就直接翻译为立即数地址，否则翻译为表达式的变量地址
        this->left_->is_evaluable()  ? IR_Addr::make_imm(this->left_->eval())  : IR_Addr::make_var(this->left_->addr),
        this->right_->is_evaluable() ? IR_Addr::make_imm(this->right_->eval()) : IR_Addr::make_var(this->right_->addr)
      )
    );
  }

  return ret;
}

std::list<IR::Ptr>
BinaryExp::translate() {
  // 当编译期可求表达式的值时，就拒绝翻译
  if (this->is_evaluable()) return std::list<IR::Ptr>();

  std::list<IR::Ptr> ret;

  if (this->op_logical()) {
    // &&, ||
    ret.splice(ret.end(), this->_translate_logical());
  } else if (this->op_rel()) {
    // ==, !=, <, <=, >, >=
    ret.splice(ret.end(), this->_translate_rel());
  } else {
    // +, -, *, /, %
    ret.splice(ret.end(), this->_translate_regular());
  }

  return ret;
}

std::list<IR::Ptr>
UnaryExp::_translate_logical() {
  std::list<IR::Ptr> ret;

  if (this->translate_to_logical()) {
    //   if (subexp) <do nothing>; else goto this_success_label;
    //   goto this_fail_label;
    // this_success_label:
    // ... <codes that execute on success>
    // this_fail_label:
    // ... <codes that execute on fail>

    // 要求子表达式翻译为逻辑表达式
    this->exp_->cast_to_regular = false;
    this->exp_->cast_to_logical = true;

    int this_success = context.allocator.allocate_label();
    this->exp_->label_fail = this_success;

    ret.splice(ret.end(), this->exp_->translate());

    ret.emplace_back(std::make_shared<SingalOpIR>(
      IR::Op::JMP, IR_Addr::make_label(this->label_fail)
    ));
    ret.emplace_back(std::make_shared<SingalOpIR>(
      IR::Op::LABEL, IR_Addr::make_label(this_success)
    ));
  } 
  else {
    // 要求子表达式翻译为算术表达式
    this->exp_->cast_to_regular = true;
    this->exp_->cast_to_logical = false;

    // 为子表达式分配存储空间
    this->exp_->addr = (this->exp_->addr==-1) ? context.allocator.allocate_addr() : this->exp_->addr;
    this->exp_->addr = context.allocator.allocate_addr();

    // 计算子表达式值
    ret.splice(ret.end(), this->exp_->translate());

    // cmp subexp->addr, 0
    // moveq this->addr, 1
    // movne this->addr, 0
    ret.emplace_back(std::make_shared<UnaryOpIR>(
      IR::Op::CMP, IR_Addr::make_var(this->exp_->addr), IR_Addr::make_imm(0)
    ));
    ret.emplace_back(std::make_shared<SingalOpIR>(
      IR::Op::MOVEQ, IR_Addr::make_label(this->addr), IR_Addr::make_imm(1)
    ));
    ret.emplace_back(std::make_shared<SingalOpIR>(
      IR::Op::MOVEQ, IR_Addr::make_label(this->addr), IR_Addr::make_imm(0)
    ));
  }

  return ret;
}

std::list<IR::Ptr>
UnaryExp::_translate_regular() {
  std::list<IR::Ptr> ret;

  if (this->translate_to_logical()) {
    // 要求生成逻辑表达式
    // 直接生成一个临时性的关系表达式，利用它生成中间代码
    BinaryExp *temp = new BinaryExp(Expression::Op::EQ, this, new NumberExp(0));
    temp->label_fail = this->label_fail;

    ret.splice(ret.end(), temp->translate());

    temp->left_ = nullptr;
    delete temp;
  }
  else {
    // 要求生成算术表达式
    this->exp_->addr = this->addr;
    ret.splice(ret.end(), this->exp_->translate());

    if (Expression::Op::SUB) {
      ret.emplace_back(std::make_shared<BinaryOpIR>(
        IR::Op::SUB, IR_Addr::make_imm(0), IR_Addr::make_var(this->addr);
      ));
    }
  }

  return ret;
}

std::list<IR::Ptr>
UnaryExp::translate() {
  // 当编译期可求表达式的值时，就拒绝翻译
  if (this->is_evaluable()) return std::list<IR::Ptr>();

  std::list<IR::Ptr> ret;

  if (this->op_logical()) {
    // !
    ret.splice(ret.end(), this->_translate_logical());
  } else {
    // +, -
    ret.splice(ret.end(), this->_translate_regular());
  }

  return ret;
}

std::list<IR::Ptr>
NumberExp::translate() {
  // 常量，无需翻译
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
VarDeclStmt::translate() {
  std::list<IR::Ptr> ret;
  for (Variable *var : vars_) {
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
  LogicExp *condition = dynamic_cast<LogicExp *>(condition_);
  if (!condition) {
    return std::make_tuple(vector<IR::Ptr>(), nullptr);
  }
  vector<IR::Ptr> ret;
  FrameAccess next_label = symtab->frame()->newLabelAccess(symtab->frame());

  jmp_revert = false;
  wrap_tie(condition_vec, yes_label, condition, symtab);
  ret.insert(ret.end(), condition_vec.begin(), condition_vec.end());
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
  // TODO
  vector<IR::Ptr> ret;
  jmp_revert = false;
  wrap_tie(condition_vec, body_access, condition_, symtab);
  wrap_tie(body_vec, tmp_access, body_, body_->symtab_);

  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::JMP, continue_access_));
  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::LABEL, body_access));
  ret.insert(ret.end(), body_vec.begin(), body_vec.end());
  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::LABEL, continue_access_));
  ret.insert(ret.end(), condition_vec.begin(), condition_vec.end());
  ret.push_back(std::make_shared<SingalOpIR>(IR::Op::LABEL, break_access_));

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
  vector<IR::Ptr> ret;
  ret.push_back(
      std::make_shared<SingalOpIR>(IR::Op::JMP, parent_->break_access_));
  return std::make_tuple(ret, nullptr);
}

std::tuple<vector<IR::Ptr>, FrameAccess>
ContinueStmt::translate(SymbolTable::Ptr symtab) {
  assert(now_while);
  parent_ = now_while;
  vector<IR::Ptr> ret;
  ret.push_back(
      std::make_shared<SingalOpIR>(IR::Op::JMP, parent_->continue_access_));
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
  ret.push_back(std::make_shared<SingalOpIR>(
      IR::Op::LABEL, frame_->newLabelAccess(frame_, name_)));
  wrap_tie(vec, access, body_, body_->symtab_);
  ret.insert(ret.end(), vec.begin(), vec.end());

  now_func = temp;
  return std::make_tuple(ret, nullptr);
}
