%option noyywrap yylineno

%{
	#include "parser.tab.h"
	#include "scanner.h"
%}

%x COMENTARIO

ALIAS [a-zA-Z_][a-zA-Z0-9_]*
ENTERO [0-9]+
FLOTANTE ({ENTERO}\.{ENTERO})
RELLENO [ \t\r\n]+
%%

"/*" {BEGIN(COMENTARIO);}
<COMENTARIO>"*/" {BEGIN(INITIAL);}
<COMENTARIO>\n {}
<COMENTARIO>. {}
<COMENTARIO><<EOF>> { return yytext[0]; }
"//"[^\n]* {}

{RELLENO} {}

"hola" {return TOK_KW_HOLA;}
"atentamente" {return TOK_KW_ATENTAMENTE;}
"entero" {return TOK_KW_ENTERO;}
"flotante" {return TOK_KW_FLOTANTE;}
"letra" {return TOK_KW_LETRA;}
"nada" {return TOK_KW_NADA;}
"cuando" {return TOK_KW_CUANDO;}
"sino" {return TOK_KW_SINO;}
"mientras" {return TOK_KW_MIENTRAS;}
"devuelve" {return TOK_KW_DEVUELVE;}
"rompe" {return TOK_KW_ROMPE;}
"continua" {return TOK_KW_CONTINUA;}
"querido" {return TOK_KW_QUERIDO;}
"muestra" {return TOK_KW_MUESTRA;}
"texto" {return TOK_KW_TEXTO;}
"lee" {return TOK_KW_LEE;}

"==" {return TOK_EQ;}
"!=" {return TOK_NEQ;}
"<=" {return TOK_LE;}
">=" {return TOK_GE;}
"&&" {return TOK_AND;}
"||" {return TOK_OR;}

"=" {return TOK_ASSIGN;}
"<" {return TOK_LT;}
">" {return TOK_GT;}
"!" {return TOK_NOT;}
"+" {return TOK_PLUS;}
"-" {return TOK_MINUS;}
"*" {return TOK_MUL;}
"/" {return TOK_DIV;}
"%" {return TOK_MOD;}

"(" {return TOK_LPAREN;} 
")" {return TOK_RPAREN;}
"{" {return TOK_LBRACE;}
"}" {return TOK_RBRACE;}
"," {return TOK_COMMA;}

{FLOTANTE}              { yylval.dval = atof(yytext); return TOK_FLOTANTE_LITERAL; }
{ENTERO}                { yylval.ival = atoi(yytext); return TOK_ENTERO_LITERAL; }
'([^'\\\n]|\\.)'        { yylval.cval = yytext[1];   return TOK_LETRA_LITERAL; }
\"([^\"\\\n]|\\.)*\"    { yylval.sval = strdup(yytext); return TOK_TEXTO_LITERAL; }
{ALIAS}                 { yylval.sval = strdup(yytext); return TOK_ALIAS; }

<<EOF>>  { return 0; }
.        { return yytext[0]; }
%%
