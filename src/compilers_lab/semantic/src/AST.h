#pragma once
#include <string>
#include <vector>
#include <memory>

// unique_ptr offers automatic memory managment
using NodoPtr = std::unique_ptr<struct Nodo>;

enum class Tipo {
    ENTERO,
    FLOTANTE,
    LETRA,
    TEXTO,
    NADA,
};

enum class OpBinaria {
    SUMA, RESTA, MUL, DIV, MOD,
    EQ, NEQ, LT, LE, GT, GE,
    AND, OR,
};

enum class OpUnaria {
    NEGACION,
    NOT,
};

struct Nodo {
    int linea;
    virtual ~Nodo() = default;
};

struct NodoEntero : Nodo {
    int valor;
};

struct NodoFlotante : Nodo {
    double valor;
};

struct NodoLetra : Nodo {
    char valor;
};

struct NodoTexto : Nodo {
    std::string valor;
};

struct NodoAlias : Nodo {
    std::string nombre;
};

struct NodoBinop : Nodo { // aritmetic or logic
    OpBinaria op;
    NodoPtr izq;
    NodoPtr der;
};

struct NodoUnop : Nodo {
    OpUnaria op;
    NodoPtr hijo;
};

struct NodoLlamada : Nodo {
    std::string nombre;
    std::vector<NodoPtr> args;
};

struct NodoDecl : Nodo {
    Tipo tipo;
    std::string nombre;
    NodoPtr valor;          // nullptr si if not initialized
};

struct NodoAsignacion : Nodo {
    std::string nombre;
    NodoPtr valor;
};

struct NodoCuando : Nodo {
    NodoPtr condicion; // can be any boolean or aritmetic expresion
    std::vector<NodoPtr> entonces;
    std::vector<NodoPtr> sino;      // empty if there's no "sino"
};

struct NodoMientras : Nodo {
    NodoPtr condicion;
    std::vector<NodoPtr> cuerpo;
};

struct NodoDevuelve : Nodo {
    NodoPtr valor;
};

struct NodoMuestra : Nodo {
    NodoPtr valor;
};

struct NodoLee : Nodo {
    std::string nombre;
};

struct NodoRompe : Nodo {};
struct NodoContinua : Nodo {};

struct NodoParam : Nodo {
    Tipo tipo;
    std::string nombre;
};

struct NodoFuncion : Nodo {
    Tipo tipo_retorno;
    std::string nombre;
    std::vector<NodoPtr> params;
    std::vector<NodoPtr> cuerpo;
};

struct NodoPrograma : Nodo {
    std::string destinatario;       // alias after "hola,"
    std::string firma;              // alias after "atentamente,"
    std::vector<NodoPtr> parrafos;
};