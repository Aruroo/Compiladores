#include <stdio.h>
#include "AST.h"

extern int yyparse(void);
extern NodoPrograma *raiz;

int main(void) {
    if (yyparse() == 0) {
        printf("AST generated successfully. AST:\n");
        raiz->imprimir(0);
    } else {
        // in porduction code, we would want to print the error with line number and details, but for this test, a simple message is enough
        fprintf(stderr, "Error parsing the program.\n");
        return 1;
}
