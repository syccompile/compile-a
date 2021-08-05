%{
#include "ast.h"

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

extern int yylex();
extern int yylineno;
extern char* yytext;
void yyerror(const char* msg) { printf("line %d %s: yytext is %s\n", yylineno, msg, yytext); }
extern std::vector<VarDeclStmt*> vardecl;
extern std::vector<FunctionDecl*> funcs;

int parse_number(char const *);
%}

%union {
  int token;
  BType btype;
  std::string* string;

  Expression * exp;
  Expression::List * explist;

  Variable * var;
  Variable::List * varlist;

  Array::InitVal* initval;
  Array* array;

  Stmt* stmt;
  Stmt::List* stmtlist;
  VarDeclStmt * vardeclstmt;
  IfStmt * ifstmt;
  WhileStmt * whilestmt;
  BreakStmt * breakstmt;;
  ReturnStmt * retstmt;
  AssignmentStmt * assignstmt;
  BlockStmt * blockstmt;
  FunctionDecl* funcdecl;
  Variable::List* funcfparams;
}

%token <token> LBRACKET RBRACKET LPARENT RPARENT LCURLY RCURLY
%token <token> INT VOID 
%token <token> CONST RETURN IF BREAK WHILE CONTINUE ELSE
%token <token> SEMI COMMA ASSIGN
%token <token> EQ NEQ LT GT LE GE AND OR
%token <string> NUMBER
%token <token> ADD SUB MUL DIV MOD NOT
%token <string> IDENT
%token <string> STR

%type <token>       CompUnit
%type <btype>       BType
%type <exp>         Exp
%type <explist>     ExpList
%type <exp>         PrimaryExp UnaryExp MulExp RelExp EqExp LAndExp LOrExp AddExp
%type <vardeclstmt> VarDecl
%type <varlist>     VarDefList
%type <explist>     DimenList
%type <initval>     InitVals InitValsList

%type <stmt>        Stmt
%type <funcdecl>    FuncDef
%type <funcfparams> FuncFParams
%type <blockstmt>   BlockStmt
%type <blockstmt>   BlockItems

%nonassoc "then"
%nonassoc ELSE
%start CompUnit

%%
CompUnit: VarDecl { vardecl.push_back($1); }
        | FuncDef { funcs.push_back($1); }
        | CompUnit VarDecl { vardecl.push_back($2); }
        | CompUnit FuncDef { funcs.push_back($2); }
        ;

BType : INT { $$ =BType::INT; }
      | VOID { $$ = BType::VOID; }
      ;
Exp : AddExp { $$ = $1; }
    ;

AddExp  :  MulExp { $$ = $1; }
        |  AddExp ADD MulExp { $$ = new BinaryExp(Expression::Op::ADD, $1, $3); }
        |  AddExp SUB MulExp { $$ = new BinaryExp(Expression::Op::SUB, $1, $3); }
        ;

MulExp : UnaryExp { $$ = $1;}
       | MulExp MUL UnaryExp { $$ = new BinaryExp(Expression::Op::MUL, $1, $3); }
       | MulExp DIV UnaryExp { $$ = new BinaryExp(Expression::Op::DIV, $1, $3); }
       | MulExp MOD UnaryExp { $$ = new BinaryExp(Expression::Op::MOD, $1, $3); }

UnaryExp : PrimaryExp { $$ = $1; }
         | ADD UnaryExp { $$ = new UnaryExp(Expression::Op::ADD, $2); }
         | SUB UnaryExp { $$ = new UnaryExp(Expression::Op::SUB, $2); }
         | NOT UnaryExp { $$ = new UnaryExp(Expression::Op::NOT, $2); }
         | IDENT LPARENT ExpList RPARENT { $$ = new FuncCallExp($1, $3); delete $1; }
         | IDENT LPARENT RPARENT { $$ = new FuncCallExp($1, nullptr); delete $1; }
         ;
// 343 / 0xff / 03327 / a / a[10][1] / (...)
PrimaryExp : LPARENT LOrExp RPARENT { $$ = $2; }
           | NUMBER { $$ = new NumberExp(parse_number($1->c_str())); delete $1; }
           | IDENT  { $$ = new VarExp($1, nullptr); delete $1; }
           | IDENT DimenList  { $$ = new VarExp($1, $2); delete $1; }
           ;
// 4<=5 / 4>=5 / 4<5 / 4>5
RelExp : AddExp   { $$ = $1; }
       | RelExp LE AddExp { $$ = new BinaryExp(Expression::Op::LE, $1, $3); }
       | RelExp GE AddExp { $$ = new BinaryExp(Expression::Op::GE, $1, $3); }
       | RelExp LT AddExp { $$ = new BinaryExp(Expression::Op::LT, $1, $3); }
       | RelExp GT AddExp { $$ = new BinaryExp(Expression::Op::GT, $1, $3); }
       ;
// 4==5 / 4!=5
EqExp : RelExp { $$ = $1; }
      | EqExp EQ RelExp  { $$ = new BinaryExp(Expression::Op::EQ, $1, $3); }
      | EqExp NEQ RelExp { $$ = new BinaryExp(Expression::Op::NEQ, $1, $3); }
      ;
// 3&&4
LAndExp : EqExp { $$ = $1; }
        | LAndExp AND EqExp { $$ = new BinaryExp(Expression::Op::AND, $1, $3); }
        ;
// 3||4
LOrExp : LAndExp { $$ = $1; }
       | LOrExp OR LAndExp { $$ = new BinaryExp(Expression::Op::OR, $1, $3); }
       ;
// 3+4, 4*3, 2, ...
ExpList : Exp  { $$ = new Expression::List();
                 $$->push_back($1);
               }
	| STR  { $$ = new Expression::List(); }
        | ExpList COMMA Exp { $$ = $1;
                              $$->push_back($3);
                            }
	| ExpList COMMA STR { $$ = $1; }
        ;

// [const] int a = 21, b[10]... = {...}, ... ;
VarDecl: CONST BType VarDefList SEMI { $$ = new VarDeclStmt();  
                                      for(Variable * var : *$3){
                                        var->set_immutable(true);
                                        var->set_type($2);
                                        $$->push_back(var);
                                      }
                                   }
       | BType VarDefList SEMI { $$ = new VarDeclStmt();
                                for(Variable * var : *$2){
                                        var->set_immutable(false);
                                        var->set_type($1);
                                        $$->push_back(var);
                                }
                               }
       ; 
// [][4+4][2*4][6&3]...
DimenList: LBRACKET Exp RBRACKET { $$ = new Expression::List(); 
                                   $$->push_back($2);
                                  }
          | DimenList LBRACKET Exp RBRACKET { $1->push_back($3);
                                              $$ = $1;
                                             }
          | LBRACKET RBRACKET { $$ = new Expression::List();
                                $$->push_back(new NumberExp(0));
                              }
          ;
// 4,5,{...},4,5,{...},5...
InitValsList : Exp    { $$ = new Array::InitValContainer();
                        dynamic_cast<Array::InitValContainer*>($$)->push_back(new Array::InitValExp($1)); 
                      }
             | InitVals { $$ = new Array::InitValContainer();
                          dynamic_cast<Array::InitValContainer*>($$)->push_back($1); 
			}
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
          | VarDefList COMMA IDENT             { $$ = $1;
                                                 $$->push_back(new Variable(BType::UNKNOWN, $3, false));
                                                 delete $3;
                                               }
          | VarDefList COMMA IDENT DimenList
                      { 
                        $$ = $1; 
                        $$->push_back(new Array(BType::UNKNOWN, $3, false, $4));
                        delete $3;
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

Stmt :  SEMI    { $$ = new Stmt(); }
     |  Exp SEMI { $$ = new ExpStmt($1); }
     |  BlockStmt { $$ = $1; }
     |  RETURN SEMI { $$ = new ReturnStmt(nullptr); }
     |  RETURN Exp SEMI{ $$ = new ReturnStmt($2); }
     |  CONTINUE SEMI  { $$ = new ContinueStmt(); }
     |  BREAK SEMI  { $$ = new BreakStmt(); }
     |  VarDecl { $$ = $1; }
     |  IDENT ASSIGN Exp SEMI{ $$ = new AssignmentStmt($1, nullptr, $3); delete $1; }
     |  IDENT DimenList ASSIGN Exp SEMI { $$ = new AssignmentStmt($1, $2, $4); delete $1; }
     |  IF LPARENT LOrExp RPARENT Stmt ELSE Stmt { 
               BlockStmt* block_yes = dynamic_cast<BlockStmt*>($5);
               if (block_yes == nullptr){
                  block_yes = new BlockStmt();
                  block_yes->push_back($5);
               }
               BlockStmt* block_else = dynamic_cast<BlockStmt*>($7);
               if (block_else == nullptr){
                  block_else = new BlockStmt();
                  block_else->push_back($7);
               }
               $$ = new IfStmt($3, block_yes, block_else); }
     |  IF LPARENT LOrExp RPARENT Stmt %prec "then"{ 
               BlockStmt* block = dynamic_cast<BlockStmt*>($5);
               if (block == nullptr){
                  block = new BlockStmt();
                  block->push_back($5);
               }
               $$ = new IfStmt($3, block); }
     |  WHILE LPARENT LOrExp RPARENT Stmt { 
               BlockStmt* block = dynamic_cast<BlockStmt*>($5);
               if (block == nullptr){
                  block = new BlockStmt();
                  block->push_back($5);
               }
               $$ = new WhileStmt($3, block); }
     ;
BlockItems : Stmt { $$ = new BlockStmt(); $$->push_back($1); }
           | BlockItems Stmt { $$ = $1; $$->push_back($2); }
           ;
BlockStmt : LCURLY RCURLY { $$ = new BlockStmt(); }
          | LCURLY BlockItems RCURLY { $$ = $2; }
          ;
// int a, int b[][10], ...
FuncFParams : BType IDENT             { $$ = new Variable::List(); 
                                        $$->push_back(new Variable($1, $2, false)); 
                                        delete $2;
                                      }
            | BType IDENT DimenList   { $$ = new Variable::List();
                                        $$->push_back(new Array($1, $2, false, $3));
                                        delete $2;
                                      }
            | FuncFParams COMMA BType IDENT { $$ = $1; 
                                              $$->push_back(new Variable($3, $4, false));
                                              delete $4;
                                            }
            | FuncFParams COMMA BType IDENT DimenList { $$ = $1;
                                                        $$->push_back(new Array($3,  $4, false, $5));
                                                        delete $4;
                                                      }
            ;
// void func(int a, int b[][10], ...) {...}
FuncDef : BType IDENT LPARENT FuncFParams RPARENT BlockStmt  { $$ = new FunctionDecl($1, $2, $4, $6); delete $2; }
        | BType IDENT LPARENT RPARENT BlockStmt          { $$ = new FunctionDecl($1, $2, nullptr, $5); delete $2; }
        ;
%%

int parse_hex(char const *num) {
  int ret = 0;
  while ((*num)!='\0') {
    ret <<= 4u;
    if (*num >= 'a')      ret = ret + *num - 'a' + 10;
    else if (*num >= 'A') ret = ret + *num - 'A' + 10;
    else                  ret = ret + *num - '0';
    num++;
  }
  return ret;
}

int parse_oct(char const *num) {
  int ret = 0;
  while ((*num)!='\0') {
    ret <<= 3u;
    ret = ret + *num - '0';
    num++;
  }
  return ret;
}

int parse_dec(char const *num) {
  int ret = 0;
  while ((*num)!='\0') {
    ret *= 10;
    ret = ret + *num - '0';
    num++;
  }
  return ret;
}

int parse_number(char const *num) {
  if ((*num)=='0') {
    ++num;
    if ((*num)=='X' || (*num)=='x') return std::stoi(num-1, 0, 16);
    else                            return std::stoi(num-1, 0, 8);
  }
  return std::stoi(num);
}
