#include "ast.h"
#include "ir.h"
#include "context/context.h"

#include <cassert>
#include <stack>

#define ADD_TRP(OP, A0, A1, A2) ret.emplace_back(IR::make_triple(IR::Op:: OP , A0, A1, A2))
#define ADD_BIN(OP, A0, A1    ) ret.emplace_back(IR::make_binary(IR::Op:: OP , A0, A1))
#define ADD_UNR(OP, A0        ) ret.emplace_back(IR::make_unary (IR::Op:: OP , A0))
#define ADD_NOP(OP            ) ret.emplace_back(IR::make_no_operand (IR::Op:: OP))

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
    CASE(LT,  MOVLT);
    CASE(GE,  MOVGE);
    CASE(LE,  MOVLE);
    CASE(EQ,  MOVEQ);
    CASE(NEQ, MOVNE);
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

// 获取多维数组的偏移量（编译期常量与否均可）
std::pair<IR::Addr::Ptr, std::list<IR::Ptr> >
get_offset(const std::vector<int> &shape, Expression::List &dimens) {
  std::list<IR::Ptr> ret;

  // 先计算各个表达式的值
  std::vector<IR::Addr::Ptr> dims;
  // 先计算各个下标表达式
  for (Expression *exp: dimens) {
    IR::Addr::Ptr addr = exp->get_var_addr();
    ret.splice(ret.end(), exp->translate());
    dims.push_back(addr);
  }  

  int acc = 1;
  int i = dims.size() - 1;

  // 常量优化：数组最右边的表达式均为常量
  // 例如：a[b+c][1][2][3][4]
  // 那么[1][2][3][4]的偏移量可以直接编译期计算
  int constant_offset = 0;
  for (; i>=0 ; i--) {
    // 如果取到非编译期常量的表达式，那么就跳出循环
    if (dims[i]->kind != IR::Addr::Kind::IMM) break;
    // 如果取到编译期常量
    constant_offset = constant_offset + acc*(dims[i]->val);
    acc *= shape[i];
  } 
  // 如果所有下标都计算完了，就直接返回常地址
  if (i==0) return std::make_pair(IR::Addr::make_imm(constant_offset), ret);
  // 常量优化结束
  
  // 再计算非编译期常数的偏移量
  // 为变址分配存储空间
  IR::Addr::Ptr offset_addr = IR::Addr::make_var(context.allocator.allocate_addr());

  // 先将常量变址计算结果拷贝过来
  ADD_BIN(MOV, offset_addr, IR::Addr::make_imm(constant_offset));
  // 再根据余下的下标计算变址（运行时计算）
  for (; i>=0 ; i--) {
    auto tmp = IR::Addr::make_var(context.allocator.allocate_addr());
    ADD_TRP(MUL, tmp, dims[i], IR::Addr::make_imm(acc));
    ADD_TRP(ADD, offset_addr, offset_addr, tmp);
    acc *= shape[i];
  }

  return std::make_pair(offset_addr, ret);
}

int
multiply(const std::vector<int> &v) {
  int ret = 1;
  for (auto &i: v) ret *= i;
  return ret;
}

bool
Expression::op_logical() const {
  return this->op_<=NOT;
}

bool
Expression::op_rel() const {
  return EQ<=this->op_ &&this->op_<=GE;
}

bool
Expression::translate_to_logical() const {
  return (this->op_logical()||this->op_rel()) ? !cast_to_regular : cast_to_logical;
}

IR::Addr::Ptr
Expression::get_var_addr() {
  if (this->addr_!=nullptr) return this->addr_;
  if (this->is_evaluable()) return this->addr_ = IR::Addr::make_imm(this->eval());
  return this->addr_ = IR::Addr::make_var(context.allocator.allocate_addr());
}

IR::Addr::Ptr
Expression::get_fail_label() {
  if (this->label_fail_!=nullptr) return this->label_fail_;
  return this->label_fail_ = IR::Addr::make_label(context.allocator.allocate_label());
}

void
Expression::set_fail_label(IR::Addr::Ptr label) {
  this->label_fail_ = label;
}

bool
VarExp::is_evaluable() const {
  auto ent = context.vartab_cur->get(this->ident_);
  // TODO 未定义变量
  assert(ent!=nullptr);

  // 首先检查（数组表达式的）下标是否编译期可求值
  if (ent->is_array()) {
    assert(this->dimens_!=nullptr);
    auto offset_pair = get_offset(ent->type.arr_shape, *(this->dimens_));
    if (offset_pair.first->kind != IR::Addr::IMM) return false;
  } 

  // 再检查变量本身是不是编译期常量
  return ent->is_constant;
}

int
VarExp::eval() {
  auto ent = context.vartab_cur->get(this->ident_);
  // TODO 未定义变量
  assert(ent!=nullptr);

  if (ent->is_array()) {
    assert(this->dimens_!=nullptr);
    // 该变量是数组
    // 首先获取下标
    auto offset_pair = get_offset(ent->type.arr_shape, *(this->dimens_));
    return ent->init_val[offset_pair.first->val];
  }

  return ent->init_val[0];
}

IR::Addr::Ptr
VarExp::get_var_addr() {
  if (this->addr_!=nullptr) return this->addr_;
  auto ent = context.vartab_cur->get(this->ident_);

  // TODO 变量未定义
  assert(ent!=nullptr);

  // 如果该表达式是可求值的，就给该表达式分配常量地址
  if (this->is_evaluable()) this->addr_ = IR::Addr::make_imm(this->eval());
  // 如果该表达式不可求值，且是数组型，就需要给该表达式分配一个临时变量，作为变址取数的目的地址
  else if (ent->is_array()) this->addr_ = IR::Addr::make_var(context.allocator.allocate_addr());
  // 表达式是不可求值的单一变量
  else this->addr_ = ent->addr;

  return this->addr_;
}

std::list<IR::Ptr>
VarExp::translate() {
  // 如果编译期可求值，就不翻译
  if (this->is_evaluable()) return std::list<IR::Ptr>();

  // 如果编译期不可求值
  std::list<IR::Ptr> ret;

  // 上级语法单位要求转为逻辑表达式
  // 翻译模式：生成临时表达式 (this==0)
  if (this->translate_to_logical()) {
    // 生成 (this==0)临时表达式
    BinaryExp *tmp = new BinaryExp(Op::EQ, this, new NumberExp(0));
    // 要求这个临时表达式转为逻辑表达式，并赋予本表达式的失败标号
    tmp->cast_to_logical = true;
    tmp->cast_to_regular = false;
    tmp->set_fail_label(this->label_fail_);

    // 翻译这个临时表达式，作为本表达式的翻译结果
    ret.splice(ret.end(), tmp->translate());

    // delete 掉这个临时表达式
    tmp->left_ = nullptr;  // 为防止析构掉 this
    delete tmp;
  }
  // 上级语法单位要求转为算术表达式
  // 翻译模式：分数组和单一变量两类
  else {
    // 查符号表
    auto ent = context.vartab_cur->get(this->ident_);
    assert(ent!=nullptr);

    // 如果是数组
    if (ent->is_array()) {
      // 计算数组下标
      auto offset_pair = get_offset(ent->type.arr_shape, *(this->dimens_));
      ret.splice(ret.end(), offset_pair.second);
      // 读取数组内容
      ADD_TRP(LOAD, this->addr_, ent->addr, offset_pair.first);
    }
    // 如果是单一变量
    // 那就啥也不干
  }

  return ret;
}

bool
FuncCallExp::is_evaluable() const {
  return false;
}

int
FuncCallExp::eval() {
  return 0;
}

IR::Addr::Ptr
FuncCallExp::get_var_addr() {
  return this->addr_ = IR::Addr::make_ret();
}

std::list<IR::Ptr>
FuncCallExp::translate() {
  std::list<IR::Ptr> ret;

  // 如果存在参数
  if (params_!=nullptr) {
    int i = 0;
    
    // 首先计算各个函数参数的值
    for (Expression *exp : *params_) {
      auto addr = exp->get_var_addr();
      ret.splice(ret.end(), exp->translate());
      ++i;
    }

    // 然后生成传参中间代码
    for (Expression *exp : *params_) ADD_UNR(PARAM, exp->get_var_addr());
  }

  // 生成函数调用的代码
  ADD_UNR(CALL, IR::Addr::make_named_label(this->name_));

  return ret;
}

bool
BinaryExp::is_evaluable() const {
  switch(this->op_) {
    case Expression::Op::AND:
      if (this->left_->is_evaluable() && this->left_->eval()==0) return true;
      if (this->right_->is_evaluable() && this->right_->eval()==0) return true;
      break;
    case Expression::Op::OR:
      if (this->left_->is_evaluable() && this->left_->eval()==1) return true;
      if (this->right_->is_evaluable() && this->right_->eval()==1) return true;
      break;
    default:
      break;
  }
  return this->left_->is_evaluable() && this->right_->is_evaluable();
}

int
BinaryExp::eval() {

#define CASE(EXPR_OP, ACTUAL_OP) case Expression::Op:: EXPR_OP : return this->left_->eval() ACTUAL_OP this->right_->eval()

  switch(this->op_) {
    case Expression::Op::AND:
      if (this->left_->is_evaluable() && this->left_->eval()==0) return 0;
      if (this->right_->is_evaluable() && this->right_->eval()==0) return 0;
      return 1;
      break;
    case Expression::Op::OR:
      if (this->left_->is_evaluable() && this->left_->eval()==1) return 1;
      if (this->right_->is_evaluable() && this->right_->eval()==1) return 1;
      return 0;
      break;
    CASE(ADD, +);
    CASE(SUB, -);
    CASE(MUL, *);
    CASE(DIV, /);
    CASE(MOD, %);
    default: return 0;  // TODO WARNING
  }

#undef CASE

}

IR::Addr::Ptr
BinaryExp::get_var_addr() {
  return Expression::get_var_addr();
}

/* 翻译模式：
 * 无论上级表达式要求本（逻辑）表达式翻译为逻辑型表达式还是数值型表达式
 * 都先按照逻辑型表达式的方式翻译
 * 如果要求本表达式转为算术表达式，那么就在逻辑型表达式的基础上，在条件为真、为假处加上 MOV #0或 MOV #1 的代码
 */
std::list<IR::Ptr>
BinaryExp::_translate_logical() {
  std::list<IR::Ptr> ret;

  // 要求子表达式都转为逻辑表达式
  this->left_->cast_to_logical = this->right_->cast_to_logical = true;
  this->left_->cast_to_regular = this->right_->cast_to_regular = false;

  // 如果上级表达式没有分配标号，则自己分配标号
  // 上级表达式要求转为算术表达式时，不会分配标号
  IR::Addr::Ptr lbl_fail = this->get_fail_label();

  // 根据表达式类型，选择不同翻译模式
  switch(this->op_) {
    case Expression::Op::AND:
      // 逻辑“与”

      // 常量优化部分
      // 左右表达式有一个为0时，就可以通过is_evaluable()和eval()来计算值=0
      if (this->left_->is_evaluable() && this->left_->eval()) {
        // 左表达式求值为“1”，右表达式不是编译期常量
        // 此时可看作右表达式的单元(Unary)表达式
        this->right_->set_fail_label(this->label_fail_);
        ret.splice(ret.end(), this->right_->translate());
      }
      else if (this->right_->is_evaluable() && this->right_->eval()) {
        // 右表达式求值为“1”，左表达式不是编译期常量
        // 此时可看作左表达式的单元(Unary)表达式
        this->left_->set_fail_label(this->label_fail_);
        ret.splice(ret.end(), this->left_->translate());
      }
      // 常量优化部分结束

      else {
        // if (lhs) <do nothing>; else goto fail;
        // if (rhs) <do nothing>; else goto fail;
        // ... <codes that execute on success>
        // fail:
        // ... <codes that execute on fail>
        this->left_->set_fail_label(this->label_fail_);
        this->right_->set_fail_label(this->label_fail_);

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
        this->right_->set_fail_label(this->label_fail_);
        ret.splice(ret.end(), this->right_->translate());
      }  
      else if (this->right_->is_evaluable() && !(this->right_->eval())) {
        // 右表达式求值为“0”，左表达式不是编译期常量
        // 此时可看作左表达式的单元表达式
        this->left_->set_fail_label(this->label_fail_);
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

        this->left_->set_fail_label(IR::Addr::make_label(lhs_fail_label));
        this->right_->set_fail_label(this->label_fail_);

        ret.splice(ret.end(), this->left_->translate());
        ADD_UNR(JMP,   IR::Addr::make_label(lhs_success_label));
        ADD_UNR(LABEL, IR::Addr::make_label(lhs_fail_label));
        ret.splice(ret.end(), this->right_->translate());
        ADD_UNR(LABEL, IR::Addr::make_label(lhs_success_label));
      }
      break;
    default:
      break;
  }

  // 如果上级表达式要求本表达式转为算术表达式，则加入以下代码
  if (!(this->translate_to_logical())) {
    // mov <this address> , imm(1)
    // jmp end
    // fail:
    // mov <this address>, imm(0)
    // end:
    int end_label = context.allocator.allocate_label();

    ADD_BIN(MOV, this->addr_, IR::Addr::make_imm(1));
    ADD_UNR(JMP, IR::Addr::make_label(end_label));
    ADD_UNR(LABEL, this->label_fail_);
    ADD_BIN(MOV, this->addr_, IR::Addr::make_imm(0));
    ADD_UNR(LABEL, IR::Addr::make_label(end_label));
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
  auto l_addr = this->left_->get_var_addr();
  auto r_addr = this->right_->get_var_addr();

  // 生成计算子表达式的代码
  ret.splice(ret.end(), this->left_->translate());
  ret.splice(ret.end(), this->right_->translate());

  // 比较子表达式
  ADD_BIN(CMP, l_addr, r_addr);

  if (this->translate_to_logical()) {
    // 要求翻译为逻辑表达式
    // 因为是失败时跳转，所以需要反义操作符
    ret.emplace_back(IR::make_unary(get_ir_jmp_op(reverse_op(this->op_)), this->label_fail_));
  }
  else {
    // 要求翻译为算术表达式
    // 为假，就移0进去（加了个reverse）
    ret.emplace_back(IR::make_binary(get_ir_mov_op(reverse_op(this->op_)), this->addr_, IR::Addr::make_imm(0)));
    // 为真，就移1进去
    ret.emplace_back(IR::make_binary(get_ir_mov_op(this->op_), this->addr_, IR::Addr::make_imm(1)));
  }

  return ret;
}

std::list<IR::Ptr>
BinaryExp::_translate_regular() {
  std::list<IR::Ptr> ret;
    
  // 上级表达式要求本表达式生成一个逻辑表达式
  // 那么就直接生成一个临时性的关系表达式 (this==0) ，利用它生成中间代码
  if (this->translate_to_logical()) {
    BinaryExp *tmp = new BinaryExp(Expression::Op::EQ, this, new NumberExp(0));
    tmp->cast_to_logical = true;
    tmp->cast_to_regular = false;
    tmp->set_fail_label(this->label_fail_);

    ret.splice(ret.end(), tmp->translate());

    // 防止析构 this
    tmp->left_ = nullptr;
    delete tmp;
  }
  else {
    // 要求生成一个算术表达式
    // 如果左/右子表达式不可编译期求值，就给左右表达式分配变量地址
    auto l_addr = this->left_->get_var_addr();
    auto r_addr = this->right_->get_var_addr();

    // 生成计算左右表达式的代码
    ret.splice(ret.end(), this->left_->translate());
    ret.splice(ret.end(), this->right_->translate());

    // 综合计算左右表达式
    ret.emplace_back(IR::make_triple(get_ir_regular_op(this->op_), this->addr_, l_addr, r_addr));
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

bool
UnaryExp::is_evaluable() const {
  return this->exp_->is_evaluable();
}

int
UnaryExp::eval() {
  switch(this->op_) {
    case Expression::Op::NOT:
      return !(this->exp_->eval());
    case Expression::Op::ADD:
      return this->exp_->eval();
    case Expression::Op::SUB:
      return -(this->exp_->eval());
    default:
      return 0;  // TODO 报错
  }
}

IR::Addr::Ptr
UnaryExp::get_var_addr() {
  return Expression::get_var_addr();
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

    // 子表达式的fail标志就是本表达式的success标志
    auto this_success = this->exp_->get_fail_label();

    ret.splice(ret.end(), this->exp_->translate());

    ADD_UNR(JMP, this->label_fail_);
    ADD_UNR(LABEL, this_success);
  } 
  else {
    // 要求子表达式翻译为算术表达式
    this->exp_->cast_to_regular = true;
    this->exp_->cast_to_logical = false;

    // 为子表达式分配存储空间
    auto sub_addr = this->exp_->get_var_addr();

    // 计算子表达式值
    ret.splice(ret.end(), this->exp_->translate());

    // cmp subexp->addr, 0
    // moveq this->addr, 1
    // movne this->addr, 0
    ADD_BIN(CMP, this->exp_->get_var_addr(), IR::Addr::make_imm(0));
    ADD_BIN(MOVEQ, this->addr_, IR::Addr::make_imm(1));
    ADD_BIN(MOVNE, this->addr_, IR::Addr::make_imm(0));
  }

  return ret;
}

std::list<IR::Ptr>
UnaryExp::_translate_regular() {
  std::list<IR::Ptr> ret;

  if (this->translate_to_logical()) {
    // 要求生成逻辑表达式
    // 直接生成一个临时性的关系表达式，利用它生成中间代码
    BinaryExp *tmp = new BinaryExp(Expression::Op::EQ, this, new NumberExp(0));

    tmp->cast_to_logical = true;
    tmp->cast_to_regular = false;
    tmp->set_fail_label(this->label_fail_);

    ret.splice(ret.end(), tmp->translate());

    tmp->left_ = nullptr;
    delete tmp;
  }
  else {
    // 要求生成算术表达式
    this->exp_->cast_to_logical = false;
    this->exp_->cast_to_regular = true;

    auto sub_addr = this->exp_->get_var_addr();

    ret.splice(ret.end(), this->exp_->translate());
    ADD_BIN(MOV, this->addr_, sub_addr);
    if (Expression::Op::SUB)
      ADD_TRP(SUB, this->addr_, IR::Addr::make_imm(0), this->addr_);
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

bool
NumberExp::is_evaluable() const {
  return true;
}

int
NumberExp::eval() {
  return this->value_;
}

IR::Addr::Ptr
NumberExp::get_var_addr() {
  return IR::Addr::make_imm(this->value_);
}

std::list<IR::Ptr>
NumberExp::translate() {
  // 常量，无需翻译
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
Variable::_translate_param() {
  this->vartab_ent = std::shared_ptr<VarTabEntry>(new VarTabEntry(
    this->name_,
    std::vector<int>(),
    IR::Addr::make_param(this->param_no),
    std::vector<int>(),
    false
  ));
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
Variable::_translate_local() {
  std::list<IR::Ptr> ret;

  IR::Addr::Ptr addr;
  std::vector<int> init_val;

  if (this->immutable_) {
    assert(this->initval_!=nullptr);
    assert(this->initval_->is_evaluable());
    int val = this->initval_->eval();
    addr = IR::Addr::make_imm(val);
    init_val.push_back(val);
  }
  else {
    addr = IR::Addr::make_var(context.allocator.allocate_addr());
    if (this->initval_) {
      auto initval_addr = this->initval_->get_var_addr();
      ret.splice(ret.end(), this->initval_->translate());
      ADD_BIN(MOV, addr, initval_addr);
    }
  }

  this->vartab_ent = std::shared_ptr<VarTabEntry>(new VarTabEntry(
    this->name_,
    std::vector<int>(),
    addr,
    init_val,
    this->immutable_
  ));

  return ret;
}

std::list<IR::Ptr>
Variable::_translate_global() {
  std::list<IR::Ptr> ret;

  IR::Addr::Ptr addr;
  std::vector<int> init_val;

  addr = IR::Addr::make_named_label(this->name_);

  // 处理初始值
  // 如果有初始值，其必须为编译期常量
  if (this->initval_!=nullptr) {
    assert(this->initval_->is_evaluable());
    init_val.push_back(this->initval_->eval());
  }
  else {
    init_val.push_back(0);
  }

  ADD_UNR(VARDEF, addr);
  ADD_UNR(DATA, IR::Addr::make_imm(init_val[0]));
  ADD_NOP(VAREND);

  this->vartab_ent = std::shared_ptr<VarTabEntry>(new VarTabEntry(
    this->name_,
    std::vector<int>(),
    addr,
    init_val,
    this->immutable_
  ));

  return ret;
}

std::list<IR::Ptr>
Variable::translate() {
  std::list<IR::Ptr> ret;

  if (this->param_no!=-1)     ret.splice(ret.end(), this->_translate_param());
  else if (this->is_global()) ret.splice(ret.end(), this->_translate_global());
  else                        ret.splice(ret.end(), this->_translate_local());

  return ret;
}

std::vector<int>
Array::_get_shape() {
  std::vector<int> ret;

  // 检查表达式是否都是编译期常量，并计算
  // TODO 报错
  for (Expression *i: *(this->dimens_)) {
    assert(i->is_evaluable());
    ret.push_back(i->eval());
  }

  return ret;
}

Expression::List
Array::_flatten_initval(const std::vector<int> &shape, int shape_ptr, InitValContainer *container) {
  Expression::List ret;

  if (container==nullptr) return ret;

  // 如果已经处理完最低维数组，后面还有数据，就特殊处理（递归边界）
  if (shape_ptr>=shape.size()) {
    for (InitVal *val: container->initval_container_) {
      // 直接在ret中无限制地生成
      if (val->is_exp()) {
        InitValExp *exp = static_cast<InitValExp*>(val);
        // 否则，继续循环
        ret.push_back(exp->exp_);
      }
      else {
        InitValContainer *sub_container = static_cast<InitValContainer*>(val);
        // 否则调用
        ret.splice(ret.end(), this->_flatten_initval(shape, shape_ptr, sub_container));
      }
    }
    return ret;
  }
  
  //     int a[x][y][z]
  // shape_ptr ^
  // 这个函数调用负责生成 a[0][y][z], a[1][y][z], ... , a[x-1][y][z]的初值
  int required_size = 1;
  for (int i=shape_ptr+1 ; i<shape.size() ; i++) required_size *= shape[i];
  
  for (InitVal *val: container->initval_container_) {
    // 如果val是表达式，就直接按照数组展平的方式填充
    if (val->is_exp()) {
      InitValExp *exp = static_cast<InitValExp*>(val);
      // 如果表达式数量超过需求量，就忽略后面的表达式
      if (ret.size()>=required_size*shape[shape_ptr]) break;
      // 否则，继续循环
      ret.push_back(exp->exp_);
    }
    // 如果val是列表容器，就递归地生成
    else {
      InitValContainer *sub_container = static_cast<InitValContainer*>(val);
      // 首先对齐边界
      while (ret.size()%required_size) ret.push_back(nullptr);
      // 如果表达式数量超过需求量，就忽略后面的表达式
      if (ret.size()>=required_size*shape[shape_ptr]) break;
      // 否则递归调用
      Expression::List tmp = this->_flatten_initval(shape, shape_ptr+1, sub_container);
      if (tmp.size()<=required_size) {
        ret.splice(ret.end(), tmp);
        while (ret.size()%required_size) ret.push_back(nullptr);
      }
      else for (int i=0 ; i<required_size ; i++) ret.splice(ret.end(), tmp, tmp.begin());
    }
  }

  // 补满余下的
  while (ret.size()<required_size*shape[shape_ptr]) ret.push_back(nullptr);

  return ret;
}

std::list<IR::Ptr>
Array::_translate_param() {
  std::vector<int> shape = this->_get_shape();
  this->vartab_ent = std::make_shared<VarTabEntry>(
    this->name_,
    shape,
    IR::Addr::make_param(this->param_no),
    std::vector<int>(),
    false
  );
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
Array::_translate_local() {
  std::list<IR::Ptr> ret;
  std::vector<int> init_val;
  std::vector<int> shape = this->_get_shape();
  int total_size = multiply(shape);

  // 若存在初始值，就将初始值先展开成一维
  Expression::List flattened_container = this->_flatten_initval(shape, 0, this->initval_container_);
    
  // 栈上分配空间
  IR::Addr::Ptr addr = IR::Addr::make_var(context.allocator.allocate_addr());
  ADD_BIN(ALLOC_IN_STACK, addr, IR::Addr::make_imm(total_size));
  
  int offset = 0;
  // 如果是常值，就需要指定初值
  if (this->immutable_) {
    assert(flattened_container.size()!=0);
    // 要求所有初值都是编译期常量
    for (Expression *exp: flattened_container) {
      if (exp==nullptr) continue;
      assert(exp->is_evaluable());
    }

    for (Expression *exp: flattened_container) {
      int val = exp->eval();
      init_val.push_back(val);
      ADD_TRP(STORE, IR::Addr::make_imm(val), addr, IR::Addr::make_imm(offset));
      offset++;
    }
  }
  else for (Expression *exp: flattened_container) {
    IR::Addr::Ptr exp_addr;
    if (exp==nullptr) exp_addr = IR::Addr::make_imm(0);
    else {
      exp_addr = exp->get_var_addr();
      ret.splice(ret.end(), exp->translate());
    }
    ADD_TRP(STORE, exp_addr, addr, IR::Addr::make_imm(offset));
    offset++;
  }

  // 建立符号表项
  this->vartab_ent = std::shared_ptr<VarTabEntry>(new VarTabEntry(
    this->name_,
    shape,
    addr,
    init_val,
    this->immutable_
  ));

  return ret;
}

std::list<IR::Ptr>
Array::_translate_global() {
  std::list<IR::Ptr> ret;
  std::vector<int> shape = this->_get_shape();
  IR::Addr::Ptr addr = IR::Addr::make_named_label(this->name_);
  
  // 若存在初始值，就将初始值先展开成一维
  Expression::List flattened_container = this->_flatten_initval(shape, 0, this->initval_container_);
  
  // 检查初值序列是否为常量
  // 如果指定了初值，初值就必须均为编译期常量
  for (Expression *exp: flattened_container) {
    if (exp==nullptr) continue;
    assert(exp->is_evaluable());
  }
  
  // 生成中间代码
  ADD_UNR(VARDEF, addr);
  
  // 未指定初值
  if (flattened_container.size()==0) {
    int total_size = multiply(shape);
    ADD_UNR(ZERO, IR::Addr::make_imm(total_size));
  }
  // 已经指定初值
  else {
    // 记录零的个数，减少代码量
    int z_num = 0;
    for (Expression *exp: flattened_container) {
      int val = exp==nullptr ? 0 : exp->eval();
    
      if (val==0) {
        z_num++;
      }
      else {
        if (z_num!=0) ADD_UNR(ZERO, IR::Addr::make_imm(z_num));
        z_num = 0;
        ADD_UNR(DATA, IR::Addr::make_imm(val));
      }
    }
    if (z_num!=0) ADD_UNR(ZERO, IR::Addr::make_imm(z_num));
  }
  ADD_NOP(VAREND);

  this->vartab_ent = std::shared_ptr<VarTabEntry>(new VarTabEntry(
    this->name_,
    shape,
    addr,
    std::vector<int>(),
    this->immutable_
  ));

  return ret;
}

std::list<IR::Ptr>
Array::translate() {
  std::list<IR::Ptr> ret;

  if (this->param_no!=-1)     ret.splice(ret.end(), this->_translate_param());
  else if (this->is_global()) ret.splice(ret.end(), this->_translate_global());
  else                        ret.splice(ret.end(), this->_translate_local());

  return ret;
}

std::list<IR::Ptr>
VarDeclStmt::translate() {
  std::list<IR::Ptr> ret;
  for (Variable *var : this->vars_) {
    ret.splice(ret.end(), var->translate());
    context.vartab_cur->put(var->vartab_ent);
  }
  return ret;
}

std::list<IR::Ptr>
ExpStmt::translate() {
  // 没有 ++等影响变量值的操作，不翻译
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
BlockStmt::translate() {
  std::list<IR::Ptr> ret;
  // 建立块作用域
  context.new_scope();

  // 将预定义符号（函数参数）加入符号表中
  for (auto i: this->pre_defined) context.vartab_cur->put(i);
  // 再逐条翻译语句
  for (Stmt *s: this->stmts_) ret.splice(ret.end(), s->translate());

  // 结束块作用域
  context.end_scope();
  return ret;
}

std::list<IR::Ptr>
IfStmt::translate() {
  // TODO 报错
  assert(this->condition_);

  // 常量优化
  if (this->condition_->is_evaluable()) {
    if (this->condition_->eval()) return this->yes_->translate();
    // else block may not exist
    else if (this->no_)           return this->no_->translate();   
  }

  std::list<IR::Ptr> ret;
  
  // 翻译模式
  //   if (stmt) <do nothing>; else goto label_else;
  // ... <true expressions>
  //   jmp label_end
  // label_else:
  // ... <false expressions>
  // label_end:

  // 为条件表达式分配标号
  int label_else = context.allocator.allocate_label();
  int label_end  = context.allocator.allocate_label();
  this->condition_->set_fail_label(IR::Addr::make_label(label_else));

  // 翻译条件表达式
  this->condition_->cast_to_logical = true;
  this->condition_->cast_to_regular = false;
  ret.splice(ret.end(), this->condition_->translate());

  // true expressions
  ret.splice(ret.end(), this->yes_->translate());
  // jmp label_end
  ADD_UNR(JMP, IR::Addr::make_label(label_end));
  // set label label_else
  ADD_UNR(LABEL, IR::Addr::make_label(label_else));
  // false expressions: else may not exist
  if (this->no_!=nullptr) ret.splice(ret.end(), this->no_->translate());
  // set label label_end
  ADD_UNR(LABEL, IR::Addr::make_label(label_end));

  return ret;
}

std::list<IR::Ptr>
WhileStmt::translate() {
  std::list<IR::Ptr> ret;

  context.while_chain.push_back(this);

  // 为自己分配标号
  this->label_cond = IR::Addr::make_label(context.allocator.allocate_label());
  this->label_end  = IR::Addr::make_label(context.allocator.allocate_label());

  // 常量优化
  if (this->condition_->is_evaluable()) {
    if (this->condition_->eval()) {
      // 无限循环
      ADD_UNR(LABEL, this->label_cond);
      ret.splice(ret.end(), this->body_->translate());
      ADD_UNR(JMP, this->label_cond);
      ADD_UNR(LABEL, this->label_end);
    } else {
      // 不执行
    }
  }
  // 常量优化结束

  else {
    // 为条件表达式分配标号
    this->condition_->set_fail_label(this->label_end);

    // 翻译条件表达式
    ADD_UNR(LABEL, this->label_cond);
    ret.splice(ret.end(), this->condition_->translate());
    ret.splice(ret.end(), this->body_->translate());
    ADD_UNR(JMP, this->label_cond);
    ADD_UNR(LABEL, this->label_end);
  }

  context.while_chain.pop_back();
  return ret;
}

std::list<IR::Ptr>
ReturnStmt::translate() {
  std::list<IR::Ptr> ret;

  IR::Addr::Ptr retexp_addr = nullptr;
  if (this->ret_exp_) retexp_addr = this->ret_exp_->get_var_addr();
  else                retexp_addr = IR::Addr::make_imm(0);

  if (this->ret_exp_) ret.splice(ret.end(), ret_exp_->translate());
  ADD_UNR(RET, retexp_addr);

  return ret;
}

std::list<IR::Ptr>
BreakStmt::translate() {
  std::list<IR::Ptr> ret;
  assert(!(context.while_chain.empty()));
  
  ADD_UNR(JMP, context.while_chain.back()->label_end);

  return ret;
}

std::list<IR::Ptr>
ContinueStmt::translate() {
  std::list<IR::Ptr> ret;
  assert(!(context.while_chain.empty()));
  
  ADD_UNR(JMP, context.while_chain.back()->label_cond);

  return ret;
}

std::list<IR::Ptr>
AssignmentStmt::translate() {
  std::list<IR::Ptr> ret;
  auto ent = context.vartab_cur->get(this->name_);

  // TODO 未定义符号、符号是常数
  assert(ent!=nullptr);
  assert(!(ent->is_constant));

  // 计算右表达式的值
  auto rval_addr = this->rval_->get_var_addr();
  ret.splice(ret.end(), this->rval_->translate());

  if (ent->is_array()) {
    const std::vector<int> &shape = ent->type.arr_shape;
    // 数组下标与形状不一致
    assert(shape.size()==this->dimens_->size());

    // 根据数组形状计算偏移量
    auto offset_pair = get_offset(shape, *(this->dimens_));
    ret.splice(ret.end(), offset_pair.second);

    ADD_TRP(STORE, rval_addr, ent->addr, offset_pair.first);
  }
  else {
    // 不是数组
    ADD_BIN(MOV, ent->addr, rval_addr);
  }

  return ret;
}

std::list<IR::Ptr>
FunctionDecl::translate() {
  std::list<IR::Ptr> ret;

  int cnt = 0;
  if (this->params_) for (Variable *i: *(this->params_)) {
    i->param_no = cnt++;
    i->translate();
    auto i_ent = i->vartab_ent;
    this->body_->pre_defined.push_back(i_ent);
  }

  ADD_UNR(FUNCDEF, IR::Addr::make_named_label(this->name_));
  ret.splice(ret.end(), this->body_->translate());
  ADD_NOP(FUNCEND);

  return ret;
}
