#include "ast.h"
#include "analyzer.hh"

#include <vector>

extern int yylineno;

Frame::Ptr GlobFrame = nullptr;

/** 全局符号表 **/
SymbolTable::Ptr GlobSymTab = nullptr;

std::vector<VarDeclStmt*> vardecl;
std::vector<FunctionDecl*> funcs;

int main() {
  GlobFrame = std::make_shared<Frame>(true);
  GlobSymTab = std::make_shared<SymbolTable>(nullptr, GlobFrame);
  //GlobSymTab->set_frame(GlobFrame);
  yylineno = 1;
  yyparse();
  for(VarDeclStmt* stmt: vardecl){
    stmt->internal_print();
    wrap_tie(vec, access, stmt, GlobSymTab);
    for(auto ir : vec){
      ir->internal_print();
    }
  }
  for(FunctionDecl* f : funcs){
    f->internal_print();
    wrap_tie(vec, access, f, GlobSymTab);
    for(auto ir : vec){
      ir->internal_print();
    }
  }
}
