#include "ast.h"
#include "analyzer.hh"
#include "optimize.h"

#include <vector>

extern int yylineno;

/** 全局变量声明  **/
std::vector<VarDeclStmt *> vardecl;
/** 函数声明  **/
std::vector<FunctionDecl *> funcs;

int main() {
  yylineno = 1;
  yyparse();

  std::list<IR::Ptr> ir_list;
  for (VarDeclStmt *stmt : vardecl) {
    ir_list.splice(ir_list.end(), stmt->translate());
  }
  for (FunctionDecl *f : funcs) {
    ir_list.splice(ir_list.end(), f->translate());
  }

  for (const auto &i: ir_list) i->internal_print();
}
