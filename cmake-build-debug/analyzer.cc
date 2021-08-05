/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "src/analyzer.y"

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

#line 88 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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

#line 204 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_MNT_F_C_COMPILER_COMPILE_A_CMAKE_BUILD_DEBUG_ANALYZER_HH_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  9
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   228

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  38
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  23
/* YYNRULES -- Number of rules.  */
#define YYNRULES  81
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  152

#define YYUNDEFTOK  2
#define YYMAXUTOK   292


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    76,    76,    77,    78,    79,    82,    83,    85,    88,
      89,    90,    93,    94,    95,    96,    98,    99,   100,   101,
     102,   103,   106,   107,   108,   109,   112,   113,   114,   115,
     116,   119,   120,   121,   124,   125,   128,   129,   132,   135,
     141,   148,   157,   160,   163,   168,   171,   174,   179,   184,
     185,   189,   193,   197,   201,   205,   209,   215,   219,   227,
     228,   229,   230,   231,   232,   233,   234,   235,   236,   237,
     249,   256,   264,   265,   267,   268,   271,   275,   279,   283,
     289,   290
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LBRACKET", "RBRACKET", "LPARENT",
  "RPARENT", "LCURLY", "RCURLY", "INT", "VOID", "CONST", "RETURN", "IF",
  "BREAK", "WHILE", "CONTINUE", "ELSE", "SEMI", "COMMA", "ASSIGN", "EQ",
  "NEQ", "LT", "GT", "LE", "GE", "AND", "OR", "NUMBER", "ADD", "SUB",
  "MUL", "DIV", "MOD", "NOT", "IDENT", "\"then\"", "$accept", "CompUnit",
  "BType", "Exp", "AddExp", "MulExp", "UnaryExp", "PrimaryExp", "RelExp",
  "EqExp", "LAndExp", "LOrExp", "ExpList", "VarDecl", "DimenList",
  "InitValsList", "InitVals", "VarDefList", "Stmt", "BlockItems",
  "BlockStmt", "FuncFParams", "FuncDef", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292
};
# endif

#define YYPACT_NINF (-83)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     132,   -83,   -83,    74,   110,   -12,   -83,   -83,    -8,   -83,
     -83,   -83,    22,    89,    18,   118,     1,   197,    44,    43,
     -83,     8,   -83,   -83,    44,   -83,    44,    44,    44,    17,
      84,   119,   139,   -83,   -83,    40,    30,    66,   -83,    44,
      94,    57,   119,   185,   180,    76,     5,   -83,   -83,   -83,
     109,   131,   -83,    44,    44,    44,    44,    44,    82,   -83,
     148,    40,    74,   151,   161,   -83,    44,    61,    44,    44,
      44,    44,    44,    44,    44,   -83,    44,   -83,   -83,    80,
     139,   139,   -83,   -83,   -83,   -83,   152,   170,   168,   172,
     171,   -83,    38,    -8,   198,   -83,   -83,   117,   -83,   131,
     -83,   181,   -83,   -83,   -83,     0,   -83,   -83,    94,   119,
     119,   119,   119,   185,   185,   180,    76,   -83,    44,   -83,
     200,    44,   -83,    44,   -83,    44,    62,   -83,   -83,   -83,
     148,   -83,   169,   -83,   -83,   -83,    11,    12,   201,    44,
     131,   -83,   -83,   149,   149,   -83,   202,   204,   -83,   -83,
     149,   -83
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     6,     7,     0,     0,     0,     2,     3,     0,     1,
       4,     5,    53,     0,    53,     0,     0,     0,     0,    54,
      41,     0,    40,    44,     0,    23,     0,     0,     0,    24,
       0,     8,     9,    12,    16,     0,     0,     0,    51,     0,
       0,    55,    26,    31,    34,    36,     0,    17,    18,    19,
       0,    25,    42,     0,     0,     0,     0,     0,     0,    81,
      76,     0,     0,     0,     0,    52,     0,    56,     0,     0,
       0,     0,     0,     0,     0,    22,     0,    21,    38,     0,
      10,    11,    13,    14,    15,    74,     0,     0,     0,     0,
       0,    59,    24,     0,     0,    66,    72,     0,    61,    77,
      80,     0,    43,    49,    45,     0,    46,    57,     0,    29,
      30,    27,    28,    32,    33,    35,    37,    20,     0,    62,
       0,     0,    65,     0,    64,     0,    25,    60,    75,    73,
      78,    50,     0,    58,    39,    63,     0,     0,     0,     0,
      79,    47,    48,     0,     0,    67,     0,    70,    71,    68,
       0,    69
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -83,   -83,     9,   -16,   -17,   159,   167,   -83,   142,   153,
     150,   -45,   -83,    10,   -25,   -83,   -63,   217,   -82,   -83,
     -32,   -83,   224
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,    93,    94,    31,    32,    33,    34,    43,    44,
      45,    46,    79,    95,    19,   105,    65,    13,    96,    97,
      98,    37,     7
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      30,   106,    38,    59,    51,    23,    24,    42,   131,     5,
       6,    75,     8,     5,    10,   129,    67,   143,   144,   132,
      16,    16,    50,    63,    12,    16,    36,    17,    14,   100,
      25,    26,    27,    76,    78,    99,    28,    29,    18,    76,
      76,    16,    18,    50,    41,   133,    39,    58,   104,    24,
     107,   109,   110,   111,   112,    42,    42,    42,   125,    42,
      16,   147,   148,    40,    39,    39,    60,   126,   151,   142,
     120,   101,    61,    25,    26,    27,   136,    66,   137,    28,
      29,   108,   139,     1,     2,    62,   117,    24,    52,    58,
      85,     1,     2,     3,    86,    87,    88,    89,    90,   118,
      91,    64,   134,    74,    42,   140,    42,    20,    21,   138,
       9,    25,    26,    27,    24,    77,   141,    28,    92,     1,
       2,     3,    24,   146,    58,   128,     1,     2,     3,    86,
      87,    88,    89,    90,    39,    91,    22,    21,    25,    26,
      27,     1,     2,     3,    28,    29,    25,    26,    27,    53,
      54,    16,    28,    92,    24,   102,    58,    24,     1,     2,
       3,    86,    87,    88,    89,    90,    24,    91,    64,   103,
     119,    55,    56,    57,    24,   121,    64,   123,    25,    26,
      27,    25,    26,    27,    28,    92,   122,    28,    29,   124,
      25,    26,    27,    47,    48,    49,    28,    29,    25,    26,
      27,    72,    73,    35,    28,    29,     1,     2,    68,    69,
      70,    71,    80,    81,   113,   114,   127,   130,   135,   145,
     149,   150,    82,    83,    84,    15,   116,   115,    11
};

static const yytype_uint8 yycheck[] =
{
      16,    64,    18,    35,    29,     4,     5,    24,     8,     0,
       0,     6,     3,     4,     4,    97,    41,     6,     6,    19,
       3,     3,     5,    39,    36,     3,    17,     5,    36,    61,
      29,    30,    31,    28,    50,    60,    35,    36,    20,    28,
      28,     3,    20,     5,    36,   108,     3,     7,    64,     5,
      66,    68,    69,    70,    71,    72,    73,    74,    20,    76,
       3,   143,   144,    20,     3,     3,    36,    92,   150,   132,
      86,    62,     6,    29,    30,    31,   121,    20,   123,    35,
      36,    20,    20,     9,    10,    19,     6,     5,     4,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    19,
      18,     7,   118,    27,   121,   130,   123,    18,    19,   125,
       0,    29,    30,    31,     5,     6,   132,    35,    36,     9,
      10,    11,     5,   139,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,     3,    18,    18,    19,    29,    30,
      31,     9,    10,    11,    35,    36,    29,    30,    31,    30,
      31,     3,    35,    36,     5,     4,     7,     5,     9,    10,
      11,    12,    13,    14,    15,    16,     5,    18,     7,     8,
      18,    32,    33,    34,     5,     5,     7,     5,    29,    30,
      31,    29,    30,    31,    35,    36,    18,    35,    36,    18,
      29,    30,    31,    26,    27,    28,    35,    36,    29,    30,
      31,    21,    22,     6,    35,    36,     9,    10,    23,    24,
      25,    26,    53,    54,    72,    73,    18,    36,    18,    18,
      18,    17,    55,    56,    57,     8,    76,    74,     4
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     9,    10,    11,    39,    40,    51,    60,    40,     0,
      51,    60,    36,    55,    36,    55,     3,     5,    20,    52,
      18,    19,    18,     4,     5,    29,    30,    31,    35,    36,
      41,    42,    43,    44,    45,     6,    40,    59,    41,     3,
      20,    36,    42,    46,    47,    48,    49,    44,    44,    44,
       5,    52,     4,    30,    31,    32,    33,    34,     7,    58,
      36,     6,    19,    41,     7,    54,    20,    52,    23,    24,
      25,    26,    21,    22,    27,     6,    28,     6,    41,    50,
      43,    43,    44,    44,    44,     8,    12,    13,    14,    15,
      16,    18,    36,    40,    41,    51,    56,    57,    58,    52,
      58,    40,     4,     8,    41,    53,    54,    41,    20,    42,
      42,    42,    42,    46,    46,    47,    48,     6,    19,    18,
      41,     5,    18,     5,    18,    20,    52,    18,     8,    56,
      36,     8,    19,    54,    41,    18,    49,    49,    41,    20,
      52,    41,    54,     6,     6,    18,    41,    56,    56,    18,
      17,    56
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    38,    39,    39,    39,    39,    40,    40,    41,    42,
      42,    42,    43,    43,    43,    43,    44,    44,    44,    44,
      44,    44,    45,    45,    45,    45,    46,    46,    46,    46,
      46,    47,    47,    47,    48,    48,    49,    49,    50,    50,
      51,    51,    52,    52,    52,    53,    53,    53,    53,    54,
      54,    55,    55,    55,    55,    55,    55,    55,    55,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    57,    57,    58,    58,    59,    59,    59,    59,
      60,    60
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     2,     2,     1,     1,     1,     1,
       3,     3,     1,     3,     3,     3,     1,     2,     2,     2,
       4,     3,     3,     1,     1,     2,     1,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     1,     3,     1,     3,
       4,     3,     3,     4,     2,     1,     1,     3,     3,     2,
       3,     3,     4,     1,     2,     3,     4,     5,     6,     1,
       2,     1,     2,     3,     2,     2,     1,     4,     5,     7,
       5,     5,     1,     2,     2,     3,     2,     3,     4,     5,
       6,     5
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 76 "src/analyzer.y"
                  { vardecl.push_back((yyvsp[0].vardeclstmt)); }
#line 1497 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 3:
#line 77 "src/analyzer.y"
                  { funcs.push_back((yyvsp[0].funcdecl)); }
#line 1503 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 4:
#line 78 "src/analyzer.y"
                           { vardecl.push_back((yyvsp[0].vardeclstmt)); }
#line 1509 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 5:
#line 79 "src/analyzer.y"
                           { funcs.push_back((yyvsp[0].funcdecl)); }
#line 1515 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 6:
#line 82 "src/analyzer.y"
            { (yyval.btype) =BType::INT; }
#line 1521 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 7:
#line 83 "src/analyzer.y"
             { (yyval.btype) = BType::VOID; }
#line 1527 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 8:
#line 85 "src/analyzer.y"
             { (yyval.exp) = (yyvsp[0].exp); }
#line 1533 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 9:
#line 88 "src/analyzer.y"
                  { (yyval.exp) = (yyvsp[0].exp); }
#line 1539 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 10:
#line 89 "src/analyzer.y"
                             { (yyval.exp) = new BinaryExp(Expression::Op::ADD, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1545 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 11:
#line 90 "src/analyzer.y"
                             { (yyval.exp) = new BinaryExp(Expression::Op::SUB, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1551 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 12:
#line 93 "src/analyzer.y"
                  { (yyval.exp) = (yyvsp[0].exp);}
#line 1557 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 13:
#line 94 "src/analyzer.y"
                             { (yyval.exp) = new BinaryExp(Expression::Op::MUL, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1563 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 14:
#line 95 "src/analyzer.y"
                             { (yyval.exp) = new BinaryExp(Expression::Op::DIV, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1569 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 15:
#line 96 "src/analyzer.y"
                             { (yyval.exp) = new BinaryExp(Expression::Op::MOD, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1575 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 16:
#line 98 "src/analyzer.y"
                      { (yyval.exp) = (yyvsp[0].exp); }
#line 1581 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 17:
#line 99 "src/analyzer.y"
                        { (yyval.exp) = new UnaryExp(Expression::Op::ADD, (yyvsp[0].exp)); }
#line 1587 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 18:
#line 100 "src/analyzer.y"
                        { (yyval.exp) = new UnaryExp(Expression::Op::SUB, (yyvsp[0].exp)); }
#line 1593 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 19:
#line 101 "src/analyzer.y"
                        { (yyval.exp) = new UnaryExp(Expression::Op::NOT, (yyvsp[0].exp)); }
#line 1599 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 20:
#line 102 "src/analyzer.y"
                                         { (yyval.exp) = new FuncCallExp((yyvsp[-3].string), (yyvsp[-1].explist)); delete (yyvsp[-3].string); }
#line 1605 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 21:
#line 103 "src/analyzer.y"
                                 { (yyval.exp) = new FuncCallExp((yyvsp[-2].string), nullptr); delete (yyvsp[-2].string); }
#line 1611 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 22:
#line 106 "src/analyzer.y"
                                    { (yyval.exp) = (yyvsp[-1].exp); }
#line 1617 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 23:
#line 107 "src/analyzer.y"
                    { (yyval.exp) = new NumberExp(parse_number((yyvsp[0].string)->c_str())); delete (yyvsp[0].string); }
#line 1623 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 24:
#line 108 "src/analyzer.y"
                    { (yyval.exp) = new VarExp((yyvsp[0].string), nullptr); delete (yyvsp[0].string); }
#line 1629 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 25:
#line 109 "src/analyzer.y"
                              { (yyval.exp) = new VarExp((yyvsp[-1].string), (yyvsp[0].explist)); delete (yyvsp[-1].string); }
#line 1635 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 26:
#line 112 "src/analyzer.y"
                  { (yyval.exp) = (yyvsp[0].exp); }
#line 1641 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 27:
#line 113 "src/analyzer.y"
                          { (yyval.exp) = new BinaryExp(Expression::Op::LE, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1647 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 28:
#line 114 "src/analyzer.y"
                          { (yyval.exp) = new BinaryExp(Expression::Op::GE, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1653 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 29:
#line 115 "src/analyzer.y"
                          { (yyval.exp) = new BinaryExp(Expression::Op::LT, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1659 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 30:
#line 116 "src/analyzer.y"
                          { (yyval.exp) = new BinaryExp(Expression::Op::GT, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1665 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 31:
#line 119 "src/analyzer.y"
               { (yyval.exp) = (yyvsp[0].exp); }
#line 1671 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 32:
#line 120 "src/analyzer.y"
                         { (yyval.exp) = new BinaryExp(Expression::Op::EQ, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1677 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 33:
#line 121 "src/analyzer.y"
                         { (yyval.exp) = new BinaryExp(Expression::Op::NEQ, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1683 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 34:
#line 124 "src/analyzer.y"
                { (yyval.exp) = (yyvsp[0].exp); }
#line 1689 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 35:
#line 125 "src/analyzer.y"
                            { (yyval.exp) = new BinaryExp(Expression::Op::AND, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1695 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 36:
#line 128 "src/analyzer.y"
                 { (yyval.exp) = (yyvsp[0].exp); }
#line 1701 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 37:
#line 129 "src/analyzer.y"
                           { (yyval.exp) = new BinaryExp(Expression::Op::OR, (yyvsp[-2].exp), (yyvsp[0].exp)); }
#line 1707 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 38:
#line 132 "src/analyzer.y"
               { (yyval.explist) = new Expression::List();
                 (yyval.explist)->push_back((yyvsp[0].exp));
               }
#line 1715 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 39:
#line 135 "src/analyzer.y"
                            { (yyval.explist) = (yyvsp[-2].explist);
                              (yyval.explist)->push_back((yyvsp[0].exp));
                            }
#line 1723 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 40:
#line 141 "src/analyzer.y"
                                     { (yyval.vardeclstmt) = new VarDeclStmt();  
                                      for(Variable * var : *(yyvsp[-1].varlist)){
                                        var->set_immutable(true);
                                        var->set_type((yyvsp[-2].btype));
                                        (yyval.vardeclstmt)->push_back(var);
                                      }
                                   }
#line 1735 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 41:
#line 148 "src/analyzer.y"
                               { (yyval.vardeclstmt) = new VarDeclStmt();
                                for(Variable * var : *(yyvsp[-1].varlist)){
                                        var->set_immutable(false);
                                        var->set_type((yyvsp[-2].btype));
                                        (yyval.vardeclstmt)->push_back(var);
                                }
                               }
#line 1747 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 42:
#line 157 "src/analyzer.y"
                                 { (yyval.explist) = new Expression::List(); 
                                   (yyval.explist)->push_back((yyvsp[-1].exp));
                                  }
#line 1755 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 43:
#line 160 "src/analyzer.y"
                                            { (yyvsp[-3].explist)->push_back((yyvsp[-1].exp));
                                              (yyval.explist) = (yyvsp[-3].explist);
                                             }
#line 1763 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 44:
#line 163 "src/analyzer.y"
                              { (yyval.explist) = new Expression::List();
                                (yyval.explist)->push_back(new NumberExp(0));
                              }
#line 1771 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 45:
#line 168 "src/analyzer.y"
                      { (yyval.initval) = new Array::InitValContainer();
                        dynamic_cast<Array::InitValContainer*>((yyval.initval))->push_back(new Array::InitValExp((yyvsp[0].exp))); 
                      }
#line 1779 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 46:
#line 171 "src/analyzer.y"
                        { (yyval.initval) = new Array::InitValContainer();
                          dynamic_cast<Array::InitValContainer*>((yyval.initval))->push_back((yyvsp[0].initval)); 
			}
#line 1787 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 47:
#line 175 "src/analyzer.y"
                       { 
                         (yyval.initval) = (yyvsp[-2].initval); 
                         dynamic_cast<Array::InitValContainer*>((yyval.initval))->push_back(new Array::InitValExp((yyvsp[0].exp))); 
                       }
#line 1796 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 48:
#line 179 "src/analyzer.y"
                                            { (yyval.initval) = (yyvsp[-2].initval); 
                           dynamic_cast<Array::InitValContainer*>((yyval.initval))->push_back((yyvsp[0].initval)); 
                          }
#line 1804 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 49:
#line 184 "src/analyzer.y"
                                      { (yyval.initval) = new Array::InitValContainer(); }
#line 1810 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 50:
#line 185 "src/analyzer.y"
                                      { (yyval.initval) = (yyvsp[-1].initval); }
#line 1816 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 51:
#line 189 "src/analyzer.y"
                                               { (yyval.varlist) = new Variable::List();
                                                 (yyval.varlist)->push_back(new Variable(BType::UNKNOWN, (yyvsp[-2].string), false, (yyvsp[0].exp)));
                                                 delete (yyvsp[-2].string);
                                               }
#line 1825 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 52:
#line 193 "src/analyzer.y"
                                               { (yyval.varlist) = new Variable::List();
                                                 (yyval.varlist)->push_back(new Array(BType::UNKNOWN, (yyvsp[-3].string), false, (yyvsp[-2].explist), (yyvsp[0].initval)));
                                                 delete (yyvsp[-3].string);
                                               }
#line 1834 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 53:
#line 197 "src/analyzer.y"
                                               { (yyval.varlist) = new Variable::List();
                                                 (yyval.varlist)->push_back(new Variable(BType::UNKNOWN, (yyvsp[0].string), false));
                                                 delete (yyvsp[0].string);
                                               }
#line 1843 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 54:
#line 201 "src/analyzer.y"
                                               { (yyval.varlist) = new Variable::List();
                                                 (yyval.varlist)->push_back(new Array(BType::UNKNOWN, (yyvsp[-1].string), false, (yyvsp[0].explist)));
                                                 delete (yyvsp[-1].string);
                                               }
#line 1852 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 55:
#line 205 "src/analyzer.y"
                                               { (yyval.varlist) = (yyvsp[-2].varlist);
                                                 (yyval.varlist)->push_back(new Variable(BType::UNKNOWN, (yyvsp[0].string), false));
                                                 delete (yyvsp[0].string);
                                               }
#line 1861 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 56:
#line 210 "src/analyzer.y"
                      { 
                        (yyval.varlist) = (yyvsp[-3].varlist); 
                        (yyval.varlist)->push_back(new Array(BType::UNKNOWN, (yyvsp[-1].string), false, (yyvsp[0].explist)));
                        delete (yyvsp[-1].string);
                      }
#line 1871 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 57:
#line 215 "src/analyzer.y"
                                               { (yyval.varlist) = (yyvsp[-4].varlist);
                                                 (yyval.varlist)->push_back(new Variable(BType::UNKNOWN, (yyvsp[-2].string), false, (yyvsp[0].exp)));
                                                 delete (yyvsp[-2].string);
                                               }
#line 1880 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 58:
#line 220 "src/analyzer.y"
                      { 
                        (yyval.varlist) = (yyvsp[-5].varlist); 
                        (yyval.varlist)->push_back(new Array(BType::UNKNOWN, (yyvsp[-3].string), false, (yyvsp[-2].explist), (yyvsp[0].initval)));
                        delete (yyvsp[-3].string);
                      }
#line 1890 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 59:
#line 227 "src/analyzer.y"
                { (yyval.stmt) = new Stmt(); }
#line 1896 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 60:
#line 228 "src/analyzer.y"
                 { (yyval.stmt) = new ExpStmt((yyvsp[-1].exp)); }
#line 1902 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 61:
#line 229 "src/analyzer.y"
                  { (yyval.stmt) = (yyvsp[0].blockstmt); }
#line 1908 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 62:
#line 230 "src/analyzer.y"
                    { (yyval.stmt) = new ReturnStmt(nullptr); }
#line 1914 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 63:
#line 231 "src/analyzer.y"
                       { (yyval.stmt) = new ReturnStmt((yyvsp[-1].exp)); }
#line 1920 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 64:
#line 232 "src/analyzer.y"
                       { (yyval.stmt) = new ContinueStmt(); }
#line 1926 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 65:
#line 233 "src/analyzer.y"
                    { (yyval.stmt) = new BreakStmt(); }
#line 1932 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 66:
#line 234 "src/analyzer.y"
                { (yyval.stmt) = (yyvsp[0].vardeclstmt); }
#line 1938 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 67:
#line 235 "src/analyzer.y"
                             { (yyval.stmt) = new AssignmentStmt((yyvsp[-3].string), nullptr, (yyvsp[-1].exp)); delete (yyvsp[-3].string); }
#line 1944 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 68:
#line 236 "src/analyzer.y"
                                        { (yyval.stmt) = new AssignmentStmt((yyvsp[-4].string), (yyvsp[-3].explist), (yyvsp[-1].exp)); delete (yyvsp[-4].string); }
#line 1950 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 69:
#line 237 "src/analyzer.y"
                                                 { 
               BlockStmt* block_yes = dynamic_cast<BlockStmt*>((yyvsp[-2].stmt));
               if (block_yes == nullptr){
                  block_yes = new BlockStmt();
                  block_yes->push_back((yyvsp[-2].stmt));
               }
               BlockStmt* block_else = dynamic_cast<BlockStmt*>((yyvsp[0].stmt));
               if (block_else == nullptr){
                  block_else = new BlockStmt();
                  block_else->push_back((yyvsp[0].stmt));
               }
               (yyval.stmt) = new IfStmt((yyvsp[-4].exp), block_yes, block_else); }
#line 1967 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 70:
#line 249 "src/analyzer.y"
                                                   { 
               BlockStmt* block = dynamic_cast<BlockStmt*>((yyvsp[0].stmt));
               if (block == nullptr){
                  block = new BlockStmt();
                  block->push_back((yyvsp[0].stmt));
               }
               (yyval.stmt) = new IfStmt((yyvsp[-2].exp), block); }
#line 1979 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 71:
#line 256 "src/analyzer.y"
                                          { 
               BlockStmt* block = dynamic_cast<BlockStmt*>((yyvsp[0].stmt));
               if (block == nullptr){
                  block = new BlockStmt();
                  block->push_back((yyvsp[0].stmt));
               }
               (yyval.stmt) = new WhileStmt((yyvsp[-2].exp), block); }
#line 1991 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 72:
#line 264 "src/analyzer.y"
                  { (yyval.blockstmt) = new BlockStmt(); (yyval.blockstmt)->push_back((yyvsp[0].stmt)); }
#line 1997 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 73:
#line 265 "src/analyzer.y"
                             { (yyval.blockstmt) = (yyvsp[-1].blockstmt); (yyval.blockstmt)->push_back((yyvsp[0].stmt)); }
#line 2003 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 74:
#line 267 "src/analyzer.y"
                          { (yyval.blockstmt) = new BlockStmt(); }
#line 2009 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 75:
#line 268 "src/analyzer.y"
                                     { (yyval.blockstmt) = (yyvsp[-1].blockstmt); }
#line 2015 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 76:
#line 271 "src/analyzer.y"
                                      { (yyval.funcfparams) = new Variable::List(); 
                                        (yyval.funcfparams)->push_back(new Variable((yyvsp[-1].btype), (yyvsp[0].string), false)); 
                                        delete (yyvsp[0].string);
                                      }
#line 2024 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 77:
#line 275 "src/analyzer.y"
                                      { (yyval.funcfparams) = new Variable::List();
                                        (yyval.funcfparams)->push_back(new Array((yyvsp[-2].btype), (yyvsp[-1].string), false, (yyvsp[0].explist)));
                                        delete (yyvsp[-1].string);
                                      }
#line 2033 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 78:
#line 279 "src/analyzer.y"
                                            { (yyval.funcfparams) = (yyvsp[-3].funcfparams); 
                                              (yyval.funcfparams)->push_back(new Variable((yyvsp[-1].btype), (yyvsp[0].string), false));
                                              delete (yyvsp[0].string);
                                            }
#line 2042 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 79:
#line 283 "src/analyzer.y"
                                                      { (yyval.funcfparams) = (yyvsp[-4].funcfparams);
                                                        (yyval.funcfparams)->push_back(new Array((yyvsp[-2].btype),  (yyvsp[-1].string), false, (yyvsp[0].explist)));
                                                        delete (yyvsp[-1].string);
                                                      }
#line 2051 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 80:
#line 289 "src/analyzer.y"
                                                             { (yyval.funcdecl) = new FunctionDecl((yyvsp[-5].btype), (yyvsp[-4].string), (yyvsp[-2].funcfparams), (yyvsp[0].blockstmt)); delete (yyvsp[-4].string); }
#line 2057 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;

  case 81:
#line 290 "src/analyzer.y"
                                                         { (yyval.funcdecl) = new FunctionDecl((yyvsp[-4].btype), (yyvsp[-3].string), nullptr, (yyvsp[0].blockstmt)); delete (yyvsp[-3].string); }
#line 2063 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"
    break;


#line 2067 "/mnt/f/C++/Compiler/compile-a/cmake-build-debug/analyzer.cc"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 292 "src/analyzer.y"


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
    if ((*num)=='X' || (*num)=='x') return parse_hex(num+1);
    else                            return parse_oct(num);
  }
  return parse_dec(num);
}
