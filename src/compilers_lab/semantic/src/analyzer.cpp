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
};

using TablaNivel = std::map<std::string, Simbolo>;
std::vector<TablaNivel> pila_scopes;
std::vector<Simbolo> todos_los_simbolos;

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
        fprintf(stderr,
            "error: '%s' was already declared in (line %d)\n",
            s.nombre.c_str(), 0);
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
            fprintf(stderr, "error: '%s' not initialized [line %d]\n",
                n->nombre.c_str(), n->linea);
        return;
    }

    //  var read

    if (auto* n = dynamic_cast<NodoLee*>(nodo)) {
        if (!buscar(n->nombre))
            fprintf(stderr, "error: '%s' not initialized [linea %d]\n",
                n->nombre.c_str(), n->linea);
        return;
    }

    // declaration

    if (auto* n = dynamic_cast<NodoDecl*>(nodo)) {
        declarar({ n->nombre, Categoria::VARIABLE, n->tipo,
                   (int)pila_scopes.size() - 1 });
        if (n->valor) visitar(n->valor.get());
        return;
    }

    // assign

    if (auto* n = dynamic_cast<NodoAsignacion*>(nodo)) {
        if (!buscar(n->nombre))
            fprintf(stderr, "error: '%s' not initialized [linea %d]\n",
                n->nombre.c_str(), n->linea);
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
            fprintf(stderr, "error: '%s' not initialized [linea %d]\n",
                n->nombre.c_str(), n->linea);
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
        // declared on father scope
        declarar({ n->nombre, Categoria::FUNCION, n->tipo_retorno,
                   (int)pila_scopes.size() - 1 });

        abrir_scope(); // own scope

        for (const auto& p : n->params) {
            if (auto* param = dynamic_cast<NodoParam*>(p.get()))
                declarar({ param->nombre, Categoria::PARAMETRO, param->tipo,
                           (int)pila_scopes.size() - 1 });
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
    return 0;
}

void yyerror(const char* msg) {
    extern int yylineno;
    fprintf(stderr, "error: %s in line %d\n", msg, yylineno);
}