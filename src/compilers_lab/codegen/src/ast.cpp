#include "ast.h"
#include <cstring>

/* @brief Make a new integer node 
    @param val The integer value
    @param linea The line number
    @return A pointer to the new integer node */
Nodo* hacer_entero(int val, int linea) {
    auto n = new NodoEntero();
    n->linea = linea;
    n->valor = val;
    return n;
}

/** @brief Make a new float node
    @param val The float value
    @param linea The line number
    @return A pointer to the new float node */
Nodo* hacer_flotante(double val, int linea) {
    auto n = new NodoFlotante();
    n->linea = linea;
    n->valor = val;
    return n;
}

/** @brief Make a new char node
    @param val The char value
    @param linea The line number
    @return A pointer to the new char node */
Nodo* hacer_letra(char val, int linea) {
    auto n = new NodoLetra();
    n->linea = linea;
    n->valor = val;
    return n;
}

/** @brief Make a new text node
    @param val The text value
    @param linea The line number
    @return A pointer to the new text node */
Nodo* hacer_texto(const char* val, int linea) {
    auto n = new NodoTexto();
    n->linea = linea;
    n->valor = std::string(val);
    return n;
}

/** @brief Make a new alias node
    @param nombre The alias name
    @param linea The line number
    @return A pointer to the new alias node */
Nodo* hacer_alias(const char* nombre, int linea) {
    auto n = new NodoAlias();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    return n;
}

/** @brief Make a new binary operation node
    @param op The binary operation
    @param izq The left operand
    @param der The right operand
    @param linea The line number
    @return A pointer to the new binary operation node */
Nodo* hacer_binop(OpBinaria op, Nodo* izq, Nodo* der, int linea) {
    auto n = new NodoBinop();
    n->linea = linea;
    n->op    = op;
    n->izq   = NodoPtr(izq);
    n->der   = NodoPtr(der);
    return n;
}

/** @brief Make a new unary operation node
    @param op The unary operation
    @param hijo The child node
    @param linea The line number
    @return A pointer to the new unary operation node */
Nodo* hacer_unop(OpUnaria op, Nodo* hijo, int linea) {
    auto n = new NodoUnop();
    n->linea = linea;
    n->op    = op;
    n->hijo  = NodoPtr(hijo);
    return n;
}

/** @brief Make a new function call node
    @param nombre The function name
    @param args The arguments
    @param linea The line number
    @return A pointer to the new function call node */
Nodo* hacer_llamada(const char* nombre, std::vector<Nodo*>* args, int linea) {
    auto n = new NodoLlamada();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    for (Nodo* arg : *args)
        n->args.push_back(NodoPtr(arg));
    delete args;
    return n;
}

/** @brief Make a new variable declaration node
    @param tipo The variable type
    @param nombre The variable name
    @param valor The initial value
    @param linea The line number
    @return A pointer to the new variable declaration node */
Nodo* hacer_decl(Tipo tipo, const char* nombre, Nodo* valor, int linea) {
    auto n = new NodoDecl();
    n->linea  = linea;
    n->tipo   = tipo;
    n->nombre = std::string(nombre);
    n->valor  = NodoPtr(valor);   // NodoPtr(nullptr) == nullptr, está bien
    return n;
}

/** @brief Make a new assignment node
    @param nombre The variable name
    @param valor The assigned value
    @param linea The line number
    @return A pointer to the new assignment node */
Nodo* hacer_asignacion(const char* nombre, Nodo* valor, int linea) {
    auto n = new NodoAsignacion();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    n->valor  = NodoPtr(valor);
    return n;
}

/** @brief Make a new conditional node
    @param cond The condition
    @param entonces The then branch
    @param sino The else branch
    @param linea The line number
    @return A pointer to the new conditional node */
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

/** @brief Make a new while loop node
    @param cond The condition
    @param cuerpo The body of the loop
    @param linea The line number
    @return A pointer to the new while loop node */
Nodo* hacer_mientras(Nodo* cond, std::vector<Nodo*>* cuerpo, int linea) {
    auto n = new NodoMientras();
    n->linea     = linea;
    n->condicion = NodoPtr(cond);
    for (Nodo* s : *cuerpo)
        n->cuerpo.push_back(NodoPtr(s));
    delete cuerpo;
    return n;
}

/** @brief Make a new return node
    @param valor The returned value
    @param linea The line number
    @return A pointer to the new return node */
Nodo* hacer_devuelve(Nodo* valor, int linea) {
    auto n = new NodoDevuelve();
    n->linea = linea;
    n->valor = NodoPtr(valor);
    return n;
}

/** @brief Make a new break node
    @param linea The line number
    @return A pointer to the new break node */
Nodo* hacer_rompe(int linea) {
    auto n = new NodoRompe();
    n->linea = linea;
    return n;
}

/** @brief Make a new continue node
    @param linea The line number
    @return A pointer to the new continue node */
Nodo* hacer_continua(int linea) {
    auto n = new NodoContinua();
    n->linea = linea;
    return n;
}

/** @brief Make a new print node
    @param valor The value to print
    @param linea The line number
    @return A pointer to the new print node */
Nodo* hacer_muestra(Nodo* valor, int linea) {
    auto n = new NodoMuestra();
    n->linea = linea;
    n->valor = NodoPtr(valor);
    return n;
}

/** @brief Make a new read node
    @param nombre The variable name
    @param linea The line number
    @return A pointer to the new read node */
Nodo* hacer_lee(const char* nombre, int linea) {
    auto n = new NodoLee();
    n->linea  = linea;
    n->nombre = std::string(nombre);
    return n;
}

/** @brief Make a new parameter node
    @param tipo The parameter type
    @param nombre The parameter name
    @param linea The line number
    @return A pointer to the new parameter node */
Nodo* hacer_param(Tipo tipo, const char* nombre, int linea) {
    auto n = new NodoParam();
    n->linea  = linea;
    n->tipo   = tipo;
    n->nombre = std::string(nombre);
    return n;
}

/** @brief Make a new function node
    @param retorno The return type
    @param nombre The function name
    @param params The parameter list
    @param cuerpo The function body
    @param linea The line number
    @return A pointer to the new function node */
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

/** @brief Make a new program node
    @param destinatario The program's destinatario (alias after "hola,")
    @param firma The program's firma (alias after "atentamente,")
    @param parrafos The program's paragraphs
    @param linea The line number
    @return A pointer to the new program node */
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