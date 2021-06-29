#include "ast.h"
#include "ir.h"
#include "context/context.h"

#include <cassert>
#include <stack>

#define ADD_TRP(OP, A0, A1, A2) ret.emplace_back(IR::make_triple(IR::Op:: OP , A0, A1, A2))
#define ADD_BIN(OP, A0, A1    ) ret.emplace_back(IR::make_binary(IR::Op:: OP , A0, A1))
#define ADD_UNR(OP, A0        ) ret.emplace_back(IR::make_unary (IR::Op:: OP , A0))
#define ADD_NOP(OP, A0        ) ret.emplace_back(IR::make_no_operand (IR::Op:: OP))

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

// 获取多维数组的偏移量（编译期可求值的）
int
get_offset(const std::vector<int> &shape, const std::vector<int> &dimens) {
  int offset = dimens.back();
  int acc    = shape.back();

  for (int i=dimens.size()-2 ; i>0 ; i--) {
    offset += (acc*dimens[i]);
    acc    *= shape[i];
  }

  return offset;
}

// 获取多维数组的偏移量（编译期不可求值的）
std::pair<IR::Addr::Ptr, std::list<IR::Ptr> >
get_offset(const std::vector<int> &shape, Expression::List &dimens) {
  std::list<IR::Ptr> ret;

  // 先计算各个表达式的值
  std::vector<IR::Addr::Ptr> dims;
  // 先解析各个下标表达式
  for (Expression *exp: dimens) {
    IR::Addr::Ptr addr = exp->get_var_addr();
    ret.splice(ret.end(), exp->translate());
    dims.push_back(addr);
  }
  
  // 再根据数组形状计算偏移量
  IR::Addr::Ptr offset_addr = IR::Addr::make_var(context.allocator.allocate_addr());
  ADD_BIN(MOV, offset_addr, dims.back());

  int acc = shape.back();
  for (int i=dims.size()-2 ; i>0 ; i++) {
    ADD_TRP(MUL, dims[i], dims[i], IR::Addr::make_imm(acc));
    ADD_TRP(ADD, offset_addr, offset_addr, dims[i]);
    acc *= shape[i];
  }

  return std::make_pair(offset_addr, ret);
}

IR::Addr::Ptr
Expression::get_fail_label() {
  if (this->label_fail_!=nullptr) return this->label_fail_;
  return IR::Addr::make_label(context.allocator.allocate_label());
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
  if (this->dimens_!=nullptr) for (Expression *each_index: *(this->dimens_)) {
    if (each_index->is_evaluable()) continue;
    return false;
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
    // 该变量是数组
    // 首先获取下标
    std::vector<int> dims;
    for (Expression *exp: *(this->dimens_)) dims.push_back(exp->eval());
    int offset = get_offset(ent->type.arr_shape, dims);
  }
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

  // 转为逻辑表达式
  if (this->translate_to_logical()) {
    // 生成临时表达式 (this==0)
    BinaryExp *tmp = new BinaryExp(Op::EQ, this, new NumberExp(0));
    
    // 要求这个临时表达式转为逻辑表达式，赋予本表达式的失败标号
    tmp->cast_to_logical = true;
    tmp->cast_to_regular = true;
    tmp->set_fail_label(this->label_fail_);

    ret.splice(ret.end(), tmp->translate());

    // 防止析构掉 this
    tmp->left_ = nullptr;
    delete tmp;
  }
  // 转为算术表达式
  else {
    auto ent = context.vartab_cur->get(this->ident_);
    assert(ent!=nullptr);

    // 如果是数组
    if (ent->is_array()) {
      auto offset_rel = get_offset(ent->type.arr_shape, *(this->dimens_));
      ret.splice(ret.end(), offset_rel.second);
      ADD_TRP(LOAD, this->addr_, ent->addr, offset_rel.first);
    }
    // 如果是单一变量
    else {
      this->addr_ = ent->addr;
    }
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
  if (this->addr_!=nullptr) return this->addr_;
  return (this->addr_=IR::Addr::make_var(context.allocator.allocate_addr()));
}

std::list<IR::Ptr>
FuncCallExp::translate() {
  std::list<IR::Ptr> ret;

  if (params_) {
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

  // TODO warning when call an undeclared function
  // 最后生成函数调用的代码
  ADD_UNR(CALL, IR::Addr::make_named_label(this->name_));

  return ret;
}

bool
BinaryExp::is_evaluable() const {
  return this->left_->is_evaluable() && this->right_->is_evaluable();
}

int
BinaryExp::eval() {

#define CASE(EXPR_OP, ACTUAL_OP) case Expression::Op:: EXPR_OP : return this->left_->eval() ACTUAL_OP this->right_->eval()

  switch(this->op_) {
    CASE(AND, &&);
    CASE(OR,  ||);
    CASE(ADD, +);
    CASE(SUB, -);
    CASE(MUL, *);
    CASE(DIV, /);
    CASE(MOD, %);
    default: return 0;  // TODO WARNING
  }

#undef CASE

}

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
  switch(this->op()) {
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
        this->right_->set_fail_label(this->label_fail);

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

  // 如果表达式被要求转为算术表达式，则加入以下代码
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
    ret.emplace_back(std::make_shared<IR>(get_ir_jmp_op(reverse_op(this->op())), this->label_fail_));
  }
  else {
    // 要求翻译为算术表达式
    // 为假，就移0进去（加了个reverse）
    ret.emplace_back(IR::make_binary(get_ir_mov_op(reverse_op(this->op())), this->addr_, IR::Addr::make_imm(0)));
    // 为真，就移1进去
    ret.emplace_back(IR::make_binary(get_ir_mov_op(this->op()), this->addr_, IR::Addr::make_imm(1)));
  }

  return ret;
}

std::list<IR::Ptr>
BinaryExp::_translate_regular() {
  std::list<IR::Ptr> ret;

  if (this->translate_to_logical()) {
    // 要求生成一个逻辑表达式
    // 那么就直接生成一个临时性的关系表达式，利用它生成中间代码
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
    // 如果左/右子表达式不可编译期求值，就给左右表达式分配变量地址，并生成计算左/右子表达式的代码
    auto l_addr = this->left_->get_var_addr();
    auto r_addr = this->right_->get_var_addr();

    // 生成计算左右表达式的代码
    ret.splice(ret.end(), this->left_->translate());
    ret.splice(ret.end(), this->right_->translate());

    // 综合计算左右表达式
    ret.emplace_back(IR::make_binary(get_ir_regular_op(this->op()), l_addr, r_addr));
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

std::list<IR::Ptr>
NumberExp::translate() {
  // 常量，无需翻译
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
Variable::_translate_immutable() {
  std::list<IR::Ptr> ret;

  // 常量要求初始化，且必须用编译期常量初始化
  // TODO 错误处理
  assert(this->initval_!=nullptr);
  assert(this->initval_->is_evaluable());

  // 处理初始化值
  std::vector<int> init_val = {this->initval_->eval()};

  // 建立符号表项
  this->vartab_ent = std::make_shared<VarTabEntry>(
    this->name(),
    std::vector<int>(),   // shape
    nullptr,              // addr
    std::move(init_val),  // initial value
    true
  );

  return ret;
}

std::list<IR::Ptr>
Variable::_translate_variable() {
  std::list<IR::Ptr> ret;

  // 先分配地址
  int var_addr = context.allocator.allocate_addr();

  // 再处理初始值
  std::vector<int> init_val;
  if (this->initval_!=nullptr) {
    auto init_val_addr = this->initval_->get_var_addr();
    if (init_val_addr->kind==IR::Addr::Kind::IMM) init_val.push_back(init_val_addr->val);

    ret.splice(ret.end(), this->initval_->translate());
    ADD_BIN(MOV, IR::Addr::make_var(var_addr), init_val_addr);
  }
  
  // 建立符号表项
  this->vartab_ent = std::make_shared<VarTabEntry>(
    this->name(),
    std::vector<int>(),            // shape
    IR::Addr::make_var(var_addr),  // addr
    std::move(init_val),           // init_val
    false
  );
  
  return ret;
}

std::list<IR::Ptr>
Variable::translate() {
  std::list<IR::Ptr> ret;

  if (this->immutable()) ret.splice(ret.end(), this->_translate_immutable());
  else                   ret.splice(ret.end(), this->_translate_variable());

  return ret;
}

std::vector<int>
Array::_get_shape() {
  std::vector<int> ret;

  // 检查表达式是否都是常数，并计算
  // TODO 报错
  for (Expression *i: *(this->dimens_)) {
    assert(i->is_evaluable());
    ret.push_back(i->eval());
  }

  return ret;
}

std::vector<int>
Array::_get_const_initval(const std::vector<int> &shape, int shape_ptr, InitValContainer *container) {
  std::vector<int> ret;

  // 如果只余下一个维度，就特殊处理
  if (shape_ptr+1==shape.size()) {
    // 循环DFS
    // TODO 可以报个警告，说初始化表达式深度过大
    std::stack<std::tuple<InitValContainer*, int> > iter_stack;
    iter_stack.push(std::make_tuple(container, 0));
    while (!(iter_stack.empty())) {
      InitValContainer *now_container;
      int now_start_pos;
      
      // 解包tuple
      std::tie(now_container, now_start_pos) = iter_stack.top();
      iter_stack.pop();

      for (int i=now_start_pos ; i<now_container->initval_container_.size() ; i++) {
        if (now_container->initval_container_[i]->is_exp()) {
          InitValExp *exp = static_cast<InitValExp*>(now_container->initval_container_[i]);
          // 如果表达式数量超过需求量，就忽略后面的表达式
          if (ret.size()>=shape[shape_ptr]) goto loop_end;
          // TODO 要求表达式是常量，报错
          assert(exp->exp_->is_evaluable());
          // 加入表达式
          ret.push_back(exp->exp_->eval());
        }
        else {
          InitValContainer *sub_container = static_cast<InitValContainer*>(now_container->initval_container_[i]);
          // 如果表达式数量超过需求量，就忽略后面的表达式
          if (ret.size()>=shape[shape_ptr]) goto loop_end;
          
          iter_stack.push(std::make_tuple(now_container, i+1));
          iter_stack.push(std::make_tuple(sub_container, 0));
          break;
        }
      }
    }
    loop_end:
    while (ret.size()<shape[shape_ptr]) ret.push_back(0);
    return ret;
  }

  //     int a[x][y][z]
  // shape_ptr ^
  // 这个函数调用负责生成 a[0][y][z], a[1][y][z], ... , a[x-1][y][z]的初值
  int required_size = 1;
  for (int i=shape_ptr+1 ; i<shape.size() ; i++) required_size *= shape[i];
  
  for (InitVal *val: container->initval_container_) {
    if (val->is_exp()) {
      // 如果val是表达式，就直接按照数组展平的方式填充

      InitValExp *exp = static_cast<InitValExp*>(val);
      // 如果表达式数量超过需求量，就忽略后面的表达式
      if (ret.size()>=required_size*shape[shape_ptr]) break;

      // TODO 要求表达式是常量，报错
      assert(exp->exp_->is_evaluable());
      // 加入表达式
      ret.push_back(exp->exp_->eval());
    }
    else {
      // 如果val是列表容器，就递归生成

      InitValContainer *sub_container = static_cast<InitValContainer*>(val);
      // 首先对齐边界
      while (ret.size()%required_size) ret.push_back(0);
      // 如果表达式数量超过需求量，就忽略后面的表达式
      if (ret.size()>=required_size*shape[shape_ptr]) break;

      // 否则递归调用
      auto tmp = this->_get_const_initval(shape, shape_ptr+1, sub_container);
    }
  }

  // 补满余下的
  while (ret.size()<required_size*shape[shape_ptr]) ret.push_back(0);

  return ret;
}

std::list<IR::Ptr>
Array::_translate_immutable() {
  std::list<IR::Ptr> ret;

  // 获取数组形状
  std::vector<int> shape = this->_get_shape();

  // 再生成初值序列
  std::vector<int> init_val = this->_get_const_initval(shape, 0, this->initval_container_);

  // 建立符号表项
  this->vartab_ent = std::make_shared<VarTabEntry>(
    this->name_,
    shape,
    nullptr,
    init_val,
    true
  );
  
  return ret;
}

std::list<IR::Ptr>
Array::translate() {
  std::list<IR::Ptr> ret;

  if (this->immutable()) ret.splice(ret.end(), this->_translate_immutable());
  else                   ret.splice(ret.end(), this->_translate_variable());

  return ret;
}

std::list<IR::Ptr>
VarDeclStmt::translate() {
  std::list<IR::Ptr> ret;
  for (Variable *var : this->vars_) ret.splice(ret.end(), var->translate());
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
    else                          return this->no_->translate();   
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
  // false expressions
  ret.splice(ret.end(), this->no_->translate());
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
    i->translate();
    auto i_ent = i->vartab_ent;
    i_ent->param_order = cnt++;
    this->body_->pre_defined.push_back(i_ent);
  }

  ADD_UNR(FUNCDEF, IR::Addr::make_named_label(this->name_));
  ret.splice(ret.end(), this->body_->translate());
  ADD_NOP(FUNCEND);

  return ret;
}
