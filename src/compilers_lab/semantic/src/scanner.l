%option noyywrap yylineno

%{
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
<COMENTARIO><<EOF>> {return TOK_ERROR;}
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

"++" {return TOK_INC;}
"--" {return TOK_DEC;}
"+=" {return TOK_PLUS_ASSIGN;}
"-=" {return TOK_MINUS_ASSIGN;}
"*=" {return TOK_MUL_ASSIGN;}
"/=" {return TOK_DIV_ASSIGN;}
"%=" {return TOK_MOD_ASSIGN;}
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
"[" {return TOK_LBRACKET;}
"]" {return TOK_RBRACKET;}
"," {return TOK_COMMA;}
";" {return TOK_SEMICOLON;}

{FLOTANTE} {return TOK_FLOTANTE_LITERAL;}
{ENTERO} {return TOK_ENTERO_LITERAL;}
'([^'\\\n]|\\.)' {return TOK_LETRA_LITERAL;}
\"([^\"\\\n]|\\.)*\" {return TOK_TEXTO_LITERAL;}
{ALIAS} {return TOK_ALIAS;}

<<EOF>> {return TOK_EOF;}
. {return TOK_ERROR;}
%%
