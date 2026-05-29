#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <sstream> // for error messages
#include "ast.h"
#include "analyzer.h"

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

Tipo tipo_retorno_esperado = Tipo::NADA; // used to check return statements are consistent with function declaration
bool dentro_de_funcion = false;
int profundidad_mientras = 0; 

// Representation of a type promotion (e.g. int promoted to float in an expression)
struct Promocion {
    Tipo origen;
    Tipo destino;
    int  linea;
};
std::vector<Promocion> promociones;

void registrar_promocion(Tipo origen, Tipo destino, int linea) {
    promociones.push_back({origen, destino, linea});
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

// type checking helpers

bool es_numerico(Tipo t) {
    return t == Tipo::ENTERO || t == Tipo::FLOTANTE;
}

bool es_asignable(Tipo destino, Tipo origen) {
    if (destino == Tipo::ERROR || origen == Tipo::ERROR) return true;
    if (destino == origen) return true;
    if (destino == Tipo::FLOTANTE && origen == Tipo::ENTERO) return true;
    return false;
}

bool es_promocion(Tipo destino, Tipo origen) {
    return destino == Tipo::FLOTANTE && origen == Tipo::ENTERO;
}

Tipo tipo_resultante(OpBinaria op, Tipo izq, Tipo der) {
    if (izq == Tipo::ERROR || der == Tipo::ERROR) return Tipo::ERROR;

    switch (op) {
        case OpBinaria::SUMA:
        case OpBinaria::RESTA:
        case OpBinaria::MUL:
        case OpBinaria::DIV:
            if (es_numerico(izq) && es_numerico(der)) {
                if (izq == Tipo::FLOTANTE || der == Tipo::FLOTANTE)
                    return Tipo::FLOTANTE;
                return Tipo::ENTERO;
            }
            return Tipo::ERROR;

        case OpBinaria::MOD:
            if (izq == Tipo::ENTERO && der == Tipo::ENTERO) return Tipo::ENTERO;
            return Tipo::ERROR;

        case OpBinaria::LT:
        case OpBinaria::LE:
        case OpBinaria::GT:
        case OpBinaria::GE:
            if (es_numerico(izq) && es_numerico(der)) return Tipo::BOOLEANO;
            return Tipo::ERROR;

        case OpBinaria::EQ:
        case OpBinaria::NEQ:
            if (es_numerico(izq) && es_numerico(der)) return Tipo::BOOLEANO;
            if (izq == der) return Tipo::BOOLEANO;
            return Tipo::ERROR;

        case OpBinaria::AND:
        case OpBinaria::OR:
            if (izq == Tipo::BOOLEANO && der == Tipo::BOOLEANO)
                return Tipo::BOOLEANO;
            return Tipo::ERROR;
    }
    return Tipo::ERROR;
}

Tipo tipo_resultante(OpUnaria op, Tipo hijo) {
    if (hijo == Tipo::ERROR) return Tipo::ERROR;
    switch (op) {
        case OpUnaria::NEGACION:
            return es_numerico(hijo) ? hijo : Tipo::ERROR;
        case OpUnaria::NOT:
            return (hijo == Tipo::BOOLEANO) ? Tipo::BOOLEANO : Tipo::ERROR;
    }
    return Tipo::ERROR;
}

void visitar(Nodo* nodo);

void visitar_bloque(const std::vector<NodoPtr>& stmts) {
    for (const auto& s : stmts)
        if (s) visitar(s.get());
}

void visitar(Nodo* nodo) {
    if (!nodo) return;

    // literals and leafs
    // each leaf node needs to set its own tipo_evaluado

    if (auto* n = dynamic_cast<NodoEntero*>(nodo)) {
        n->tipo_evaluado = Tipo::ENTERO;
        return;
    }
    if (auto* n = dynamic_cast<NodoFlotante*>(nodo)) {
        n->tipo_evaluado = Tipo::FLOTANTE;
        return;
    }
    if (auto* n = dynamic_cast<NodoLetra*>(nodo)) {
        n->tipo_evaluado = Tipo::LETRA;
        return;
    }
    if (auto* n = dynamic_cast<NodoTexto*>(nodo)) {
        n->tipo_evaluado = Tipo::TEXTO;
        return;
    }

    //  rompe/continua need to validate they're inside a loop, but they don't have a type themselves (they don't produce a value)

    if (auto* n = dynamic_cast<NodoRompe*>(nodo)) {
        if (profundidad_mientras == 0)
            reportar_error("'rompe' outside of a loop", n->linea);
        n->tipo_evaluado = Tipo::NADA;
        return;
    }
    if (auto* n = dynamic_cast<NodoContinua*>(nodo)) {
        if (profundidad_mientras == 0)
            reportar_error("'continua' outside of a loop", n->linea);
        n->tipo_evaluado = Tipo::NADA;
        return;
    }

    // var
    // looks up the symbol, reports if missing or not a variable & copies its type into tipo_evaluado for use by parent nodes

    if (auto* n = dynamic_cast<NodoAlias*>(nodo)) {
        const Simbolo* s = buscar(n->nombre);
        if (!s) {
            reportar_error("'" + n->nombre + "' was not declared in this scope",
                        n->linea);
            n->tipo_evaluado = Tipo::ERROR;
            return;
        }
        if (s->categoria == Categoria::FUNCION) {
            reportar_error("'" + n->nombre + "' is a function, not a variable",
                        n->linea);
            n->tipo_evaluado = Tipo::ERROR;
            return;
        }
        n->tipo_evaluado = s->tipo;
        return;
    }

    //  var read
    // same checks as NodoAlias but doesn't propagate a type (lee always returns NADA)

    if (auto* n = dynamic_cast<NodoLee*>(nodo)) {
        const Simbolo* s = buscar(n->nombre);
        if (!s) {
            reportar_error("'" + n->nombre + "' was not declared in this scope",
                        n->linea);
        } else if (s->categoria == Categoria::FUNCION) {
            reportar_error("'" + n->nombre + "' is a function, not a variable",
                        n->linea);
        }
        n->tipo_evaluado = Tipo::NADA;
        return;
    }

    // declaration
    // declares the variable in the current scope and, if there's an initial value,
    // visits it and checks type compatibility (with implicit promotion entero->flotante)

    if (auto* n = dynamic_cast<NodoDecl*>(nodo)) {
        Simbolo s;
        s.nombre = n->nombre;
        s.categoria = Categoria::VARIABLE;
        s.tipo = n->tipo;
        s.scope_nivel = (int)pila_scopes.size() - 1;
        s.linea = n->linea;
        declarar(s);

        if (n->valor) {
            visitar(n->valor.get());
            Tipo tv = n->valor->tipo_evaluado;
            // skip the "cannot assign" report if the value already has ERROR type (that means a deeper node already reported the real error)
            if (!es_asignable(n->tipo, tv) && tv != Tipo::ERROR) {
                reportar_error(
                    "cannot assign '" + tipo_str(tv) +
                    "' to variable '" + n->nombre +
                    "' of type '" + tipo_str(n->tipo) + "'",
                    n->linea);
            } else if (es_promocion(n->tipo, tv)) {
                registrar_promocion(tv, n->tipo, n->linea);
            }
        }
        n->tipo_evaluado = n->tipo; 
        return;
    }

    // assign
    // checks the variable was already declared and that the value's type is compatible
    // with it (with implicit promotion entero->flotante), reports if missing or not a variable

    if (auto* n = dynamic_cast<NodoAsignacion*>(nodo)) {
        const Simbolo* s = buscar(n->nombre);
        if (n->valor) visitar(n->valor.get());

        if (!s) {
            reportar_error("'" + n->nombre + "' was not declared in this scope",
                        n->linea);
            n->tipo_evaluado = Tipo::ERROR;
            return;
        }
        if (s->categoria == Categoria::FUNCION) {
            reportar_error("'" + n->nombre + "' is a function, not a variable",
                        n->linea);
            n->tipo_evaluado = Tipo::ERROR;
            return;
        }
        if (n->valor) {
            Tipo tv = n->valor->tipo_evaluado;
            if (!es_asignable(s->tipo, tv) && tv != Tipo::ERROR) {
                reportar_error(
                    "cannot assign '" + tipo_str(tv) +
                    "' to variable '" + n->nombre +
                    "' of type '" + tipo_str(s->tipo) + "'",
                    n->linea);
            } else if (es_promocion(s->tipo, tv)) {
                registrar_promocion(tv, s->tipo, n->linea);
            }
        }
        n->tipo_evaluado = s->tipo;
        return;
    }

    // exp
    // visits children, checks operator compatibility and sets tipo_evaluado with the result type (or ERROR if incompatible, with error reporting)

    if (auto* n = dynamic_cast<NodoBinop*>(nodo)) {
        visitar(n->izq.get());
        visitar(n->der.get());

        Tipo ti = n->izq ? n->izq->tipo_evaluado : Tipo::ERROR;
        Tipo td = n->der ? n->der->tipo_evaluado : Tipo::ERROR;

        Tipo r = tipo_resultante(n->op, ti, td);
        if (r == Tipo::ERROR && ti != Tipo::ERROR && td != Tipo::ERROR) {
            reportar_error(
                "operator '" + op_str(n->op) +
                "' not defined for types '" + tipo_str(ti) +
                "' and '" + tipo_str(td) + "'",
                n->linea);
        } else {
            if (r == Tipo::FLOTANTE) {
                if (ti == Tipo::ENTERO) registrar_promocion(ti, r, n->linea);
                if (td == Tipo::ENTERO) registrar_promocion(td, r, n->linea);
            }
        }
        n->tipo_evaluado = r;
        return;
    }

    // unop is similar but only has one child and operator compatibility is checked only with that child

    if (auto* n = dynamic_cast<NodoUnop*>(nodo)) {
        visitar(n->hijo.get());
        Tipo th = n->hijo ? n->hijo->tipo_evaluado : Tipo::ERROR;

        Tipo r = tipo_resultante(n->op, th);
        if (r == Tipo::ERROR && th != Tipo::ERROR) {
            std::string op_s = (n->op == OpUnaria::NEGACION) ? "-" : "!";
            reportar_error(
                "unary operator '" + op_s +
                "' not defined for type '" + tipo_str(th) + "'",
                n->linea);
        }
        n->tipo_evaluado = r;
        return;
    }

    //function call
    // checks the function exists, that it IS a function (not a variable),
    // that the number of arguments matches, and that each argument's type is compatible with the corresponding parameter

    if (auto* n = dynamic_cast<NodoLlamada*>(nodo)) {
        const Simbolo* s = buscar(n->nombre);

        for (const auto& a : n->args)
            if (a) visitar(a.get());

        if (!s) {
            reportar_error("'" + n->nombre + "' was not declared in this scope",
                        n->linea);
            n->tipo_evaluado = Tipo::ERROR;
            return;
        }
        if (s->categoria != Categoria::FUNCION) {
            reportar_error("'" + n->nombre + "' is not a function", n->linea);
            n->tipo_evaluado = Tipo::ERROR;
            return;
        }

        if (n->args.size() != s->tipos_parametros.size()) {
            std::ostringstream oss;
            oss << "function '" << n->nombre << "' expects "
                << s->tipos_parametros.size() << " arguments but got "
                << n->args.size();
            reportar_error(oss.str(), n->linea);
        } else {
            for (size_t i = 0; i < n->args.size(); ++i) {
                Tipo esperado = s->tipos_parametros[i];
                Tipo recibido = n->args[i]->tipo_evaluado;
                if (!es_asignable(esperado, recibido) && recibido != Tipo::ERROR) {
                    std::ostringstream oss;
                    oss << "argument " << (i + 1) << " of '" << n->nombre
                        << "' expects '" << tipo_str(esperado)
                        << "' but got '"  << tipo_str(recibido) << "'";
                    reportar_error(oss.str(), n->linea);
                } else if (es_promocion(esperado, recibido)) {
                    registrar_promocion(recibido, esperado, n->linea);
                }
            }
        }
        n->tipo_evaluado = s->tipo;
        return;
    }

    // muestra
    // accepts any printable type, so we just visit the value and don't validate its type

    if (auto* n = dynamic_cast<NodoMuestra*>(nodo)) {
        if (n->valor) visitar(n->valor.get());
        n->tipo_evaluado = Tipo::NADA;
        return;
    }

    // devuelve
    // must be inside a function, and the returned value's type must be compatible
    // with the function's declared return type (with implicit promotion entero->flotante)

    if (auto* n = dynamic_cast<NodoDevuelve*>(nodo)) {
        if (n->valor) visitar(n->valor.get());

        if (!dentro_de_funcion) {
            reportar_error("'devuelve' outside of a function", n->linea);
            n->tipo_evaluado = Tipo::NADA;
            return;
        }

        Tipo tv = n->valor ? n->valor->tipo_evaluado : Tipo::NADA;
        if (!es_asignable(tipo_retorno_esperado, tv) && tv != Tipo::ERROR) {
            reportar_error(
                "cannot return '" + tipo_str(tv) +
                "' from function of type '" +
                tipo_str(tipo_retorno_esperado) + "'",
                n->linea);
        } else if (es_promocion(tipo_retorno_esperado, tv)) {
            registrar_promocion(tv, tipo_retorno_esperado, n->linea);
        }
        n->tipo_evaluado = Tipo::NADA;
        return;
    }

    // cuando
    // validates the condition is boolean, then opens a new scope for each branch

    if (auto* n = dynamic_cast<NodoCuando*>(nodo)) {
        if (n->condicion) {
            visitar(n->condicion.get());
            Tipo tc = n->condicion->tipo_evaluado;
            if (tc != Tipo::BOOLEANO && tc != Tipo::ERROR) {
                reportar_error(
                    "condition must be boolean, got '" + tipo_str(tc) + "'",
                    n->linea);
            }
        }

        abrir_scope();
        visitar_bloque(n->entonces);
        cerrar_scope();

        if (!n->sino.empty()) {
            abrir_scope();
            visitar_bloque(n->sino);
            cerrar_scope();
        }
        n->tipo_evaluado = Tipo::NADA;
        return;
    }

    // mientras
    // same as cuando but also tracks loop depth so rompe/continua can validate they're inside a loop
    
    if (auto* n = dynamic_cast<NodoMientras*>(nodo)) {
        if (n->condicion) {
            visitar(n->condicion.get());
            Tipo tc = n->condicion->tipo_evaluado;
            if (tc != Tipo::BOOLEANO && tc != Tipo::ERROR) {
                reportar_error(
                    "condition must be boolean, got '" + tipo_str(tc) + "'",
                    n->linea);
            }
        }

        profundidad_mientras++;
        abrir_scope();
        visitar_bloque(n->cuerpo);
        cerrar_scope();
        profundidad_mientras--;
        n->tipo_evaluado = Tipo::NADA;
        return;
    }

    // function
    // declares the function in the parent scope (so it's visible from outside and supports recursion)
    //then opens its own scope for parameters and body

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
            if (auto* param = dynamic_cast<NodoParam*>(p.get())) {
                Simbolo sp;
                sp.nombre      = param->nombre;
                sp.categoria   = Categoria::PARAMETRO;
                sp.tipo        = param->tipo;
                sp.scope_nivel = (int)pila_scopes.size() - 1;
                sp.linea       = param->linea;
                declarar(sp);
            }
        }

        // -> save context to support nested functions, restored after visiting body
        Tipo prev_retorno = tipo_retorno_esperado; 
        bool prev_en_funcion = dentro_de_funcion; 
        tipo_retorno_esperado = n->tipo_retorno; 
        dentro_de_funcion = true;

        visitar_bloque(n->cuerpo);

        tipo_retorno_esperado = prev_retorno;
        dentro_de_funcion = prev_en_funcion;

        cerrar_scope();
        n->tipo_evaluado = Tipo::NADA;
        return;
    }

    // program
    // top level scope for the whole program (global scope)

    if (auto* n = dynamic_cast<NodoPrograma*>(nodo)) {
        abrir_scope(); // global scope
        visitar_bloque(n->parrafos);
        cerrar_scope();
        n->tipo_evaluado = Tipo::NADA;
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


// prints all collected implicit promotions 
void imprimir_promociones() {
    if (promociones.empty()) return;
    printf("\nImplicit promotions:\n");
    for (const auto& p : promociones) {
        printf("  '%s' -> '%s' at line %d\n",
            tipo_str(p.origen).c_str(),
            tipo_str(p.destino).c_str(),
            p.linea);
    }
}

bool hubo_errores() {
    return !errores_semanticos.empty();
}

void yyerror(const char* msg) {
    extern int yylineno;
    reportar_error(msg, yylineno);
}