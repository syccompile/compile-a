#include "ast.h"
#include "analyzer.hh"
#include "ir_opt.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>

extern int yylineno;

/** 全局变量声明  **/
std::vector<VarDeclStmt *> vardecl;
/** 函数声明  **/
std::vector<FunctionDecl *> funcs;

int main(int argc, char *argv[]) {
  if (argc < 4) {
    std::cerr << "usage: compiler <sysy_file> <ir_file> <asm_file>" << std::endl;
    exit(0);
  }
  freopen(argv[1], "r", stdin);
  yylineno = 1;
  yyparse();

  std::list<IR::Ptr> ir_list;
  for (VarDeclStmt *stmt : vardecl) {
    ir_list.splice(ir_list.end(), stmt->translate());
  }
  for (FunctionDecl *f : funcs) {
    ir_list.splice(ir_list.end(), f->translate());
  }

  remove_redunctant_label(ir_list);

  std::ofstream IRFile(argv[2]);
  auto old_cout_buf = std::cout.rdbuf(IRFile.rdbuf());
  for (const auto &i: ir_list) i->internal_print();
  std::cout.rdbuf(old_cout_buf);
  IRFile.close();

  // do some optimization for IR

  std::vector<std::string> asm_vector;
  for (const auto& ir: ir_list) {
    auto code = ir->translate_arm();
    asm_vector.insert(asm_vector.end(),
                      std::move_iterator(code.begin()),
                      std::move_iterator(code.end()));
  }

  // do some optimization for ASM

  std::ofstream ASMFile(argv[3]);
  for (const auto& code: asm_vector) {
    ASMFile << code << std::endl;
  }
  ASMFile.close();
}
