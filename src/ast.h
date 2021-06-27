#pragma once

#include "debug.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

// Forward Declartion for @class Expression
class Variable;

class Expression : public Debug {
public:
  using List = vector<Expression *>;
  enum class Op {
    AND, //  '&&'
    OR,  //  '||'
    NOT, //  '!'
    EQ,  //  '=='
    NEQ, //  '!='
    LT,  //  '<'
    LE,  //  '<='
    GT,  //  '>'
    GE,  //  '>='

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
  Expression(const Op op, bool evaluable) : op_(op), evaluable_(evaluable) {}
  virtual ~Expression() {}
  bool evaluable() const { return evaluable_; }
  virtual void internal_print() const override {}

protected:

  /**
   * @member op_
   * 表达式类型
   */
  const Op op_;
  /**
   * @member evaluable_
   * 是否可以在编译时求值
   */
  bool evaluable_; 
};

/**
 * 变量引用表达式，例如" a = 10; " 中的a是变量引用
 */
class VarExp : public Expression {
public:
  VarExp(const Variable *var);
  ~VarExp();
  virtual void internal_print() const override;

private:
  /**
   * @member var_ 
   * 成员指针并不指向实际引用的变量，
   * 而仅用于存储
   */
  const Variable *var_;
};

/*
 * 函数调用表达式，表示一个函数调用
 */
class FuncCallExp : public Expression {
public:
  FuncCallExp(const string *func_name, const vector<Expression *> *params);
  FuncCallExp(const string *func_name);
  ~FuncCallExp();
  virtual void internal_print() const override;

private:
  /** 
   * @member name_
   * 函数名称
   */
  const string name_;
  /**
   * @member params_
   * 函数实参，指针可能为空
   */
  const vector<Expression *> *params_;
};

/**
 * 二元表达式，例如" 1 + 2 "，" 4 * 3 "
 */
class BinaryExp : public Expression {
public:
  BinaryExp(const Op op, const Expression *left, const Expression *right);
  ~BinaryExp();
  virtual void internal_print() const override;

private:
  const Expression *left_;
  const Expression *right_;
};

/**
 * 一元表达式，例如 " +1 "， " ! (1>2) "
 */
class UnaryExp : public Expression {
public:
  UnaryExp(const Op op, const Expression *exp);
  ~UnaryExp();
  virtual void internal_print() const override;

private:
  const Expression *exp_;
};

/**
 * 数字表达式，表示一个八进制、十进制或十六进制数字
 */
class NumberExp : public Expression {
public:
  NumberExp(const string *str);
  ~NumberExp();
  virtual void internal_print() const override;

private:
  /**
   * @member string_
   * 存储数字的字符串表示，例如"0xff", "2021", "08876"
   */
  const string string_;
  /**
   * @member value_
   * 数字的实际值，假设不会超过int的表示范围
   */
  const int value_;     
};

// 基本类型
enum class BType { INT, VOID, UNKNOWN };

/**
 * 普通变量
 */
class Variable : public Debug {
public:
  using List = vector<Variable *>;

  Variable(const BType type, const string *name, bool immutable);
  Variable(const BType type, const string *name, bool immutable, const Expression *initval);
  virtual ~Variable();

  /**
   * 设置变量的类型
   */
  void set_type(BType type) { type_ = type; }
  /**
   * 设置变量的可变性
   */
  void set_immutable(bool flag) { immutable_ = flag; }

  string name() const { return name_; }
  bool immutable() const { return immutable_; }
  bool initialized() const { return initialized_; }

  /**
   * 判断变量是否是数组
   * 数组类应该重写该方法
   */
  virtual bool is_array() const { return false; }

  virtual void internal_print() const override;

protected:

  BType type_;
  const string name_;
  bool immutable_;      // 变量是否可变
  const bool initialized_;    // 变量是否初始化
  const Expression *initval_; // 变量的初始化值
};

class Array : public Variable {
public:
  /**
   * 表示数组的初始化值
   */
  class InitVal : public Debug {
    /**
     * 使用InitVal来表示其中的"1"， "2"， "{3， 4}"
     */
  public:
    virtual bool is_exp() const { return false; }
    virtual ~InitVal() {}
  };

  class InitValExp : public InitVal {
    /**
     * 对于初始化值" { 1, 2, {3, 4} } ", 使用InitValExp来
     * 表示其中的"1", "2", is_exp() 方法应该返回true表明其为普通expression
     */
  public:
    InitValExp(Expression *exp);
    ~InitValExp();
    virtual bool is_exp() const override final { return true; }
    virtual void internal_print() const override;

  private:
    const Expression *exp_;
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
    virtual bool is_exp() const override final { return false; }
    virtual void internal_print() const override;

  private:
    vector<InitVal *> initval_container_;
  };

  Array(const BType type, const string *name, bool immutable, const Expression::List *size);
  Array(const BType type, const string *name, bool immutable, const Expression::List *size,
        InitVal *container);
  ~Array();
  virtual bool is_array() const override { return true; }
  virtual void internal_print() const override;

private:
  /**
   * @member dimens_
   * 数组的维度信息，例如
   * int a[][10][32];
   * dimes_的内容应为{nullptr, 10, 32}
   * dimens_的内容应该要么为空指针, 要么是可编译时求值的
   */
  const Expression::List *dimens_;
  /**
   * @member initval_container_
   * 表示数组的初始化值
   */
  const InitValContainer *initval_container_;
};

/**
 * 表示一个语句
 */
class Stmt : public Debug {
public:
  using List = vector<Stmt *>;
  virtual ~Stmt() {}
  /**
   * 设置行号
   */
  virtual void set_lineno(int lineno) { lineno_ = lineno; }

protected:
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
  /**
   * 加入变量声明
   */
  void push_back(Variable *var) { vars_.push_back(var); }
  virtual void internal_print() const override;

private:
  /**
   * @member vars_
   * 声明的变量
   */
  vector<Variable *> vars_;
};

/**
 * 表示一个表达式语句， 例如" 3+4; "
 */
class ExpStmt : public Stmt {
public:
  ExpStmt(Expression *edp);
  ~ExpStmt();

private:
  const Expression *exp_;
};

/**
 * 表示一个块语句， 块语句中包含多个语句
 */
class BlockStmt : public Stmt {
public:
  BlockStmt();
  ~BlockStmt();
  /**
   * 将语句加入块语句
   */
  void push_back(Stmt *stmt) { stmts_.push_back(stmt); }
  /**
   * 将@param list中的语句全部加入到块语句中
   */
  void push_back(Stmt::List *list) {
    for (Stmt *stmt : *list) {
      stmts_.push_back(stmt);
    }
  }
  virtual void internal_print() const override;

private:
  vector<Stmt *> stmts_;
};

/**
 * if语句
 */
class IfStmt : public Stmt {
public:
  IfStmt(const Expression *condition, const BlockStmt *yes, const BlockStmt *no);
  IfStmt(const Expression *condition, const BlockStmt *yes);
  ~IfStmt();
  virtual void internal_print() const override;

private:
  /**
   * @member condition_
   * if语句的条件表达式, 保证该指针非空
   */
  const Expression *condition_;
  /**
   * @member yes_
   * if语句的条件成立时的块语句, 保证该指针非空
   */
  const BlockStmt *yes_;
  /**
   * @member no_
   * if语句条件不成立时的块语句, 不保证指针非空
   */
  const BlockStmt *no_;
};

/**
 * 表示一个while语句
 */
class WhileStmt : public Stmt {
public:
  WhileStmt(const Expression *condition, const BlockStmt *body);
  ~WhileStmt();
  virtual void internal_print() const override;

private:
  /**
   * @member condition_
   * while语句的条件表达式, 保证不为空
   */
  const Expression *condition_;
  /**
   * @member body_
   * while语句的循环体, 保证不为空
   */
  const BlockStmt *body_;
};

/**
 * return语句
 */
class ReturnStmt : public Stmt {
public:
  ReturnStmt(const Expression *ret);
  ~ReturnStmt();
  virtual void internal_print() const override;

private:
  /**
   * @member ret_exp_
   * return 语句的返回值, 指针可能为空
   */
  const Expression *ret_exp_;
};

/**
 * break语句
 */
class BreakStmt : public Stmt {
public:
  virtual void internal_print() const override;
};

/**
 * continue语句
 */
class ContinueStmt : public Stmt {
public:
  virtual void internal_print() const override;
};

/**
 * 赋值语句
 */
class AssignmentStmt : public Stmt {
public:
  AssignmentStmt(const string *name, const Expression::List *dimens, const Expression *rval);
  ~AssignmentStmt();
  virtual void internal_print() const override;

private:
  /**
   * @member name_
   * 左值变量的名字
   */
  const string name_;
  /**
   * @member dimens_
   * 若左值为普通变量, 该项为空；若左值为数组，该项表示数组的维度
   */
  const Expression::List *dimens_;
  /**
   * @member rval_
   * 右值，保证不为空
   */
  const Expression *rval_;
};

/**
 * 函数声明
 */
class FunctionDecl : public Debug {
public:
  /**
   * 表示函数形参
   */
  class FParam {
    friend class FunctionDecl;

  public:
    using List = vector<FParam *>;
    FParam(const BType type, const string *name, const Expression::List *dimens)
        : type_(type), name_(*name), dimens_(dimens) {}
    ~FParam() { delete dimens_; }

  private:
    /**
     * @member type_
     * 形参的类型
     */
    const BType type_;
    /**
     * @member name_
     * 形参的名称
     */
    const string name_;
    /**
     * @member dimens_
     * 若形参为普通变量, 该项为空；若形参为数组，该项表示数组的维度
     */
    const Expression::List *dimens_;
  };
  FunctionDecl(const BType ret_type, const string *name, const FParam::List *params,
               const BlockStmt *block);
  ~FunctionDecl();

  string name() const { return name_; }
  BType ret_type() const { return ret_type_; }
  virtual void internal_print() const override;

private:
  /**
   * @member ret_type_
   * 函数返回值类型
   */
  const BType ret_type_;
  /**
   * @member name_
   * 函数名称
   */
  const string name_;
  /**
   * @member params_
   * 函数形参，指针可能为空
   */
  const FParam::List *params_;
  /**
   * @member body_
   * 函数体，保证该指针非空
   */
  const BlockStmt *body_;
};
