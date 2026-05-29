#include <stdio.h>
#include <string>
#include "ast.h"
#include "analyzer.h"
#include "codegen.h"

extern int yyparse(void);
extern FILE* yyin;
extern Nodo* raiz;

static std::string nombre_base(const std::string& ruta) {
    size_t barra = ruta.find_last_of("/\\");
    std::string archivo = (barra == std::string::npos) ? ruta : ruta.substr(barra + 1);
    size_t punto = archivo.find_last_of('.');
    return (punto == std::string::npos) ? archivo : archivo.substr(0, punto);
}

int main(int argc, char** argv) {
    std::string base = "salida";

    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) {
            fprintf(stderr, "No se pudo abrir '%s'\n", argv[1]);
            return 1;
        }
        base = nombre_base(argv[1]);
    }

    if (yyparse() != 0) {
        fprintf(stderr, "Error parsing the program.\n");
        return 1;
    }
    if (!raiz) {
        fprintf(stderr, "Error: Null AST.\n");
        return 1;
    }

    // semantic analysis and printing the AST
    visitar(raiz);
    raiz->imprimir(0);
    imprimir_tabla();
    imprimir_promociones();

    // if there were semantic errors, print them and exit without generating code
    if (hubo_errores()) {
        printf("\n");
        imprimir_errores();
        return 1;
    }

    // code generation
    std::string ruta_salida = "../outputs/" + base + ".txt";
    FILE* salida = fopen(ruta_salida.c_str(), "w");
    if (!salida) {
        fprintf(stderr, "No se pudo abrir '%s' para escritura.\n", ruta_salida.c_str());
        return 1;
    }
    generar_codigo(raiz, salida);

    fclose(salida);
    printf("\nCodigo intermedio FIS-25 generado en '%s'\n", ruta_salida.c_str());
    return 0;
}