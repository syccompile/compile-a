#pragma once

#include "context/context.h"
#include "ir.h"

#include <string>
#include <vector>
#include <list>
#include <cassert>

using std::string;
using std::vector;

class AST_base {
public:
  // 翻译为中间代码
  virtual std::list<IR::Ptr> translate() = 0;

  // 调试输出
  virtual void internal_print(){};
};

// Forward Declartion for @class Expression
class Variable;

class Expression: public AST_base {
public:
  using List = std::list<Expression*>;
  enum Op: int {
    // logical operators
    AND=0, //  '&&'
    OR,    //  '||'
    NOT,   //  '!'

    EQ,    //  '=='
    NEQ,   //  '!='
    LT,    //  '<'
    LE,    //  '<='
    GT,    //  '>'
    GE,    //  '>='

    ADD,  //  '+'
    SUB,  //  '-'
    MUL,  //  '*'
    DIV,  //  '/'
    MOD,  //  '%'
    NUM,  //  number
    VAR,  //  variable referrence
    CALL, //  function call
    NIL   //  NULL
  };

  Expression(Op op, bool evaluable) : op_(op), addr(-1), label_fail(-1) { }
  virtual ~Expression() {}

  Op op() { return op_; }

  // 可否编译期求值
  virtual bool is_evaluable() const;
  // 为常量时求值
  virtual int eval();

  // logical-regular expression cast related
  // 是否为逻辑函数？
  bool op_logical() const           { return this->op_<=NOT; }
  // 是否为关系型函数？
  bool op_rel() const               { return EQ<=this->op_ && this->op_<=GE; }
  // 是否要求翻译为逻辑型表达式？
  bool translate_to_logical() const { (this->op_logical()||this->op_rel()) ? !cast_to_regular : cast_to_logical; }
  bool cast_to_logical;
  bool cast_to_regular;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

  // 为（变量型的）表达式单位分配的临时变量虚地址
  int addr;
  // 为（条件）表达式分配的标号编号
  // 只分配表达式为假时的标号
  // 表达式为真时，直接继续执行
  int label_fail;

protected:
  // 表达式类型
  Op op_;
};

/**
 * 变量引用表达式，例如" a = 10; " 中的a是变量引用
 */
class VarExp : public Expression {
public:
  VarExp(string *ident, Expression::List *dimens);
  ~VarExp();

  // 可否编译期求值
  // TODO error handling
  virtual bool is_evaluable() const override {
    // 取得符号表里的对应表项，查看标识符是否为常量
    auto entry = context.vartab_cur->get(this->ident_);
    assert(entry!=nullptr);
    return entry->is_constant;
  }
  // 为常量时求值
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

private:

  // 变量名称
  string ident_;

  // 若为普通变量，该项为空
  // 若为数组，该项表示维度信息
  Expression::List *dimens_;
};


/*
 * 函数调用表达式，表示一个函数调用
 */
class FuncCallExp : public Expression {
public:
  FuncCallExp(string *func_name, vector<Expression *> *params);
  FuncCallExp(string *func_name);
  ~FuncCallExp();

  
  // 可否编译期求值
  virtual bool is_evaluable() const { return false; }
  // evaluate when it's const
  virtual int eval() override { return 0; }

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

private:
  // 函数名
  string name_;

  // 函数实参，指针可能为空
  Expression::List *params_;
};

class UnaryExp;
/**
 * 二元表达式，例如" 1 + 2 "，" 4 * 3 "
 */
class BinaryExp : public Expression {
public:
  friend class UnaryExp;

  BinaryExp(Op op, Expression *left, Expression *right);
  virtual ~BinaryExp();

  // 可否编译期求值
  virtual bool is_evaluable() { return this->left_->is_evaluable() && this->right_->is_evaluable(); }
  // evaluate when it's const
  // 请注意：无论是算术还是逻辑表达式，求值结果均是整数类型
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

protected:
  // 分两种不同的翻译形式
  std::list<IR::Ptr> _translate_logical();
  std::list<IR::Ptr> _translate_rel();
  std::list<IR::Ptr> _translate_regular();

  Expression *left_;
  Expression *right_;
};

/**
 * 一元表达式，例如 " +1 "， " ! (1>2) "
 */
class UnaryExp : public Expression {
public:
  UnaryExp(Op op, Expression *exp);
  ~UnaryExp();

  // 可否编译期求值
  virtual bool is_evaluable() const { return this->exp_->is_evaluable(); }
  // evaluate when it's const
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

private:
  std::list<IR::Ptr> _translate_regular();
  std::list<IR::Ptr> _translate_logical();

  Expression *exp_;
};

/**
 * 数字表达式，表示一个八进制、十进制或十六进制数字
 */
class NumberExp : public Expression {
public:
  NumberExp(string *str);
  NumberExp(int);
  ~NumberExp();

  // 可否编译期求值
  virtual bool is_evaluable() const { return true; }
  // evaluate when it's const
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

private:
  // 存储数字的字符串表示，例如"0xff", "2021", "08876"
  string string_;

  // 数字的实际值，假设不会超过int的表示范围
  int value_;
};

// 基本类型
enum class BType { INT, VOID, UNKNOWN };

/**
 * 普通变量
 */
class Variable: public AST_base {
public:
  friend class VarDeclStmt;
  using List = std::list<Variable *>;

  Variable(BType type, string *name, bool immutable);
  Variable(BType type, string *name, bool immutable, Expression *initval);

  virtual ~Variable();

  // 设置变量的类型
  void set_type(BType type) { type_ = type; }

  // 设置变量的可变性
  void set_immutable(bool flag) { immutable_ = flag; }

  // 设置变量是否初始化
  void set_initialzied(bool flag) { initialized_ = flag; }

  string name() { return name_; }
  bool immutable() { return immutable_; }
  bool initialized() { return initialized_; }

  // 判断变量是否是数组
  // 数组类应该重写该方法
  virtual bool is_array() { return false; }

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

protected:
  // 变量类型
  BType type_;

  // 变量名称
  string name_;
  
  // 若使用const修饰，变量不可变，该项为真
  bool immutable_;
  
  // 若变量初始化，该项为真
  bool initialized_;
  
  // 若变量初始化，该项表示初始化值，否则为空
  Expression *initval_;

private:
  std::list<IR::Ptr> _translate_immutable();
  std::list<IR::Ptr> _translate_variable();
};

class Array : public Variable {
public:
  friend class VarDeclStmt;

  /**
   * 表示数组的初始化值
   */
  class InitVal: public AST_base {
    /**
     * 使用InitVal来表示其中的"1"， "2"， "{3， 4}"
     */
  public:
    virtual ~InitVal() {}
    virtual bool is_exp() { return false; }
  };

  class InitValExp : public InitVal {
    /**
     * 对于初始化值" { 1, 2, {3, 4} } ", 使用InitValExp来
     * 表示其中的"1", "2", is_exp() 方法应该返回true表明其为普通expression
     */
  public:
    InitValExp(Expression *exp);
    ~InitValExp();
    virtual bool is_exp() override final { return true; }
    virtual void internal_print() override;

    Expression *exp_;
  };

  class InitValContainer : public InitVal {
    /**
     * 对于初始化值" { 1, 2, {3, 4} } ", 使用InitValContainer来
     * 表示其中的"{3, 4}",  is_exp() 方法应该返回false表明其为普通expression
     */
  public:
    InitValContainer();
    ~InitValContainer();
    void push_back(InitVal *initval) { initval_container_.push_back(initval); }
    virtual bool is_exp() override final { return false; }
    virtual void internal_print() override;

    vector<InitVal *> initval_container_;
  };

  Array(BType type, string *name, bool immutable, Expression::List *size);
  Array(BType type, string *name, bool immutable, Expression::List *size,
        InitVal *container);
  ~Array();

  virtual bool is_array() override { return true; }
  virtual void internal_print() override;

  virtual std::list<IR::Ptr> translate() override;

private:
  /**
   * @member dimens_
   * 数组的维度信息，例如
   * int a[][10][32];
   * dimes_的内容应为{nullptr, 10, 32}
   * dimens_的内容应该要么为空指针, 要么是可编译时求值的
   */
  Expression::List *dimens_;
  /**
   * @member initval_container_
   * 表示数组的初始化值
   */
  InitValContainer *initval_container_;

  std::vector<int> _get_shape();
  std::vector<int> _get_const_initval(const std::vector<int> &shape, int shape_ptr, InitValContainer *container);

  std::list<IR::Ptr> _translate_immutable();
  std::list<IR::Ptr> _translate_variable();
};

/**
 * 表示一个语句
 */
class Stmt: public AST_base {
public:
  using List = vector<Stmt *>;
  virtual ~Stmt() {}
  
  // 设置行号
  virtual void set_lineno(int lineno) { lineno_ = lineno; }
  
  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

  int lineno_;
};

/**
 * 变量声明语句, 例如" int a[10] = {1,2,3}, b = 12; "
 * 一个变量声明语句可以声明多个变量
 */
class VarDeclStmt : public Stmt {
public:
  VarDeclStmt();
  ~VarDeclStmt();

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

  // 加入变量声明
  void push_back(Variable *var) { vars_.push_back(var); }
  void set_global();
private:
  // 声明的变量
  Variable::List vars_;
};

/**
 * return语句
 */
class ReturnStmt : public Stmt {
public:
  ReturnStmt(Expression *ret);
  ~ReturnStmt();

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

  // return语句对应的函数
  FunctionDecl *parent_;

private:
  // return 语句的返回值, 指针可能为空
  Expression *ret_exp_;
};

class WhileStmt;
/**
 * break语句
 */
class BreakStmt : public Stmt {
public:
  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

  // break语句对应的while语句
  WhileStmt *parent_;
};

/**
 * continue语句
 */
class ContinueStmt : public Stmt {
public:
  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

  // continue语句对应的while语句
  WhileStmt *parent_;
};

/**
 * 表示一个表达式语句， 例如" 3+4; "
 */
class ExpStmt : public Stmt {
public:
  ExpStmt(Expression *edp);
  ~ExpStmt();
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

private:
  Expression *exp_;
};

/**
 * 表示一个块语句， 块语句中包含多个语句
 */
class BlockStmt : public Stmt {
public:
  friend class FunctionDecl;
  friend class IfStmt;
  friend class WhileStmt;
  BlockStmt();
  ~BlockStmt();
  // 将语句加入块语句
  void push_back(Stmt *stmt);
  
  // 将@param list中的语句全部加入到块语句中
  void push_back(Stmt::List *list) {
    for (Stmt *stmt : *list) {
      stmts_.push_back(stmt);
    }
  }

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

private:
  // 块语句中的所有语句
  vector<Stmt *> stmts_;

  // 每个块语句有一个符号表
  SymbolTable::Ptr symtab_;
};

/**
 * if语句
 */
class IfStmt : public Stmt {
public:
  IfStmt(Expression *condition, BlockStmt *yes, BlockStmt *no);
  IfStmt(Expression *condition, BlockStmt *yes);
  ~IfStmt();

  BlockStmt *yes() { return yes_; }
  BlockStmt *no() { return no_; }
  
  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

private:
  // if语句的条件表达式, 保证该指针非空
  Expression *condition_;
  
  // if语句的条件成立时的块语句, 保证该指针非空
  BlockStmt *yes_;
  
  // if语句条件不成立时的块语句, 不保证指针非空
  BlockStmt *no_;
};

/**
 * 表示一个while语句
 */
class WhileStmt : public Stmt {
public:
  friend class BreakStmt;
  friend class ContinueStmt;

  WhileStmt(Expression *condition, BlockStmt *body);
  ~WhileStmt();
  
  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

private:
  // while语句的条件表达式, 保证不为空
  Expression *condition_;
  
  // while语句的循环体, 保证不为空
  BlockStmt *body_;

  // break语句跳转的Label
  FrameAccess break_access_;
  
  // break语句跳转的Label
  FrameAccess continue_access_;
};

/**
 * 赋值语句
 */
class AssignmentStmt : public Stmt {
public:
  AssignmentStmt(string *name, Expression::List *dimens, Expression *rval);
  ~AssignmentStmt();

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

private:
  // 左值变量的名字
  string name_;
  
  // 若左值为普通变量, 该项为空；若左值为数组，该项表示数组的维度
  Expression::List *dimens_;
  
  // 右值，保证不为空
  Expression *rval_;
};

/**
 * 函数声明
 */
class FunctionDecl: public AST_base {
public:
  FunctionDecl(BType ret_type, string *name, Variable::List *params,
               BlockStmt *block);
  ~FunctionDecl();

  string name() { return name_; }
  BType ret_type() { return ret_type_; }

  // 返回对应形参的FrameAccess
  // FrameAccess get_params_access(size_t index) {
  //   return symtab_->find(params_->at(index)).access_;
  // }
  // 返回函数返回值的FrameAccess
  FrameAccess get_return_access() { return ret_access_; }

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

private:
  
  // 函数返回值类型
  BType ret_type_;
  
  // 函数名称
  string name_;
  
  // 函数形参，指针可能为空
  Variable::List *params_;
  
  // 函数体，保证该指针非空
  BlockStmt *body_;
   
  // member frame_
  Frame::Ptr frame_;

  // 返回值的FrameAccess
  FrameAccess ret_access_;
};
