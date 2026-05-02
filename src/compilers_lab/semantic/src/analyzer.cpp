#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include "ast.h"

extern int yyparse(void);
extern Nodo* raiz;


enum class Categoria { VARIABLE, PARAMETRO, FUNCION };

struct Simbolo {
    std::string nombre;
    Categoria   categoria;
    Tipo        tipo;
    int         scope_nivel;
    int         linea; // line where it was declared (for error reporting)
    std::vector<Tipo> tipos_parametros; // only used if categoria == FUNCION
};

using TablaNivel = std::map<std::string, Simbolo>;
std::vector<TablaNivel> pila_scopes;
std::vector<Simbolo> todos_los_simbolos;

// Representation of a semantic error
struct ErrorSemantico {
    std::string mensaje; // error message description
    int         linea;   // line where the error occurred
};

std::vector<ErrorSemantico> errores_semanticos; // global list of semantic errors (errors are collected during the AST traversal)

// helper function to report an error
void reportar_error(const std::string& msg, int linea) {
    errores_semanticos.push_back({msg, linea});
}

// operations on the stack

void abrir_scope() {
    pila_scopes.push_back({});
}

void cerrar_scope() {
    pila_scopes.pop_back();
}

// declares on the top scope
void declarar(const Simbolo& s) {
    auto& scope_actual = pila_scopes.back();
    if (scope_actual.count(s.nombre)) {
        reportar_error(
            "'" + s.nombre + "' was already declared in this scope",
            s.linea);
        return;
    }
    scope_actual[s.nombre] = s;
    todos_los_simbolos.push_back(s);
}

// searches from the top to the bottom
const Simbolo* buscar(const std::string& nombre) {
    for (auto it = pila_scopes.rbegin(); it != pila_scopes.rend(); ++it) {
        auto found = it->find(nombre);
        if (found != it->end())
            return &found->second;
    }
    return nullptr;
}

void visitar(Nodo* nodo);

void visitar_bloque(const std::vector<NodoPtr>& stmts) {
    for (const auto& s : stmts)
        if (s) visitar(s.get());
}

void visitar(Nodo* nodo) {
    if (!nodo) return;

    // literals and leafs

    if (dynamic_cast<NodoEntero*>(nodo)   ||
        dynamic_cast<NodoFlotante*>(nodo) ||
        dynamic_cast<NodoLetra*>(nodo)    ||
        dynamic_cast<NodoTexto*>(nodo)    ||
        dynamic_cast<NodoRompe*>(nodo)    ||
        dynamic_cast<NodoContinua*>(nodo))
        return;

    // var

    if (auto* n = dynamic_cast<NodoAlias*>(nodo)) {
        if (!buscar(n->nombre))
            reportar_error(
                "'" + n->nombre + "' is not initialized",
                n->linea);
        return;
    }

    //  var read

    if (auto* n = dynamic_cast<NodoLee*>(nodo)) {
        if (!buscar(n->nombre))
            reportar_error(
                "'" + n->nombre + "' is not initialized",
                n->linea);
        return;
    }

    // declaration

    if (auto* n = dynamic_cast<NodoDecl*>(nodo)) {
        Simbolo s;
        s.nombre = n->nombre;
        s.categoria = Categoria::VARIABLE;
        s.tipo = n->tipo;
        s.scope_nivel = (int)pila_scopes.size() - 1;
        s.linea = n->linea;
        declarar(s);

        if (n->valor) visitar(n->valor.get());
        return;
    }

    // assign

    if (auto* n = dynamic_cast<NodoAsignacion*>(nodo)) {
        if (!buscar(n->nombre))
            reportar_error(
                "'" + n->nombre + "' is not initialized",
                n->linea);
        if (n->valor) visitar(n->valor.get());
        return;
    }

    // exp

    if (auto* n = dynamic_cast<NodoBinop*>(nodo)) {
        visitar(n->izq.get());
        visitar(n->der.get());
        return;
    }

    if (auto* n = dynamic_cast<NodoUnop*>(nodo)) {
        visitar(n->hijo.get());
        return;
    }

    //function call

    if (auto* n = dynamic_cast<NodoLlamada*>(nodo)) {
        if (!buscar(n->nombre))
            reportar_error(
                "'" + n->nombre + "' is not initialized",
                n->linea);
        for (const auto& a : n->args)
            if (a) visitar(a.get());
        return;
    }

    // muestra - devuelve

    if (auto* n = dynamic_cast<NodoMuestra*>(nodo)) {
        visitar(n->valor.get());
        return;
    }

    if (auto* n = dynamic_cast<NodoDevuelve*>(nodo)) {
        if (n->valor) visitar(n->valor.get());
        return;
    }

    // cuando

    if (auto* n = dynamic_cast<NodoCuando*>(nodo)) {
        visitar(n->condicion.get());

        abrir_scope();
        visitar_bloque(n->entonces);
        cerrar_scope();

        if (!n->sino.empty()) {
            abrir_scope();
            visitar_bloque(n->sino);
            cerrar_scope();
        }
        return;
    }

    // mientras

    if (auto* n = dynamic_cast<NodoMientras*>(nodo)) {
        visitar(n->condicion.get());
        abrir_scope();
        visitar_bloque(n->cuerpo);
        cerrar_scope();
        return;
    }

    // function

    if (auto* n = dynamic_cast<NodoFuncion*>(nodo)) {
        Simbolo s;
        s.nombre = n->nombre;
        s.categoria = Categoria::FUNCION;
        s.tipo = n->tipo_retorno;
        s.scope_nivel = (int)pila_scopes.size() - 1;
        s.linea = n->linea;

        // collect parameter types for later use (e.g. type checking on calls)
        for (const auto& p : n->params) {
            if (auto* param = dynamic_cast<NodoParam*>(p.get()))
                s.tipos_parametros.push_back(param->tipo);
        }

        // declared on father scope
        declarar(s);

        abrir_scope(); // own scope

        for (const auto& p : n->params) {
            if (auto* param = dynamic_cast<NodoParam*>(p.get())){
                Simbolo sp;
                sp.nombre = param->nombre;
                sp.categoria = Categoria::PARAMETRO;
                sp.tipo = param->tipo;
                sp.scope_nivel = (int)pila_scopes.size() - 1;
                sp.linea = param->linea;
                declarar(sp); // declare parameters on own scope
            }
        }

        visitar_bloque(n->cuerpo);
        cerrar_scope();
        return;
    }

    // program

    if (auto* n = dynamic_cast<NodoPrograma*>(nodo)) {
        abrir_scope(); // global scope
        visitar_bloque(n->parrafos);
        cerrar_scope();
        return;
    }
}

void imprimir_tabla() {
    printf("\n%-20s %-12s %-10s %s\n", "NOMBRE", "CATEGORIA", "TIPO", "SCOPE");
    printf("%s\n", std::string(55, '-').c_str());
    for (const auto& s : todos_los_simbolos) {
        std::string cat =
            s.categoria == Categoria::VARIABLE  ? "variable"  :
            s.categoria == Categoria::PARAMETRO ? "parametro" : "funcion";
        printf("%-20s %-12s %-10s %d\n",
            s.nombre.c_str(), cat.c_str(),
            tipo_str(s.tipo).c_str(), s.scope_nivel);
    }
}

// prints all collected semantic errors
void imprimir_errores() {
    for (const auto& e : errores_semanticos) {
        fprintf(stderr, "semantic error: %s at line %d\n",
                e.mensaje.c_str(), e.linea);
    }
}

int main(void) {
    if (yyparse() != 0) {
        fprintf(stderr, "Error parsing the program.\n");
        return 1;
    }
    if (!raiz) {
        fprintf(stderr, "Error: Null AST.\n");
        return 1;
    }

    raiz->imprimir(0);
    visitar(raiz);
    imprimir_tabla();

    if (!errores_semanticos.empty()) {
        printf("\n");
        imprimir_errores();
        return 1;
    }

    return 0;
}

void yyerror(const char* msg) {
    extern int yylineno;
    reportar_error(msg, yylineno);
}