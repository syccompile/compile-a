#include "ast.h"
#include "ir.h"
#include "context/context.h"

#include <cassert>
#include <stack>

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

std::list<IR::Ptr>
VarExp::translate() {
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
FuncCallExp::translate() {
  std::list<IR::Ptr> ret;

  if (params_) {
    int i = 0;
    
    // 首先计算各个函数参数的值
    for (Expression *exp : *params_) {
      // 常量优化
      if (exp->is_evaluable()) {
        // 遇到可编译期求值的表达式，就跳过计算
        exp->addr = IR::Addr::make_imm(exp->eval());
        continue;
      }
      
      // 否则，就给这个表达式分配一个地址
      exp->addr = (exp->addr==nullptr) ?
        IR::Addr::make_var(context.allocator.allocate_addr()) :
        exp->addr;

      exp->cast_to_regular = true;
      exp->cast_to_logical = false;

      ret.splice(ret.end(), exp->translate());
      ++i;
    }

    // 然后生成传参中间代码
    for (Expression *exp : *params_) {
      // TODO p_s_i.setAddress(exp.address);
      ret.emplace_back(IR::make_unary(
        IR::Op::PARAM,
        exp->addr
      ));
    }
  }

  // TODO warning when call an undeclared function
  // 最后生成函数调用的代码
  ret.emplace_back(IR::make_unary(
    IR::Op::CALL,
    IR::Addr::make_named_label(this->name_)
  ));

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
  if (this->label_fail==nullptr)
    this->label_fail = IR::Addr::make_label(context.allocator.allocate_label());

  // 根据表达式类型，选择不同翻译模式
  switch(this->op()) {
    case Expression::Op::AND:
      // 逻辑“与”

      // 常量优化部分
      // 左右表达式有一个为0时，就可以通过is_evaluable()和eval()来计算值=0
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

        this->left_->label_fail  = IR::Addr::make_label(lhs_fail_label);
        this->right_->label_fail = this->label_fail;

        ret.splice(ret.end(), this->left_->translate());

        ret.emplace_back(IR::make_unary(
          IR::Op::JMP,
          IR::Addr::make_label(lhs_success_label)
        ));
        ret.emplace_back(IR::make_unary(
          IR::Op::LABEL,
          IR::Addr::make_label(lhs_fail_label)
        ));

        ret.splice(ret.end(), this->right_->translate());
        
        ret.emplace_back(IR::make_unary(
          IR::Op::LABEL,
          IR::Addr::make_label(lhs_success_label)
        ));
      }
      break;
    default:
      this->left_->label_fail  = nullptr;
      this->right_->label_fail = nullptr;
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

    ret.emplace_back(IR::make_binary(
        IR::Op::MOV, this->addr, IR::Addr::make_imm(1)
    ));
    ret.emplace_back(IR::make_unary(
        IR::Op::JMP, IR::Addr::make_label(end_label)
    ));
    ret.emplace_back(IR::make_unary(
        IR::Op::LABEL, this->label_fail
    ));
    ret.emplace_back(IR::make_binary(
        IR::Op::MOV, this->addr, IR::Addr::make_imm(0)
    ));
    ret.emplace_back(IR::make_unary(
        IR::Op::LABEL, IR::Addr::make_label(end_label)
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
    this->left_->addr = (this->left_->addr==nullptr) ?
      IR::Addr::make_var(context.allocator.allocate_addr()):
      this->left_->addr;
    ret.splice(ret.end(), this->left_->translate());
  } else {
    this->left_->addr = IR::Addr::make_imm(this->left_->eval());
  }

  if (!(this->right_->is_evaluable())) {
    this->right_->addr = (this->right_->addr==nullptr) ?
      IR::Addr::make_var(context.allocator.allocate_addr()):
      this->right_->addr;
    ret.splice(ret.end(), this->right_->translate());
  } else {
    this->right_->addr = IR::Addr::make_imm(this->right_->eval());
  }

  // 比较子表达式
  ret.emplace_back(IR::make_binary(
    IR::Op::CMP,
    // 地址生成思路：如果表达式可以求值，就直接翻译为立即数地址，否则翻译为表达式的变量地址
    this->left_->addr,
    this->right_->addr
  ));

  if (this->translate_to_logical()) {
    // 要求翻译为逻辑表达式
    ret.emplace_back(IR::make_unary(
      // 因为是失败时跳转，所以需要反义操作符
      get_ir_jmp_op(reverse_op(this->op())),
      this->label_fail
    ));
  }
  else {
    // 要求翻译为算术表达式
    // 为假，就移0进来
    ret.emplace_back(IR::make_binary(
      // 此处加了个reserve，表示为假时的情况
      get_ir_mov_op(reverse_op(this->op())),
      this->addr,
      IR::Addr::make_imm(0)
      )
    );
    // 为真，就移1进来
    ret.emplace_back(IR::make_binary(
        get_ir_mov_op(this->op()),
        this->addr,
        IR::Addr::make_imm(1)
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
      // 左表达式可以编译期求值
      this->left_->addr = IR::Addr::make_imm(this->left_->eval());
    } else {
      this->left_->addr = (this->left_->addr==nullptr) ?
        IR::Addr::make_var(context.allocator.allocate_addr()) :
        this->left_->addr;
      ret.splice(ret.end(), this->left_->translate());
    }

    if (this->right_->is_evaluable()) {
      // 右表达式可以编译期求值
      this->right_->addr = IR::Addr::make_imm(this->right_->eval());
    } else {
      this->right_->addr = (this->right_->addr==nullptr) ?
        IR::Addr::make_var(context.allocator.allocate_addr()) :
        this->right_->addr;
      ret.splice(ret.end(), this->right_->translate());
    }

    // 综合计算左右表达式
    ret.emplace_back(IR::make_binary(
      get_ir_regular_op(this->op()),
      // 地址生成思路：如果表达式可以求值，就直接翻译为立即数地址，否则翻译为表达式的变量地址
      this->left_->addr,
      this->right_->addr
    ));
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
    this->exp_->label_fail = IR::Addr::make_label(this_success);

    ret.splice(ret.end(), this->exp_->translate());

    ret.emplace_back(IR::make_unary(
      IR::Op::JMP, this->label_fail
    ));
    ret.emplace_back(IR::make_unary(
      IR::Op::LABEL, IR::Addr::make_label(this_success)
    ));
  } 
  else {
    // 要求子表达式翻译为算术表达式
    this->exp_->cast_to_regular = true;
    this->exp_->cast_to_logical = false;

    // 为子表达式分配存储空间
    this->exp_->addr = (this->exp_->addr==nullptr) ?
      IR::Addr::make_var(context.allocator.allocate_addr()):
      this->exp_->addr;

    // 计算子表达式值
    ret.splice(ret.end(), this->exp_->translate());

    // cmp subexp->addr, 0
    // moveq this->addr, 1
    // movne this->addr, 0
    ret.emplace_back(IR::make_binary(
      IR::Op::CMP, this->exp_->addr, IR::Addr::make_imm(0)
    ));
    ret.emplace_back(IR::make_binary(
      IR::Op::MOVEQ, this->addr, IR::Addr::make_imm(1)
    ));
    ret.emplace_back(IR::make_binary(
      IR::Op::MOVEQ, this->addr, IR::Addr::make_imm(0)
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
      ret.emplace_back(IR::make_triple(
        IR::Op::SUB, this->addr, IR::Addr::make_imm(0), this->addr
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
Variable::_translate_immutable() {
  std::list<IR::Ptr> ret;

  // 常量要求初始化，且必须用编译期常量初始化
  // TODO 错误处理
  assert(this->initval_ != nullptr);
  assert(this->initval_->is_evaluable());

  // 处理初始化值
  std::vector<int> init_val = {this->initval_->eval()};

  // 加入符号表
  context.vartab_cur->put(std::make_shared<VarTabEntry>(
    this->name(),
    std::vector<int>(),
    -1,
    std::move(init_val),
    true
  ));

  return ret;
}

std::list<IR::Ptr>
Variable::_translate_variable() {
  std::list<IR::Ptr> ret;

  // 先分配地址
  int var_addr = context.allocator.allocate_addr();

  // 再处理初始值
  std::vector<int> init_val;
  if (this->initval_ != nullptr) {
    // 常量初始化函数
    if (this->initval_->is_evaluable()) {
      init_val.push_back(this->initval_->eval());
      ret.emplace_back(IR::make_binary(
        IR::Op::MOV, IR::Addr::make_var(var_addr), IR::Addr::make_imm(init_val.back())
      ));
    }
    // 变量初始化函数
    else {
      // 表达式计算结果放到变量分配的地址中
      this->initval_->addr = IR::Addr::make_var(var_addr);
      ret.splice(ret.end(), this->initval_->translate());
    }
  }
  
  // 加入符号表
  context.vartab_cur->put(std::make_shared<VarTabEntry>(
    this->name(),
    std::vector<int>(),
    IR::Addr::make_var(var_addr),
    std::move(init_val),
    false
  ));
  
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
  
  return ret;
}

std::list<IR::Ptr>
Array::translate() {
  std::list<IR::Ptr> ret;

  if (this->immutable()) {
    ret.splice(ret.end(), this->_translate_immutable());
  }
  else {
    ret.splice(ret.end(), this->_translate_variable());
  }

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
  return std::list<IR::Ptr>();
}

std::list<IR::Ptr>
BlockStmt::translate() {
  std::list<IR::Ptr> ret;
  // 建立块作用域
  context.new_scope();

  // 将预定义符号加入符号表中
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
  this->condition_->label_fail = IR::Addr::make_label(label_else);

  // 翻译条件表达式
  this->condition_->cast_to_logical = true;
  this->condition_->cast_to_regular = false;
  ret.splice(ret.end(), this->condition_->translate());

  // true expressions
  ret.splice(ret.end(), this->yes_->translate());
  // jmp label_end
  ret.emplace_back(IR::make_unary(
    IR::Op::JMP, IR::Addr::make_label(label_end)
  ));
  // set label label_else
  ret.emplace_back(IR::make_unary(
    IR::Op::LABEL, IR::Addr::make_label(label_else)
  ));
  // false expressions
  ret.splice(ret.end(), this->no_->translate());
  // set label label_end
  ret.emplace_back(IR::make_unary(
    IR::Op::LABEL, IR::Addr::make_label(label_end)
  ));

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
      ret.emplace_back(IR::make_unary(
        IR::Op::LABEL, this->label_cond
      ));
      ret.splice(ret.end(), this->body_->translate());
      ret.emplace_back(IR::make_unary(
        IR::Op::JMP, this->label_cond
      ));
      ret.emplace_back(IR::make_unary(
        IR::Op::LABEL, this->label_end
      ));
      return ret;
    } else {
      // 不执行
      return ret;
    }
  }
  // 常量优化结束

  // 为条件表达式分配标号
  int label_cond = context.allocator.allocate_label();
  int label_end  = context.allocator.allocate_label();
  this->condition_->label_fail = IR::Addr::make_label(label_end);

  // 翻译条件表达式
  ret.emplace_back(IR::make_unary(
    IR::Op::LABEL, IR::Addr::make_label(label_cond)
  ));
  ret.splice(ret.end(), this->condition_->translate());
  ret.splice(ret.end(), this->body_->translate());
  ret.emplace_back(IR::make_unary(
    IR::Op::JMP, IR::Addr::make_label(label_cond)
  ));
  ret.emplace_back(IR::make_unary(
    IR::Op::LABEL, IR::Addr::make_label(label_end)
  ));

  context.while_chain.pop_back();

  return ret;
}

std::list<IR::Ptr>
ReturnStmt::translate() {
  std::list<IR::Ptr> ret;
  
  if (this->ret_exp_!=nullptr) {
    if (this->ret_exp_->is_evaluable()) {
      ret.emplace_back(IR::make_unary(
        IR::Op::RET, IR::Addr::make_imm(this->ret_exp_->eval())
      ));
    } else {
      this->ret_exp_->addr = (this->ret_exp_->addr==nullptr) ?
        IR::Addr::make_var(context.allocator.allocate_addr()):
        this->ret_exp_->addr;
      ret.splice(ret.end(), this->ret_exp_->translate());
      ret.emplace_back(IR::make_unary(
        IR::Op::RET, this->ret_exp_->addr
      ));
    }
  }

  else {
    ret.emplace_back(IR::make_unary(
      IR::Op::RET, IR::Addr::make_imm(0)
    ));
  }

  return ret;
}

std::list<IR::Ptr>
BreakStmt::translate() {
  std::list<IR::Ptr> ret;

  assert(!(context.while_chain.empty()));
  
  ret.push_back(IR::make_unary(
    IR::Op::JMP, context.while_chain.back()->label_end
  ));

  return ret;
}

std::list<IR::Ptr>
ContinueStmt::translate() {
  std::list<IR::Ptr> ret;

  assert(!(context.while_chain.empty()));
  
  ret.push_back(IR::make_unary(
    IR::Op::JMP, context.while_chain.back()->label_cond
  ));

  return ret;
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
