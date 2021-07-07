/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_MNT_F_C_COMPILER_COMPILE_A_CMAKE_BUILD_DEBUG_ANALYZER_HH_INCLUDED
# define YY_YY_MNT_F_C_COMPILER_COMPILE_A_CMAKE_BUILD_DEBUG_ANALYZER_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    LBRACKET = 258,
    RBRACKET = 259,
    LPARENT = 260,
    RPARENT = 261,
    LCURLY = 262,
    RCURLY = 263,
    INT = 264,
    VOID = 265,
    CONST = 266,
    RETURN = 267,
    IF = 268,
    BREAK = 269,
    WHILE = 270,
    CONTINUE = 271,
    ELSE = 272,
    SEMI = 273,
    COMMA = 274,
    ASSIGN = 275,
    EQ = 276,
    NEQ = 277,
    LT = 278,
    GT = 279,
    LE = 280,
    GE = 281,
    AND = 282,
    OR = 283,
    NUMBER = 284,
    ADD = 285,
    SUB = 286,
    MUL = 287,
    DIV = 288,
    MOD = 289,
    NOT = 290,
    IDENT = 291
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 19 "src/analyzer.y"

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

#line 121 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.hh"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_MNT_F_C_COMPILER_COMPILE_A_CMAKE_BUILD_DEBUG_ANALYZER_HH_INCLUDED  */
