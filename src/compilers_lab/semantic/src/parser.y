%{
#include <stdio.h>
#include "scanner.h"

extern int yylex(void);
extern int yylineno;
void yyerror(const char *msg);
%}

%token TOK_KW_HOLA
%token TOK_KW_ATENTAMENTE
%token TOK_KW_QUERIDO
%token TOK_KW_MUESTRA
%token TOK_KW_TEXTO
%token TOK_KW_LEE
%token TOK_KW_ENTERO
%token TOK_KW_FLOTANTE
%token TOK_KW_LETRA
%token TOK_KW_NADA
%token TOK_KW_CUANDO
%token TOK_KW_SINO
%token TOK_KW_MIENTRAS
%token TOK_KW_DEVUELVE
%token TOK_KW_ROMPE
%token TOK_KW_CONTINUA
%token TOK_ALIAS
%token TOK_ENTERO_LITERAL
%token TOK_FLOTANTE_LITERAL
%token TOK_TEXTO_LITERAL
%token TOK_LETRA_LITERAL
%token TOK_ASSIGN
%token TOK_EQ TOK_NEQ
%token TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_AND TOK_OR TOK_NOT
%token TOK_PLUS TOK_MINUS TOK_MUL TOK_DIV TOK_MOD
%token TOK_LPAREN TOK_RPAREN
%token TOK_LBRACE TOK_RBRACE
%token TOK_COMMA

%%

programa
    : TOK_KW_HOLA TOK_COMMA TOK_ALIAS TOK_LBRACE parrafos TOK_RBRACE TOK_KW_ATENTAMENTE TOK_COMMA TOK_ALIAS
    ;

parrafos
    : parrafo parrafos
    |
    ;

parrafo
    : funcion
    | oracion
    ;

tipo
    : TOK_KW_ENTERO
    | TOK_KW_FLOTANTE
    | TOK_KW_LETRA
    | TOK_KW_TEXTO
    | TOK_KW_NADA
    ;

funcion
    : TOK_KW_QUERIDO tipo TOK_ALIAS TOK_LPAREN params TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE
    ;

params
    : lista_params
    |
    ;

lista_params
    : tipo TOK_ALIAS
    | lista_params TOK_COMMA tipo TOK_ALIAS
    ;

argumento
    : tipo TOK_ALIAS
    ;

args
    : lista_args
    |
    ;

lista_args
    : expresion
    | lista_args TOK_COMMA expresion
    ;

%%

void yyerror(const char *msg) {
    fprintf(stderr, "syntax error: %s at line %d\n", msg, yylineno);
}