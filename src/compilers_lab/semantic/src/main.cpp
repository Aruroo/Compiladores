#include <stdio.h>
#include "ast.h"

extern int yyparse(void);
extern Nodo* raiz;

int main(void) {
    if (yyparse() == 0) {
        printf("AST generated successfully. AST:\n");
        raiz->imprimir(0);
    } else {
        fprintf(stderr, "Error parsing the program.\n");
        return 1;
    }
    return 0;
}

void yyerror(const char *msg) {
    extern int yylineno;
    extern char *yytext;
    fprintf(stderr, "syntax error: %s near '%s' at line %d\n",
            msg, yytext, yylineno);
}