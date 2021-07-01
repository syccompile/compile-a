#include "ast.h"
#include "analyzer.hh"

#include <vector>

extern int yylineno;

/** 全局变量声明  **/
std::vector<VarDeclStmt *> vardecl;
/** 函数声明  **/
std::vector<FunctionDecl *> funcs;

int main() {
  yylineno = 1;
  yyparse();

  for (VarDeclStmt *stmt : vardecl) {
    stmt->translate();
  }
  for (FunctionDecl *f : funcs) {
    for (auto ir_ptr: f->translate()) ir_ptr->internal_print();
  }
}
