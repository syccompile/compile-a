#include "ast.h"
#include "analyzer.hh"

#include <vector>

extern int yylineno;

/** 全局栈帧 **/
Frame::Ptr GlobFrame = std::make_shared<Frame>(true);

/** 全局符号表 **/
SymbolTable::Ptr GlobSymTab = std::make_shared<SymbolTable>(nullptr, GlobFrame);

/** 全局变量声明  **/
std::vector<VarDeclStmt *> vardecl;
/** 函数声明  **/
std::vector<FunctionDecl *> funcs;

int main() {
  yylineno = 1;
  yyparse();
  for (VarDeclStmt *stmt : vardecl) {
    stmt->internal_print();
    wrap_tie(vec, access, stmt, GlobSymTab);
    for (auto ir : vec) {
      ir->internal_print();
    }
  }
  for (FunctionDecl *f : funcs) {
    f->internal_print();
    wrap_tie(vec, access, f, GlobSymTab);
    for (auto ir : vec) {
      ir->internal_print();
    }
  }
}
