%code requires {
    #include <vector>
    #include "ast.h"
}

%{
#include "scanner.h"
extern int yylex(void);
extern int yylineno;
void yyerror(const char *msg);
%}

%union {
    int    ival;
    double dval;
    char   cval;
    char*  sval;

    Nodo*  nodo;
    Tipo  tipo;

    std::vector<Nodo*>* lista;
}

%token <ival> TOK_ENTERO_LITERAL
%token <dval> TOK_FLOTANTE_LITERAL
%token <cval> TOK_LETRA_LITERAL
%token <sval> TOK_TEXTO_LITERAL TOK_ALIAS

%token TOK_KW_HOLA TOK_KW_ATENTAMENTE TOK_KW_QUERIDO
%token TOK_KW_MUESTRA TOK_KW_TEXTO TOK_KW_LEE
%token TOK_KW_ENTERO TOK_KW_FLOTANTE TOK_KW_LETRA TOK_KW_NADA
%token TOK_KW_CUANDO TOK_KW_SINO TOK_KW_MIENTRAS
%token TOK_KW_DEVUELVE TOK_KW_ROMPE TOK_KW_CONTINUA
%token TOK_ASSIGN
%token TOK_EQ TOK_NEQ TOK_LT TOK_LE TOK_GT TOK_GE
%token TOK_AND TOK_OR TOK_NOT
%token TOK_PLUS TOK_MINUS TOK_MUL TOK_DIV TOK_MOD
%token TOK_LPAREN TOK_RPAREN TOK_LBRACE TOK_RBRACE TOK_COMMA

%type <nodo>  expresion termino factor
%type <nodo>  expresion_booleana termino_bool factor_bool
%type <nodo>  oracion oracion_cerrada oracion_abierta
%type <nodo>  funcion programa
%type <nodo>  parrafo
%type <tipo>  tipo
%type <lista> oraciones parrafos
%type <lista> params lista_params
%type <lista> args lista_args

%destructor { delete $$; } <nodo>
%destructor { delete $$; } <lista>

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