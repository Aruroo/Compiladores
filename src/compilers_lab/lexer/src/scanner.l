%option noyywrap yylineno

%{
	#include "scanner.h"
%}

%x COMMENT

ID [a-zA-Z_][a-zA-Z0-9_]*
INT [0-9]+
FLOAT ({INT}\.{INT})|({INT}\.{INT}[eE][+-]?{INT})|({INT}[eE][+-]?{INT})
WS [ \t\r\n]+
%%

"/*" {BEGIN(COMMENT);}
<COMMENT>"*/" {BEGIN(INITIAL);}
<COMMENT>\n {}
<COMMENT>. {}
<COMMENT><<EOF>> {return TOK_ERROR;}
"//"[^\n]* {}

{WS} {}

"int" {return TOK_KW_INT;}
"float" {return TOK_KW_FLOAT;}
"double" {return TOK_KW_DOUBLE;}
"char" {return TOK_KW_CHAR;}
"void" {return TOK_KW_VOID;}
"if" {return TOK_KW_IF;}
"else" {return TOK_KW_ELSE;}
"while" {return TOK_KW_WHILE;}
"for" {return TOK_KW_FOR;}
"return" {return TOK_KW_RETURN;}
"break" {return TOK_KW_BREAK;}
"continue" {return TOK_KW_CONTINUE;}

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

{FLOAT} {return TOK_FLOAT_LITERAL;}
{INT} {return TOK_INT_LITERAL;}
'([^'\\\n]|\\.)' {return TOK_CHAR_LITERAL;}
\"([^\"\\\n]|\\.)*\" {return TOK_STRING_LITERAL;}
{ID} {return TOK_IDENTIFIER;}


<<EOF>> {return TOK_EOF;}

/*
SECCIÓN 11: Delimitadores
- (, ), {, }, [, ], ,, ;
- Devuelven los TOK_* correspondientes.
*/

/*
SECCIÓN 12: Literales e identificadores (orden importante)
- Primero FLOAT -> TOK_FLOAT_LITERAL.
- Luego INT -> TOK_INT_LITERAL.
- Luego literal de char (admitiendo escapes) -> TOK_CHAR_LITERAL.
- Luego literal de string (admitiendo escapes) -> TOK_STRING_LITERAL.
- Finalmente ID -> TOK_IDENTIFIER.

Nota: si pones ID antes de keywords, las keywords se reconocerán como identificadores.
*/

/*
SECCIÓN 13: Fin de archivo y error genérico
- Regla <<EOF>> que retorna TOK_EOF.
- Regla final con . que retorna TOK_ERROR para cualquier carácter no reconocido.
*/

/*
SECCIÓN 14: Cierre de reglas
- Coloca el separador %% final.
*/

. {return TOK_ERROR;}
%%