#include <string>
#include <vector>

using std::string;
using std::vector;

/** 
 * Represents an expression.
 * For example, `5+3` , a function call or only a variable referrence is an expression.
 * An expression can be composed by other expressions. `5*3` is composed by expression `5` and
 *    expression `3`.
 * PS:After constructing an expression with other expressions, these expressions shouldn't be used.
 *
 **/
class Expression {
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

    ADD, //  '+'
    SUB, //  '-'
    MUL, //  '*'
    DIV, //  '/'
    MOD, //  '%'
    NUM, // number
    IDENT, // variable referrence
    CALL, //  function call
    NIL   //  NULL
  };
  Expression(string ident);
  Expression(int value);
  Expression(string function, vector<Expression *> *params);
  Expression(Op, Expression *, Expression *);
  Expression(Op, Expression *);
  ~Expression();

private:
  Op op_;
  Expression *sub_exp1_;
  Expression *sub_exp2_;
  bool evaluable_;
  int number_;
  vector<Expression *> *params_;
  string name_;
};

class Variable {
public:
  using List = vector<Variable *>;
  enum class BType { INT, UNKNOWN };

  Variable(Variable::BType type, string name, bool immutable);
  Variable(Variable::BType type, string name, bool immutable,
           Expression *initval);
  // Array Assignment
  Variable(Variable::BType type, string name, bool immutable, Expression *size,
           Expression::List *initvals);
  ~Variable();
  void setType(Variable::BType type) { type_ = type; }
  void setImmutable(bool flag) { immutable_ = flag; }
  void setInitialzied(bool flag) { initialized_ = flag; }

private:
  BType type_;
  string name_;
  bool immutable_;
  bool initialized_;
  Expression::List *initvals_;
  Expression *size_;
};

class Stmt {};
class VarDeclStmt : public Stmt {
public:
  VarDeclStmt() {}
  void push_back(Variable *var) { vars_.push_back(var); }

private:
  vector<Variable *> vars_;
};
class IfStmt : public Stmt {};
class WhileStmt : public Stmt {
public:
  Expression *condition_;
  vector<Stmt *> stmts_;
};
class BreakStmt : public Stmt {};
class ReturnStmt : public Stmt {
public:
  Expression *ret_;
};
class AssignmentStmt : public Stmt {};
class BlockStmt : public Stmt {
public:
  void push_back(Stmt *stmt) { stmt_.push_back(stmt); }

private:
  vector<Stmt *> stmt_;
};

/* Function */
class Function {
public:
  enum class RetType { INT, VOID };
  class FuncParams {
  public:
    FuncParams();
    void push_back(Variable param) { params_.push_back(param); }

  private:
    vector<Variable> params_;
  };

  Function(RetType ret_type, string name, FuncParams params);

  void push_back(Stmt *stmt) { stmts_.push_back(stmt); }

private:
  vector<Stmt *> stmts_;
  FuncParams params_;
};
