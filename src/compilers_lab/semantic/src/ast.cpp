#include "ast.h"
#include <cstring>

Nodo* hacer_entero(int val, int linea) {
    auto n = new NodoEntero();
    n->linea = linea;
    n->valor = val;
    return n;
}

Nodo* hacer_flotante(double val, int linea) {
    auto n = new NodoFlotante();
    n->linea = linea;
    n->valor = val;
    return n;
}

Nodo* hacer_letra(char val, int linea) {
    auto n = new NodoLetra();
    n->linea = linea;
    n->valor = val;
    return n;
}

Nodo* hacer_texto(const char* val, int linea) {
    auto n = new NodoTexto();
    n->linea = linea;
    n->valor = std::string(val);
    return n;
}

Nodo* hacer_alias(const char* nombre, int linea) {
    auto n = new NodoAlias();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    return n;
}

Nodo* hacer_binop(OpBinaria op, Nodo* izq, Nodo* der, int linea) {
    auto n = new NodoBinop();
    n->linea = linea;
    n->op    = op;
    n->izq   = NodoPtr(izq);
    n->der   = NodoPtr(der);
    return n;
}

Nodo* hacer_unop(OpUnaria op, Nodo* hijo, int linea) {
    auto n = new NodoUnop();
    n->linea = linea;
    n->op    = op;
    n->hijo  = NodoPtr(hijo);
    return n;
}

Nodo* hacer_llamada(const char* nombre, std::vector<Nodo*>* args, int linea) {
    auto n = new NodoLlamada();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    for (Nodo* arg : *args)
        n->args.push_back(NodoPtr(arg));
    delete args;
    return n;
}

Nodo* hacer_decl(Tipo tipo, const char* nombre, Nodo* valor, int linea) {
    auto n = new NodoDecl();
    n->linea  = linea;
    n->tipo   = tipo;
    n->nombre = std::string(nombre);
    n->valor  = NodoPtr(valor);   // NodoPtr(nullptr) == nullptr, está bien
    return n;
}

Nodo* hacer_asignacion(const char* nombre, Nodo* valor, int linea) {
    auto n = new NodoAsignacion();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    n->valor  = NodoPtr(valor);
    return n;
}

Nodo* hacer_cuando(Nodo* cond, std::vector<Nodo*>* entonces,
                   std::vector<Nodo*>* sino, int linea) {
    auto n = new NodoCuando();
    n->linea     = linea;
    n->condicion = NodoPtr(cond);
    for (Nodo* s : *entonces)
        n->entonces.push_back(NodoPtr(s));
    for (Nodo* s : *sino)
        n->sino.push_back(NodoPtr(s));
    delete entonces;
    delete sino;
    return n;
}

Nodo* hacer_mientras(Nodo* cond, std::vector<Nodo*>* cuerpo, int linea) {
    auto n = new NodoMientras();
    n->linea     = linea;
    n->condicion = NodoPtr(cond);
    for (Nodo* s : *cuerpo)
        n->cuerpo.push_back(NodoPtr(s));
    delete cuerpo;
    return n;
}

Nodo* hacer_devuelve(Nodo* valor, int linea) {
    auto n = new NodoDevuelve();
    n->linea = linea;
    n->valor = NodoPtr(valor);
    return n;
}

Nodo* hacer_rompe(int linea) {
    auto n = new NodoRompe();
    n->linea = linea;
    return n;
}

Nodo* hacer_continua(int linea) {
    auto n = new NodoContinua();
    n->linea = linea;
    return n;
}

Nodo* hacer_muestra(Nodo* valor, int linea) {
    auto n = new NodoMuestra();
    n->linea = linea;
    n->valor = NodoPtr(valor);
    return n;
}

Nodo* hacer_lee(const char* nombre, int linea) {
    auto n = new NodoLee();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    return n;
}

Nodo* hacer_param(Tipo tipo, const char* nombre, int linea) {
    auto n = new NodoParam();
    n->linea  = linea;
    n->tipo   = tipo;
    n->nombre = std::string(nombre);
    return n;
}

Nodo* hacer_funcion(Tipo retorno, const char* nombre,
                    std::vector<Nodo*>* params,
                    std::vector<Nodo*>* cuerpo, int linea) {
    auto n = new NodoFuncion();
    n->linea        = linea;
    n->tipo_retorno = retorno;
    n->nombre       = std::string(nombre);
    for (Nodo* p : *params)
        n->params.push_back(NodoPtr(p));
    for (Nodo* s : *cuerpo)
        n->cuerpo.push_back(NodoPtr(s));
    delete params;
    delete cuerpo;
    return n;
}

Nodo* hacer_programa(const char* destinatario, const char* firma,
                     std::vector<Nodo*>* parrafos, int linea) {
    auto n = new NodoPrograma();
    n->linea        = linea;
    n->destinatario = std::string(destinatario);
    n->firma        = std::string(firma);
    for (Nodo* p : *parrafos)
        n->parrafos.push_back(NodoPtr(p));
    delete parrafos;
    return n;
}