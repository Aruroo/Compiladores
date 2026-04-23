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


oraciones
    : oracion oraciones
    |
    ;

oracion
    : oracion_abierta
    | oracion_cerrada
    ;

oracion_cerrada
    : TOK_KW_CUANDO TOK_LPAREN expresion_booleana TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE TOK_KW_SINO TOK_LBRACE oraciones TOK_RBRACE
    | TOK_KW_MIENTRAS TOK_LPAREN expresion_booleana TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE
    | TOK_KW_DEVUELVE expresion
    | TOK_KW_ROMPE
    | TOK_KW_CONTINUA
    | TOK_KW_MUESTRA TOK_LPAREN expresion TOK_RPAREN
    | TOK_KW_LEE TOK_LPAREN TOK_ALIAS TOK_RPAREN
    | tipo TOK_ALIAS TOK_ASSIGN expresion
    | tipo TOK_ALIAS
    | TOK_ALIAS TOK_ASSIGN expresion
    | TOK_ALIAS TOK_LPAREN args TOK_RPAREN
    ;

oracion_abierta
    : TOK_KW_CUANDO TOK_LPAREN expresion_booleana TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE
    ;

expresion
    : expresion TOK_PLUS termino
    | expresion TOK_MINUS termino
    | termino
    ;

termino
    : termino TOK_MUL factor
    | termino TOK_DIV factor 
    | termino TOK_MOD factor
    | factor
    ;

factor
    : TOK_LPAREN expresion TOK_RPAREN
    | TOK_MINUS factor
    | TOK_ALIAS
    | TOK_ALIAS TOK_LPAREN args TOK_RPAREN
    | TOK_ENTERO_LITERAL
    | TOK_FLOTANTE_LITERAL
    | TOK_TEXTO_LITERAL
    | TOK_LETRA_LITERAL
    ;

expresion_booleana
    : expresion_booleana TOK_OR termino_bool
    | termino_bool
    ;

termino_bool
    : termino_bool TOK_AND factor_bool
    | factor_bool
    ;

factor_bool
    : TOK_NOT factor_bool
    | expresion TOK_EQ expresion
    | expresion TOK_NEQ expresion
    | expresion TOK_LT expresion
    | expresion TOK_LE expresion
    | expresion TOK_GT expresion
    | expresion TOK_GE expresion
    | TOK_LPAREN expresion_booleana TOK_RPAREN
    | expresion /* to use 0, 1 or var*/
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