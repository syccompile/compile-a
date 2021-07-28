#include "ast.h"
#include "analyzer.hh"
#include "ir_opt.h"
#include "reg_allocate/reg_allocate.h"
#include "ir_to_asm/ir_armify.h"
#include "ir_to_asm/translate_to_asm.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <filesystem>

extern int yylineno;

/** 全局变量声明  **/
std::vector<VarDeclStmt *> vardecl;
/** 函数声明  **/
std::vector<FunctionDecl *> funcs;

/* 优化级别 */
int optimize_level = 0;

int main(int argc, char *argv[]) {
  int ch;
  string asm_filename, ir_filename;
  while ((ch = getopt(argc, argv, "So:O::r:")) != -1) {
    switch (ch) {
      case 'S':
        // do nothing
        break;
      case 'o':
        asm_filename = optarg;
        break;
      case 'r':
        ir_filename = optarg;
        break;
      case 'O':
        if (optarg) {
          optimize_level = atoi(optarg);
        }
        break;
      default:
        std::cerr << "usage: compiler <sysy_file> [-S] [-o <asm_file>] [-O <optimize_level>] [-r <ir_file>]" << std::endl;
        exit(0);
    }
  }
  if (optind == argc) {
    std::cerr << "usage: compiler <sysy_file> [-S] [-o <asm_file>] [-O<optimize_level>] [-r <ir_file>]" << std::endl;
    exit(0);
  }
  std::filesystem::path sysy_file(argv[optind]);

  if (asm_filename.empty()) {
    asm_filename = sysy_file.stem().string() + ".s";
  }
  if (ir_filename.empty()) {
    ir_filename = sysy_file.stem().string() + ".ir";
  }

  freopen(sysy_file.c_str(), "r", stdin);
  yylineno = 1;
  yyparse();

  std::list<IR::List> def_list;
  std::list<IR::List> func_list;

  for (VarDeclStmt *stmt : vardecl) {
    def_list.emplace_back(stmt->translate());
  }
  for (FunctionDecl *f : funcs) {
    func_list.emplace_back(f->translate());
  }

  for (auto &func: func_list)
    remove_redunctant_label(func);
  
  for (auto &func: func_list) {
    IR::List def;
    ir_armify(def, func);
  }

  for (auto &func: func_list)
    register_allocate(func);

  // outputs
  std::ofstream IRFile(ir_filename);
  auto old_cout_buf = std::cout.rdbuf(IRFile.rdbuf());

  for (auto &def: def_list)
    for (auto ir: def)
      ir->internal_print();

  for (auto &func: func_list)
    for (auto ir: func)
      ir->internal_print();

  std::cout.rdbuf(old_cout_buf);
  IRFile.close();

  std::ofstream ASMFile(asm_filename);
  old_cout_buf = std::cout.rdbuf(ASMFile.rdbuf());

  for (auto &func: func_list) {
    auto asm_list = translate_function(func);
    for (auto &asm_line: asm_list) {
      std::cout << asm_line << std::endl;
    }
  }

  std::cout.rdbuf(old_cout_buf);
  ASMFile.close();
}
