#include <stdio.h>
#include "ast.h"

extern int yyparse(void);
extern Nodo* raiz;

int main(void) {
    if (yyparse() == 0) {
        // DEBUG: yyparse ok
        if (raiz == nullptr) {
            // DEBUG: raiz es nullptr
            fprintf(stderr, "Error: Null AST.\n");
            return 1;
        }
        // DEBUG: raiz ok, imprimiendo...
        printf("AST generated successfully: \n\n");
        raiz->imprimir(0);
    } else {
        fprintf(stderr, "Error parsing the program.\n");
        return 1;
    }
    return 0;
}

/* Custom error reporting function for Bison */
void yyerror(const char *msg) {
    extern int yylineno; // Bison provides the current line number in yylineno
    fprintf(stderr, "error: %s at line %d\n", msg, yylineno);
}