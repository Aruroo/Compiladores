%code requires {
    #include <vector>
    #include "ast.h"
}

%{
#include <stdio.h>
#include "scanner.h"
extern int yylex(void);
extern int yylineno;
extern char *yytext;
void yyerror(const char *msg);

Nodo* raiz = nullptr;  
%}

%locations
%define parse.error custom
%define parse.lac full

%union {
    int    ival;
    double dval;
    char   cval;
    char*  sval;

    Nodo*  nodo;
    Tipo  tipo;

    std::vector<Nodo*>* lista;
}

%token <ival> TOK_ENTERO_LITERAL "entero literal"
%token <dval> TOK_FLOTANTE_LITERAL "flotante literal"
%token <cval> TOK_LETRA_LITERAL "letra literal"
%token <sval> TOK_TEXTO_LITERAL "texto literal"
%token <sval> TOK_ALIAS "alias"

%token TOK_KW_HOLA "hola"
%token TOK_KW_ATENTAMENTE "atentamente"
%token TOK_KW_QUERIDO "querido"
%token TOK_KW_MUESTRA "muestra"
%token TOK_KW_TEXTO "texto"
%token TOK_KW_LEE "lee"
%token TOK_KW_ENTERO "entero"
%token TOK_KW_FLOTANTE "flotante"
%token TOK_KW_LETRA "letra"
%token TOK_KW_NADA "nada"
%token TOK_KW_CUANDO "cuando"
%token TOK_KW_SINO "sino"
%token TOK_KW_MIENTRAS "mientras"
%token TOK_KW_DEVUELVE "devuelve"
%token TOK_KW_ROMPE "rompe"
%token TOK_KW_CONTINUA "continua"

%token TOK_ASSIGN  "="
%token TOK_EQ      "=="
%token TOK_NEQ     "!="
%token TOK_LT      "<"
%token TOK_LE      "<="
%token TOK_GT      ">"
%token TOK_GE      ">="
%token TOK_AND     "&&"
%token TOK_OR      "||"
%token TOK_NOT     "!"
%token TOK_PLUS    "+"
%token TOK_MINUS   "-"
%token TOK_MUL     "*"
%token TOK_DIV     "/"
%token TOK_MOD     "%"
%token TOK_LPAREN  "("
%token TOK_RPAREN  ")"
%token TOK_LBRACE  "{"
%token TOK_RBRACE  "}"
%token TOK_COMMA   ","

%type <nodo>  expresion termino factor
%type <nodo>  expresion_booleana termino_bool factor_bool
%type <nodo>  oracion oracion_cerrada oracion_abierta
%type <nodo>  funcion 
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
        {
            raiz = hacer_programa($3, $9, $5, yylineno); 
            free($3);
            free($9);
        }
    ;

parrafos
    : parrafos parrafo  { $1->push_back($2); $$ = $1; }
    |                   { $$ = new std::vector<Nodo*>(); }
    ;

parrafo
    : funcion   { $$ = $1; }
    | oracion   { $$ = $1; }
    ;

tipo
    : TOK_KW_ENTERO     { $$ = Tipo::ENTERO; }
    | TOK_KW_FLOTANTE   { $$ = Tipo::FLOTANTE; }
    | TOK_KW_LETRA      { $$ = Tipo::LETRA; }
    | TOK_KW_TEXTO      { $$ = Tipo::TEXTO; }
    | TOK_KW_NADA       { $$ = Tipo::NADA; }
    ;

funcion
    : TOK_KW_QUERIDO tipo TOK_ALIAS TOK_LPAREN params TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE
        {
            $$ = hacer_funcion($2, $3, $5, $8, yylineno);
            free($3);
        }
    ;

params
    : lista_params  { $$ = $1; }
    |               { $$ = new std::vector<Nodo*>(); }
    ;

lista_params
    : tipo TOK_ALIAS
        {
            $$ = new std::vector<Nodo*>();
            $$->push_back(hacer_param($1, $2, yylineno));
            free($2);
        }
    | lista_params TOK_COMMA tipo TOK_ALIAS
        {
            $1->push_back(hacer_param($3, $4, yylineno));
            free($4);
            $$ = $1;
        }
    ;

oraciones
    : oraciones oracion { $1->push_back($2); $$ = $1; }
    |                   { $$ = new std::vector<Nodo*>(); }
    ;

oracion
    : oracion_abierta   { $$ = $1; }
    | oracion_cerrada   { $$ = $1; }
    ;

oracion_cerrada
    : TOK_KW_CUANDO TOK_LPAREN expresion_booleana TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE TOK_KW_SINO TOK_LBRACE oraciones TOK_RBRACE
        { $$ = hacer_cuando($3, $6, $10, yylineno); }
    | TOK_KW_MIENTRAS TOK_LPAREN expresion_booleana TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE
        { $$ = hacer_mientras($3, $6, yylineno); }
    | TOK_KW_DEVUELVE expresion
        { $$ = hacer_devuelve($2, yylineno); }
    | TOK_KW_ROMPE
        { $$ = hacer_rompe(yylineno); }
    | TOK_KW_CONTINUA
        { $$ = hacer_continua(yylineno); }
    | TOK_KW_MUESTRA TOK_LPAREN expresion TOK_RPAREN
        { $$ = hacer_muestra($3, yylineno); }
    | TOK_KW_LEE TOK_LPAREN TOK_ALIAS TOK_RPAREN
        { $$ = hacer_lee($3, yylineno); free($3); }
    | tipo TOK_ALIAS TOK_ASSIGN expresion
        { $$ = hacer_decl($1, $2, $4, yylineno); free($2); }
    | tipo TOK_ALIAS
        { $$ = hacer_decl($1, $2, nullptr, yylineno); free($2); }
    | TOK_ALIAS TOK_ASSIGN expresion
        { $$ = hacer_asignacion($1, $3, yylineno); free($1); }
    | TOK_ALIAS TOK_LPAREN args TOK_RPAREN
        { $$ = hacer_llamada($1, $3, yylineno); free($1); }
    ;

oracion_abierta
    : TOK_KW_CUANDO TOK_LPAREN expresion_booleana TOK_RPAREN TOK_LBRACE oraciones TOK_RBRACE
        { $$ = hacer_cuando($3, $6, new std::vector<Nodo*>(), yylineno); }
    ;

expresion
    : expresion TOK_PLUS termino
        { $$ = hacer_binop(OpBinaria::SUMA,  $1, $3, yylineno); }
    | expresion TOK_MINUS termino
        { $$ = hacer_binop(OpBinaria::RESTA, $1, $3, yylineno); }
    | termino
        { $$ = $1; }
    ;

termino
    : termino TOK_MUL factor
        { $$ = hacer_binop(OpBinaria::MUL, $1, $3, yylineno); }
    | termino TOK_DIV factor
        { $$ = hacer_binop(OpBinaria::DIV, $1, $3, yylineno); }
    | termino TOK_MOD factor
        { $$ = hacer_binop(OpBinaria::MOD, $1, $3, yylineno); }
    | factor
        { $$ = $1; }
    ;

factor
    : TOK_LPAREN expresion TOK_RPAREN
        { $$ = $2; }
    | TOK_MINUS factor
        { $$ = hacer_unop(OpUnaria::NEGACION, $2, yylineno); }
    | TOK_ALIAS
        { $$ = hacer_alias($1, yylineno); free($1); }
    | TOK_ALIAS TOK_LPAREN args TOK_RPAREN
        { $$ = hacer_llamada($1, $3, yylineno); free($1); }
    | TOK_ENTERO_LITERAL
        { $$ = hacer_entero($1, yylineno); }
    | TOK_FLOTANTE_LITERAL
        { $$ = hacer_flotante($1, yylineno); }
    | TOK_TEXTO_LITERAL
        { $$ = hacer_texto($1, yylineno); free($1); }
    | TOK_LETRA_LITERAL
        { $$ = hacer_letra($1, yylineno); }
    ;

expresion_booleana
    : expresion_booleana TOK_OR termino_bool
        { $$ = hacer_binop(OpBinaria::OR,  $1, $3, yylineno); }
    | termino_bool
        { $$ = $1; }
    ;

termino_bool
    : termino_bool TOK_AND factor_bool
        { $$ = hacer_binop(OpBinaria::AND, $1, $3, yylineno); }
    | factor_bool
        { $$ = $1; }
    ;

factor_bool
    : TOK_NOT factor_bool
        { $$ = hacer_unop(OpUnaria::NOT, $2, yylineno); }
    | expresion TOK_EQ expresion
        { $$ = hacer_binop(OpBinaria::EQ,  $1, $3, yylineno); }
    | expresion TOK_NEQ expresion
        { $$ = hacer_binop(OpBinaria::NEQ, $1, $3, yylineno); }
    | expresion TOK_LT expresion
        { $$ = hacer_binop(OpBinaria::LT,  $1, $3, yylineno); }
    | expresion TOK_LE expresion
        { $$ = hacer_binop(OpBinaria::LE,  $1, $3, yylineno); }
    | expresion TOK_GT expresion
        { $$ = hacer_binop(OpBinaria::GT,  $1, $3, yylineno); }
    | expresion TOK_GE expresion
        { $$ = hacer_binop(OpBinaria::GE,  $1, $3, yylineno); }
    | expresion
        { $$ = $1; }
    ;

args
    : lista_args    { $$ = $1; }
    |               { $$ = new std::vector<Nodo*>(); }
    ;

lista_args
    : expresion
        {
            $$ = new std::vector<Nodo*>();
            $$->push_back($1);
        }
    | lista_args TOK_COMMA expresion
        { $1->push_back($3); $$ = $1; }
    ;

%%

static int yyreport_syntax_error(const yypcontext_t *ctx) {
    yysymbol_kind_t lookahead = yypcontext_token(ctx);

    enum { MAX_ESPERADOS = 32 };
    yysymbol_kind_t esperados[MAX_ESPERADOS];
    int n = yypcontext_expected_tokens(ctx, esperados, MAX_ESPERADOS);
    if (n < 0) return n;   /* error interno de Bison */

    const YYLTYPE *loc = yypcontext_location(ctx);
    int line = (loc && loc->first_line > 0) ? loc->first_line : yylineno;

    fprintf(stderr, "syntax error: ");

    if (n == 1) {
        fprintf(stderr, "expected %s", yysymbol_name(esperados[0]));
    } else if (n > 1) {
        fprintf(stderr, "expected one of ");
        for (int i = 0; i < n; ++i) {
            fprintf(stderr, "%s%s",
                    yysymbol_name(esperados[i]),
                    (i + 1 < n) ? ", " : "");
        }
    } else {
        fprintf(stderr, "unexpected token");
    }

    if (lookahead != YYSYMBOL_YYEMPTY) {
        fprintf(stderr, " but got %s", yysymbol_name(lookahead));
    }

    fprintf(stderr, " at line %d\n", line);
    return 0;
}
