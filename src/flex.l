%option noyywrap
%{
#include "ast.h"
#include "analyzer.tab.hh"

#include <string>

%}

%%

"int"       { return INT;}
[0-9]*      { yylval.string = new std::string(yytext); return NUMBER; }
"0x"[0-9A-Fa-f]+  { yylval.string = new std::string(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]*      { yylval.string = (new std::string(yytext)); return IDENT;}
"="           { yylval.token = ASSIGN; return ASSIGN; }
"+"           { yylval.token = ADD; return ADD; }
"-"           { yylval.token = SUB; return SUB; }
"*"           { yylval.token = MUL; return MUL; }
"/"           { yylval.token = DIV; return DIV; }
"%"           { yylval.token = MOD; return MOD; }
";"           { yylval.token = SEMI; return SEMI; }
","           { yylval.token = COMMA; return COMMA; }
"{"           { yylval.token = LCURLY; return LCURLY; }
"}"           { yylval.token = RCURLY; return RCURLY; }
"("           { yylval.token = LPARENT; return LPARENT; }
")"           { yylval.token = RPARENT; return RPARENT; }
"["           { yylval.token = LBRACKET; return LBRACKET; }
"]"           { yylval.token = RBRACKET; return RBRACKET; }
[ \t\n]       {}
.             {}
%%
