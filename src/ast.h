#pragma once

#include "debug.h"
#include "ir.h"
#include "ir_translator.h"
#include "symtab.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

// Forward Declartion for @class Expression
class Variable;

class Expression : public Debug_impl, public IrTranslator_impl {
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
  Expression(Op op, bool evaluable) : op_(op), evaluable_(evaluable) {}
  virtual ~Expression() {}
  bool evaluable() { return evaluable_; }
  virtual int eval();
  virtual void internal_print() override {}

  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
  translate(SymbolTable::Ptr symtab) override {
    return std::make_tuple(vector<IR::Ptr>(), nullptr);
  }

protected:
  void set_evaluable(bool evaluable) { evaluable_ = evaluable; }

  /**
   * @member op_
   * 表达式类型
   */
  Op op_;
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
  VarExp(string *ident, Expression::List *dimens);
  ~VarExp();
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
  translate(SymbolTable::Ptr symtab) override;
  virtual int eval() override;

private:
  /**
   * @member ident_
   * 变量名称
   */
  string ident_;
  /**
   * @member dimens_
   * 若为普通变量，该项为空
   * 若为数组，该项表示维度信息
   */
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
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
  translate(SymbolTable::Ptr symtab) override;
  virtual int eval() override;

private:
  /**
   * @member name_
   * 函数名称
   */
  string name_;
  /**
   * @member params_
   * 函数实参，指针可能为空
   */
  Expression::List *params_;
};

/**
 * 二元表达式，例如" 1 + 2 "，" 4 * 3 "
 */
class BinaryExp : public Expression {
public:
  BinaryExp(Op op, Expression *left, Expression *right);
  ~BinaryExp();
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
  translate(SymbolTable::Ptr symtab) override;
  virtual int eval() override;

private:
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
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
  translate(SymbolTable::Ptr symtab) override;
  virtual int eval() override;

private:
  Expression *exp_;
};

/**
 * 数字表达式，表示一个八进制、十进制或十六进制数字
 */
class NumberExp : public Expression {
public:
  NumberExp(string *str);
  ~NumberExp();
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
  translate(SymbolTable::Ptr symtab) override;
  virtual int eval() override;

private:
  /**
   * @member string_
   * 存储数字的字符串表示，例如"0xff", "2021", "08876"
   */
  string string_;
  /**
   * @member value_
   * 数字的实际值，假设不会超过int的表示范围
   */
  int value_;
};

// 基本类型
enum class BType { INT, VOID, UNKNOWN };

/**
 * 普通变量
 */
class Variable : public Debug_impl, public IrTranslator_impl {
public:
  friend class VarDeclStmt;
  using List = vector<Variable *>;

  Variable(BType type, string *name, bool immutable);
  Variable(BType type, string *name, bool immutable, Expression *initval);
  virtual ~Variable();

  /**
   * 设置变量的类型
   */
  void set_type(BType type) { type_ = type; }
  /**
   * 设置变量的可变性
   */
  void set_immutable(bool flag) { immutable_ = flag; }
  /**
   * 设置变量是否初始化
   */
  void set_initialzied(bool flag) { initialized_ = flag; }

  /**
   * 设置是否是全局变量
   */
  void set_global(bool global) { global_ = global; }

  string name() { return name_; }
  bool immutable() { return immutable_; }
  bool global() { return global_; }
  bool initialized() { return initialized_; }

  /**
   * 判断变量是否是数组
   * 数组类应该重写该方法
   */
  virtual bool is_array() { return false; }

  virtual void internal_print() override;

protected:
  /**
   * @member global_
   * 变量是否为全局变量
   */
  bool global_;
  /**
   * @member type_
   * 变量类型
   */
  BType type_;
  /**
   * @member name_
   * 变量名称
   */
  string name_;
  /**
   * @member immutable_
   * 若使用const修饰，变量不可变，该项为真
   */
  bool immutable_;      
  /**
   * @member initialized_
   * 若变量初始化，该项为真
   */
  bool initialized_;
  /**
   * @member initval_
   * 若变量初始化，该项表示初始化值，否则为空
   */
  Expression *initval_;
};

class Array : public Variable {
public:
  /**
   * 表示数组的初始化值
   */
  class InitVal : public Debug_impl, public IrTranslator_impl {
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

  private:
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

  private:
    vector<InitVal *> initval_container_;
  };

  Array(BType type, string *name, bool immutable, Expression::List *size);
  Array(BType type, string *name, bool immutable, Expression::List *size,
        InitVal *container);
  ~Array();
  virtual bool is_array() override { return true; }
  virtual void internal_print() override;

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
};

/**
 * 表示一个语句
 */
class Stmt : public Debug_impl, public IrTranslator_impl {
public:
  using List = vector<Stmt *>;
  virtual ~Stmt() {}
  /**
   * 设置行号
   */
  virtual void set_lineno(int lineno) { lineno_ = lineno; }

  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
  translate(SymbolTable::Ptr) override {
    return std::make_tuple(vector<IR::Ptr>(), nullptr);
  }

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
  void set_global();
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;

private:
  /**
   * @member vars_
   * 声明的变量
   */
  vector<Variable *> vars_;
};

/**
 * return语句
 */
class ReturnStmt : public Stmt {
public:
  ReturnStmt(Expression *ret);
  ~ReturnStmt();
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;
  /**
   * @member parent_
   * return语句对应的函数
   */
  FunctionDecl *parent_;

private:
  /**
   * @member ret_exp_
   * return 语句的返回值, 指针可能为空
   */
  Expression *ret_exp_;
};

class WhileStmt;
/**
 * break语句
 */
class BreakStmt : public Stmt {
public:
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;
  /**
   * @member parent_
   * break语句对应的while语句
   */
  WhileStmt *parent_;
};

/**
 * continue语句
 */
class ContinueStmt : public Stmt {
public:
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;
  /**
   * @member parent_
   * continue语句对应的while语句
   */
  WhileStmt *parent_;
};

/**
 * 表示一个表达式语句， 例如" 3+4; "
 */
class ExpStmt : public Stmt {
public:
  ExpStmt(Expression *edp);
  ~ExpStmt();
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;

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
  /**
   * 将语句加入块语句
   */
  void push_back(Stmt *stmt);
  /**
   * 将@param list中的语句全部加入到块语句中
   */
  void push_back(Stmt::List *list) {
    for (Stmt *stmt : *list) {
      stmts_.push_back(stmt);
    }
  }
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;

private:
  /**
   * @member stmts_
   * 块语句中的所有语句
   */
  vector<Stmt *> stmts_;

  /** 
   * @member symtab_
   * 每个块语句有一个符号表 
   **/
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
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;

private:
  /**
   * @member condition_
   * if语句的条件表达式, 保证该指针非空
   */
  Expression *condition_;
  /**
   * @member yes_
   * if语句的条件成立时的块语句, 保证该指针非空
   */
  BlockStmt *yes_;
  /**
   * @member no_
   * if语句条件不成立时的块语句, 不保证指针非空
   */
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
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;

private:
  /**
   * @member condition_
   * while语句的条件表达式, 保证不为空
   */
  Expression *condition_;
  /**
   * @member body_
   * while语句的循环体, 保证不为空
   */
  BlockStmt *body_;

  /**
   * @member break_access_
   * break语句跳转的Label
   */
  FrameAccess break_access_;
  /**
   * @member continue_access_
   * break语句跳转的Label
   */
  FrameAccess continue_access_;
};

/**
 * 赋值语句
 */
class AssignmentStmt : public Stmt {
public:
  AssignmentStmt(string *name, Expression::List *dimens, Expression *rval);
  ~AssignmentStmt();
  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;

private:
  /**
   * @member name_
   * 左值变量的名字
   */
  string name_;
  /**
   * @member dimens_
   * 若左值为普通变量, 该项为空；若左值为数组，该项表示数组的维度
   */
  Expression::List *dimens_;
  /**
   * @member rval_
   * 右值，保证不为空
   */
  Expression *rval_;
};

/**
 * 函数声明
 */
class FunctionDecl : public Debug_impl, public IrTranslator_impl {
public:
  FunctionDecl(BType ret_type, string *name, Variable::List *params,
               BlockStmt *block);
  ~FunctionDecl();

  string name() { return name_; }
  BType ret_type() { return ret_type_; }

  /**
   * 返回对应形参的FrameAccess
   */
  FrameAccess get_params_access(size_t index) {
    return symtab_->find(params_->at(index)).access_;
  }
  /**
   * 返回函数返回值的FrameAccess
   */
  FrameAccess get_return_access() { return ret_access_; }

  virtual void internal_print() override;
  virtual std::tuple<vector<IR::Ptr>, FrameAccess>
      translate(SymbolTable::Ptr) override;

private:
  /**
   * @member ret_type_
   * 函数返回值类型
   */
  BType ret_type_;
  /**
   * @member name_
   * 函数名称
   */
  string name_;
  /**
   * @member params_
   * 函数形参，指针可能为空
   */
  Variable::List *params_;
  /**
   * @member body_
   * 函数体，保证该指针非空
   */
  BlockStmt *body_;

  /**
   * @member frame_
   * 函数的栈帧
   */
  Frame::Ptr frame_;

  /**
   * @member symtab_
   * 函数的符号表，一般只保存函数的形参
   * 函数内部的变量声明放在blockstmt的符号表内
   */
  SymbolTable::Ptr symtab_;

  /**
   * @member ret_access_
   * 返回值的FrameAccess
   */
  FrameAccess ret_access_;
};
