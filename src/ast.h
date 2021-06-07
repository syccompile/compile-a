#pragma once

#include "debug.h"

#include <string>
#include <vector>

using std::string;
using std::vector;

// Forward Declartion for @class Expression
class Variable;
/**
 * Represents an expression.
 * For example, `5+3` , a function call or only a variable referrence is an
 *expression. An expression can be composed by other expressions. `5*3` is
 *composed by expression `5` and expression `3`. PS:After constructing an
 *expression with other expressions, these expressions shouldn't be used. And
 *all the pointer parameters shouldn't be null.
 *
 **/
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

    ADD,   //  '+'
    SUB,   //  '-'
    MUL,   //  '*'
    DIV,   //  '/'
    MOD,   //  '%'
    NUM,   // number
    IDENT, // variable referrence
    CALL,  //  function call
    NIL    //  NULL
  };
  Expression(Variable *var);
  Expression(string *number);
  Expression(string *function, vector<Expression *> *params);
  Expression(Op, Expression *, Expression *);
  Expression(Op, Expression *);
  ~Expression();

  virtual void internal_print() override;

private:
  Op op_;
  Expression *sub_exp1_;
  Expression *sub_exp2_;
  bool evaluable_;
  string number_;
  string func_name_;
  vector<Expression *> *func_params_;
  Variable *var_;
};

enum class BType { INT, VOID, UNKNOWN };

class Variable : public Debug {
public:
  using List = vector<Variable *>;

  Variable(BType type, string *name, bool immutable);
  Variable(BType type, string *name, bool immutable, Expression *initval);
  virtual ~Variable();
  virtual bool is_array() { return false; }
  void setType(BType type) { type_ = type; }
  void setImmutable(bool flag) { immutable_ = flag; }
  void setInitialzied(bool flag) { initialized_ = flag; }
  virtual void internal_print() override;

protected:
  BType type_;
  string name_;
  bool immutable_;
  bool initialized_;
  Expression *initval_;
};

class Array : public Variable {
public:
  class InitVal : public Debug{
  public:
    virtual bool is_exp() { return false; }
    virtual ~InitVal(){}
  };
  class InitValExp : public InitVal {
  public:
    InitValExp(Expression *exp);
    ~InitValExp();
    virtual bool is_exp() override final { return true; }
    virtual void internal_print() override;

  private:
    Expression *exp_;
  };
  class InitValContainer : public InitVal {
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
  Expression::List *dimens_;
  InitValContainer *container_;
};

class Stmt : public Debug {
public:
  using List = vector<Stmt*>;
  virtual ~Stmt() {}
  virtual void set_lineno(int lineno) { lineno_ = lineno; }
protected:
  int lineno_;
};

class VarDeclStmt : public Stmt{
public:
  VarDeclStmt();
  ~VarDeclStmt();
  void push_back(Variable *var) { vars_.push_back(var); }
  virtual void internal_print() override;
private:
  vector<Variable *> vars_;
};

class ExpStmt : public Stmt {
  public:
    ExpStmt(Expression* edp);
    ~ExpStmt();
  private:
    Expression* exp_;
};

class BlockStmt : public Stmt {
public:
  BlockStmt();
  ~BlockStmt();
  void push_back(Stmt *stmt) { stmts_.push_back(stmt); }
  void push_back(Stmt::List * list) {
    for (Stmt *stmt : *list) {
      stmts_.push_back(stmt);
    }
  }
  virtual void internal_print() override;
private:
  vector<Stmt *> stmts_;
};

class IfStmt : public Stmt {
public:
  IfStmt(Expression *condition, BlockStmt *yes, BlockStmt *no);
  IfStmt(Expression *condition, BlockStmt *yes);
  ~IfStmt();
  virtual void internal_print() override;
private:
  Expression* condition_;
  BlockStmt* yes_;
  BlockStmt* no_;
};

class WhileStmt : public Stmt {
public:
  WhileStmt(Expression *condition, BlockStmt* body);
  ~WhileStmt();
  virtual void internal_print() override;
private:
  Expression *condition_;
  BlockStmt *body_;
};
class ReturnStmt : public Stmt {
public:
  ReturnStmt(Expression *ret);
  ~ReturnStmt();
  virtual void internal_print() override;
private:
  Expression *ret_exp_;
};

class BreakStmt : public Stmt {
public:
  virtual void internal_print() override;
};

class ContinueStmt : public Stmt {
public:
  virtual void internal_print() override;
};

class AssignmentStmt : public Stmt {
public:
  AssignmentStmt(string *name, Expression::List *dimens, Expression *rval);
  ~AssignmentStmt();
  virtual void internal_print() override;
private:
  string name_;
  Expression::List *dimens_;
  Expression* rval_;
};

/* Function */
class FunctionDecl : public Debug{
public:
  class FParam {
    friend class FunctionDecl;
  public:
    using List = vector<FParam*> ;
    FParam(BType type, string *name, Expression::List *dimens)
        : type_(type), name_(*name), dimens_(dimens) {}
    ~FParam() { delete dimens_; }
  private:
      BType type_;
      string name_;
      Expression::List* dimens_;
  };
  FunctionDecl(BType ret_type, string *name, FParam::List *params,
           BlockStmt *block);
  ~FunctionDecl();
  virtual void internal_print() override;
private:
  BType ret_type_;
  string func_name_;
  FParam::List* params_;
  BlockStmt* body_;
};
