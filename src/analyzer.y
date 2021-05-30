%{
#include "ast.h"
#include <cstdio>
#include <vector>

#include <string>

extern int yylex();
void yyerror(const char* msg) { printf("%s\n", msg); }
std::vector<VarDeclStmt*> vardecl;
std::vector<Function*> funcs;
%}

%union {
  int token;
  std::string* string;
  Expression * exp;
  Expression::List * explist;
  Variable * var;
  Variable::List * varlist;
  Variable::BType vartype;
  Stmt* stmt;
  VarDeclStmt * vardeclstmt;
  IfStmt * ifstmt;
  WhileStmt * whilestmt;
  BreakStmt * breakstmt;;
  ReturnStmt * retstmt;
  AssignmentStmt * assignstmt;
  BlockStmt * blockstmt;
  Function* func;
  Function::FuncParams * fparams;
}

%token <token> LBRACKET RBRACKET LPARENT RPARENT LCURLY RCURLY
%token <token> INT CONST SEMI COMMA ASSIGN
%token <string> NUMBER
%token <token> ADD SUB MUL DIV MOD NOT
%token <string> IDENT

%type <token>  CompUnit
%type <vartype>  BType
%type <vardeclstmt> VarDecl
%type <var> VarDef
%type <varlist> VarDefList
%type <varinit> InitVal
%type <func> FuncDef
%type <fparams> FuncParams
%type <blockstmt> Block
%type <exp> Exp
%type <explist> DimentList
%type <exp> PrimaryExp
%type <exp> UnaryExp
%type <token> UnaryOp
%type <exp> MulExp
%type <exp> AddExp
%type <exp> RelExp
%type <exp> EqExp
%type <exp> LAndExp
%type <exp> LOrExp

%start CompUnit

%%
CompUnit: VarDecl { vardecl.push_back($1); }
        //| FuncDef { funcs.push_back($1); }
        | CompUnit VarDecl { vardecl.push_back($2); }
        //| CompUnit FuncDef { vardecl.push_back($2); }
        ;

BType : INT { $$ = Variable::BType::INT; }
      ;

Exp  :  MulExp { $$ = $1; }
     |  Exp ADD MulExp { $$ = new Expression(Expression::Op::ADD, $1, $3); }
     ;

MulExp : UnaryExp { $$ = $1;}
       | MulExp MUL UnaryExp { $$ = new Expression(Expression::Op::MUL, $1, $3); }
       | MulExp DIV UnaryExp { $$ = new Expression(Expression::Op::DIV, $1, $3); }
       | MulExp MOD UnaryExp { $$ = new Expression(Expression::Op::MOD, $1, $3); }

// TODO
UnaryExp : PrimaryExp { $$ = $1; }
         | ADD UnaryExp { $$ = new Expression(Expression::Op::ADD, $2); }
         | SUB UnaryExp { $$ = new Expression(Expression::Op::SUB, $2); }
         | NOT UnaryExp { $$ = new Expression(Expression::Op::NOT, $2); }
         ;
// TODO
PrimaryExp : LPARENT Exp RPARENT { $$ = $2; }
           | NUMBER { $$ = new Expression(*$1); }
           ;

VarDecl: CONST BType VarDefList SEMI { $$ = new VarDeclStmt();  
                                      for(Variable * var : *$3){
                                        var->setImmutable(true);
                                        var->setType($2);
                                        $$->push_back(var);
                                      }
                                   }
       | BType VarDefList SEMI { $$ = new VarDeclStmt();
                                for(Variable * var : *$2){
                                        var->setImmutable(false);
                                        var->setType($1);
                                        $$->push_back(var);
                                }
                                }
       ; 
DimentList: Exp { $$ = new Expression::List(); 
                  $$->push_back($1);
                }
          | DimentList COMMA Exp { $1->push_back($3);
                                   $$ = $1;
                                 }
          ;
VarDefList: IDENT ASSIGN Exp { $$ = new Variable::List();
                               $$->push_back(new Variable(Variable::BType::UNKNOWN, *$1, false, $3));
                             }
          | IDENT LBRACKET Exp RBRACKET ASSIGN LCURLY DimentList RCURLY {
                               $$ = new Variable::List();
                               $$->push_back(new Variable(Variable::BType::UNKNOWN, *$1, false, $3, $7));
                              }
          | IDENT { $$ = new Variable::List();
                    $$->push_back(new Variable(Variable::BType::UNKNOWN, *$1, false));
                  }
          | IDENT LBRACKET Exp RBRACKET {
                        $$ = new Variable::List();
                        $$->push_back(new Variable(Variable::BType::UNKNOWN, *$1, false, $3, nullptr));
                      }
          | VarDefList COMMA IDENT ASSIGN Exp {
                              $$ = $1;
                              $$->push_back(new Variable(Variable::BType::UNKNOWN, *$3, false, $5));
                            }
          | VarDefList COMMA IDENT LPARENT Exp RPARENT ASSIGN LCURLY DimentList RCURLY {
                             $$ = $1; 
                             $$->push_back(new Variable(Variable::BType::UNKNOWN, *$3, false, $5, $9));
                            }
          ;
%%
int main () {
  yyparse();
}
