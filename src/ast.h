#pragma once

#include "context/context.h"
#include "ir.h"
#include "debug.h"

#include <string>
#include <vector>
#include <list>
#include <cassert>

using std::string;
using std::vector;

class AST_base: public Debug_impl {
public:
  virtual ~AST_base() { }
  // 翻译为中间代码
  virtual std::list<IR::Ptr> translate() = 0;
};

// Forward Declartion for @class Expression
class Variable;

class Expression: public AST_base {
public:
  using List = std::list<Expression*>;
  enum Op{
    // logical operators
    AND, //  '&&'
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

  Expression(Op op, bool evaluable);
  virtual ~Expression();

  // 可否编译期求值
  virtual bool is_evaluable() const = 0;
  // 为常量时求值
  virtual int eval() = 0;

  // logical-regular expression cast related
  // 是否为逻辑函数？
  bool op_logical() const;
  // 是否为关系型函数？
  bool op_rel() const;
  // 是否要求翻译为逻辑型表达式？
  bool translate_to_logical() const;
  bool cast_to_logical;
  bool cast_to_regular;
  // 允许引用数组基址
  bool allow_pass_base;

  // debug
  virtual void internal_print() = 0;
  // IR generate
  virtual std::list<IR::Ptr> translate() = 0;
  
  // 获取变量地址
  // 如果没有分配，它会自动分配一个
  // 带常量优化
  virtual IR::Addr::Ptr get_var_addr();

  // 获取值为假时跳转的标号
  // 如果没有分配，它会自动分配一个
  IR::Addr::Ptr get_fail_label();

  // 指定标号
  void set_fail_label(IR::Addr::Ptr label);

protected:
  // 表达式类型
  Op op_;

  // 为（变量型的）表达式单位分配的临时变量虚地址
  IR::Addr::Ptr addr_;
  // 为（条件）表达式分配的标号编号
  // 只分配表达式为假时的标号
  // 表达式为真时，直接继续执行
  IR::Addr::Ptr label_fail_;
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
  virtual bool is_evaluable() const override;
  // 为常量时求值
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

  // 获取变量地址
  // 如果没有分配，它会自动分配一个
  virtual IR::Addr::Ptr get_var_addr() override;

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
  FuncCallExp(string *func_name, Expression::List *params);
  FuncCallExp(string *func_name);
  ~FuncCallExp();

  // 可否编译期求值
  virtual bool is_evaluable() const;
  // evaluate when it's const
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

  // 获取变量地址
  // 函数调用的变量地址总是返回值地址 r0
  virtual IR::Addr::Ptr get_var_addr() override;

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
  friend class VarExp;
  friend class FuncCallExp;

  BinaryExp(Op op, Expression *left, Expression *right);
  virtual ~BinaryExp();

  // 可否编译期求值
  virtual bool is_evaluable() const;
  // evaluate when it's const
  // 请注意：无论是算术还是逻辑表达式，求值结果均是整数类型
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

  // 获取变量地址
  // 如果没有分配，它会自动分配一个
  virtual IR::Addr::Ptr get_var_addr();

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
  virtual bool is_evaluable() const;
  // evaluate when it's const
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

  // 获取变量地址
  // 如果没有分配，它会自动分配一个
  virtual IR::Addr::Ptr get_var_addr();

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
  virtual bool is_evaluable() const override;
  // evaluate when it's const
  virtual int eval() override;

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

  // 获取变量地址
  virtual IR::Addr::Ptr get_var_addr() override;

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
  void set_type(BType type);

  // 设置变量的可变性
  void set_immutable(bool flag);

  // 判断变量是否是数组
  // 数组类应该重写该方法
  virtual bool is_array() { return false; }

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

  // 翻译过程中为自己创建的符号表项
  VarTabEntry::Ptr vartab_ent;

  // 生成的符号表项目是否为函数参数
  int param_no;
  bool is_global() const { return param_no==-1 && context.vartab_cur->is_global(); }

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
  std::list<IR::Ptr> _translate_param();
  std::list<IR::Ptr> _translate_local();
  std::list<IR::Ptr> _translate_global();
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
    virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); };

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
    virtual std::list<IR::Ptr> translate() override { return std::list<IR::Ptr>(); }

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

  // 将初始值容器一维展开
  // 
  // 如果容器指针container不是nullptr，则该函数会产生一个与数组总大小等大的一维初始化表达式列表
  // 反之，则产生一个空列表
  // 
  // 初始化表达式列表的表项可能为空，空表项表示此处的表达式是隐式给出的“0”
  Expression::List _flatten_initval(const std::vector<int> &shape, int shape_ptr, InitValContainer *container);

  std::list<IR::Ptr> _translate_param();
  std::list<IR::Ptr> _translate_local();
  std::list<IR::Ptr> _translate_global();
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
  virtual void internal_print() { }
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
  virtual std::list<IR::Ptr> translate() override;

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
  virtual std::list<IR::Ptr> translate() override;

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
  virtual std::list<IR::Ptr> translate() override;
};

/**
 * continue语句
 */
class ContinueStmt : public Stmt {
public:
  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;
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
  virtual std::list<IR::Ptr> translate() override;

private:
  // 没在块中定义，但是逻辑上属于块的变量（例如函数参数）
  std::vector<std::shared_ptr<VarTabEntry> > pre_defined;

  // 块语句中的所有语句
  vector<Stmt *> stmts_;
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
  virtual std::list<IR::Ptr> translate() override;

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
  virtual std::list<IR::Ptr> translate() override;

private:
  // while语句的条件表达式, 保证不为空
  Expression *condition_;
  
  // while语句的循环体, 保证不为空
  BlockStmt *body_;

  // 结束标号
  IR::Addr::Ptr label_cond;
  
  // 条件判断标号
  IR::Addr::Ptr label_end;
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
  virtual std::list<IR::Ptr> translate() override;

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

  // debug
  virtual void internal_print() override;
  // IR generate
  virtual std::list<IR::Ptr> translate() override;

private:
  
  // 函数返回值类型
  BType ret_type_;
  
  // 函数名称
  string name_;
  
  // 函数形参，指针可能为空
  Variable::List *params_;
  
  // 函数体，保证该指针非空
  BlockStmt *body_;
};
