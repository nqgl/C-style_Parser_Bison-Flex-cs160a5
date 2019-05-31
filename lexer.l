%option yylineno
%pointer

%{
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <limits>
#include "ast.hpp"
#include "parser.hpp"

void yyerror(const char *);
%}

/* WRITEME: Copy any definitions and start conditions from Project 4 here. */
%x COMMENT
digit       [0-9]
identifier ([a-zA-Z][a-zA-Z0-9_]*)
%%

((0|[1-9]{digit}*))   {long int value = strtol(yytext,NULL,0); yylval.integerliteral_ptr =new IntegerLiteralNode (new IntegerNode(value));return T_LITERAL;}
"new"         {return T_NEW;}
"integer"     {return T_INTEGER;}
"return"  {return T_RETURN;}
"extends" {return T_EXTENDS;}
"print"   {return T_PRINT;}
"false"     {yylval.booleanliteral_ptr =new BooleanLiteralNode(new IntegerNode(0));return T_FALSE;}
"while" {return T_WHILE;}
"else" {return T_ELSE;}
"true"  {yylval.booleanliteral_ptr =new BooleanLiteralNode(new IntegerNode(1));return T_TRUE;}
"none"   {return T_NONE;}
"equals"  {return T_EQUALS;}
"boolean"    {return T_BOOLEAN;}
"and" {return T_AND;}
"if"  {return T_IF;}
">="  {return T_GTET;}
"do"  {return T_DO;}
"or"  {return T_OR;}
"->"    {return T_ARROW;}


"not"  {return T_NOT;}
{identifier}       { std::string x= (yytext); yylval.identifier_ptr =new IdentifierNode(x);return T_ID;}
[\t ] ;
\n     ;
"/*" { BEGIN(COMMENT); }
<COMMENT>.
<COMMENT>\n ;
<COMMENT><<EOF>> { yyerror("dangling comment"); }
<COMMENT>"*/" { BEGIN(INITIAL); }


[=\.\;\,\(\)\{\}\*\/\-\+\>] { return *yytext; }
.                 { yyerror("invalid character"); }

%%

int yywrap(void) {
  return 1;
}
