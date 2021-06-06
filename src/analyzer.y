%{
#include "ast.h"

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

extern int yylex();
extern char* yytext;
void yyerror(const char* msg) { printf("%s: yytext is %s\n", msg, yytext); }
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
  BType btype;
  Array::InitVal* initval;
  Array* array;
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
%token <token> INT VOID CONST SEMI COMMA ASSIGN
%token <string> NUMBER
%token <token> ADD SUB MUL DIV MOD NOT
%token <string> IDENT

%type <token>  CompUnit
%type <btype>  BType
%type <vardeclstmt> VarDecl
%type <var> VarDef
%type <varlist> VarDefList
%type <initval> InitVals InitValsList
%type <func> FuncDef
%type <fparams> FuncParams
%type <blockstmt> Block
%type <exp> Exp
%type <explist> ExpList
%type <explist> DimenList
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

// int 
BType : INT { $$ =BType::INT; }
      | VOID { $$ = BType::VOID; }
      ;

Exp  :  MulExp { $$ = $1; }
     |  Exp ADD MulExp { $$ = new Expression(Expression::Op::ADD, $1, $3); }
     ;

MulExp : UnaryExp { $$ = $1;}
       | MulExp MUL UnaryExp { $$ = new Expression(Expression::Op::MUL, $1, $3); }
       | MulExp DIV UnaryExp { $$ = new Expression(Expression::Op::DIV, $1, $3); }
       | MulExp MOD UnaryExp { $$ = new Expression(Expression::Op::MOD, $1, $3); }

// TODO func call
UnaryExp : PrimaryExp { $$ = $1; }
         | ADD UnaryExp { $$ = new Expression(Expression::Op::ADD, $2); }
         | SUB UnaryExp { $$ = new Expression(Expression::Op::SUB, $2); }
         | NOT UnaryExp { $$ = new Expression(Expression::Op::NOT, $2); }
         ;
// TODO
PrimaryExp : LPARENT Exp RPARENT { $$ = $2; }
           | NUMBER { $$ = new Expression($1); delete $1; }
           | IDENT  { $$ = new Expression(new Variable(BType::UNKNOWN, $1, false)); delete $1; }
           | IDENT DimenList  { $$ = new Expression(new Array(BType::UNKNOWN, $1, false, $2)); delete $1; }
           ;

// 3+4, 4*3, 2, ...
ExpList : Exp  { $$ = new Expression::List();
                 $$->push_back($1);
               }
        | ExpList COMMA Exp { $$ = $1;
                              $$->push_back($3);
                            }
        ;

// [const] int a = 21, b[10]... = {...}, ... ;
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
// [4+4][2*4][6&3]...
DimenList: LBRACKET Exp RBRACKET { $$ = new Expression::List(); 
                                   $$->push_back($2);
                                  }
          | DimenList LBRACKET Exp RBRACKET { $1->push_back($3);
                                              $$ = $1;
                                             }
          ;
// 4,5,{...},4,5,{...},5...
InitValsList : Exp    { $$ = new Array::InitValContainer();
                        dynamic_cast<Array::InitValContainer*>($$)->push_back(new Array::InitValExp($1)); 
                      }
             | InitVals                     { $$ = $1;}
             | InitValsList COMMA Exp       
                       { 
                         $$ = $1; 
                         dynamic_cast<Array::InitValContainer*>($$)->push_back(new Array::InitValExp($3)); 
                       }
             | InitValsList COMMA InitVals  { $$ = $1; 
                           dynamic_cast<Array::InitValContainer*>($$)->push_back($3); 
                          }
             ;
// {...}
InitVals : LCURLY RCURLY              { $$ = new Array::InitValContainer(); }
         | LCURLY InitValsList RCURLY { $$ = $2; }
         ;

// a = 10, b[10][3]... = {...}, ...
VarDefList: IDENT ASSIGN Exp                   { $$ = new Variable::List();
                                                 $$->push_back(new Variable(BType::UNKNOWN, $1, false, $3));
                                                 delete $1;
                                               }
          | IDENT DimenList ASSIGN InitVals    { $$ = new Variable::List();
                                                 $$->push_back(new Array(BType::UNKNOWN, $1, false, $2, $4));
                                                 delete $1;
                                               }
          | IDENT                              { $$ = new Variable::List();
                                                 $$->push_back(new Variable(BType::UNKNOWN, $1, false));
                                                 delete $1;
                                               }
          | IDENT DimenList                    { $$ = new Variable::List();
                                                 $$->push_back(new Array(BType::UNKNOWN, $1, false, $2));
                                                 delete $1;
                                               }
          | VarDefList COMMA IDENT ASSIGN Exp  { $$ = $1;
                                                 $$->push_back(new Variable(BType::UNKNOWN, $3, false, $5));
                                                 delete $3;
                                               }
          | VarDefList COMMA IDENT DimenList ASSIGN InitVals 
                      { 
                        $$ = $1; 
                        $$->push_back(new Array(BType::UNKNOWN, $3, false, $4, $6));
                        delete $3;
                      } 
          ;
%%
int main () {
  yyparse();
  for(VarDeclStmt* stmt: vardecl){
    for(Variable* var : stmt->vars()){
      var->internal_print();
    }
    std::cout << std::endl;
  }
}
