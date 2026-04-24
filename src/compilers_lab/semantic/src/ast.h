#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

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

inline std::string tipo_str(Tipo t) { // return the string representation of the type
    switch(t) {
        case Tipo::ENTERO: return "entero";
        case Tipo::FLOTANTE: return "flotante";
        case Tipo::LETRA: return "letra";
        case Tipo::TEXTO: return "texto";
        case Tipo::NADA: return "nada";
    }
    return "?";
}

inline std::string op_str(OpBinaria op) { // return the string representation of the operator
    switch(op) {
        case OpBinaria::SUMA: return "+";
        case OpBinaria::RESTA: return "-";
        case OpBinaria::MUL: return "*";
        case OpBinaria::DIV: return "/";
        case OpBinaria::MOD: return "%";
        case OpBinaria::EQ: return "==";
        case OpBinaria::NEQ: return "!=";
        case OpBinaria::LT: return "<";
        case OpBinaria::LE: return "<=";
        case OpBinaria::GT: return ">";
        case OpBinaria::GE: return ">=";
        case OpBinaria::AND: return "&&";
        case OpBinaria::OR: return "||";
    }
    return "?";
}

inline std::string sangria(int nivel) { // return a string with 2 spaces per level of indentation
    return std::string(nivel * 2, ' ');
}

struct Nodo {
    int linea;
    virtual ~Nodo() = default;
    virtual void imprimir(int nivel = 0) const = 0;
};

struct NodoEntero : Nodo {
    int valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "ENTERO(" << valor << ") [linea " << linea << "]\n";
    }
};

struct NodoFlotante : Nodo {
    double valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "FLOTANTE(" << valor << ") [linea " << linea << "]\n";
    }
};

struct NodoLetra : Nodo {
    char valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "LETRA('" << valor << "') [linea " << linea << "]\n";
    }
};

struct NodoTexto : Nodo {
    std::string valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "TEXTO(\"" << valor << "\") [linea " << linea << "]\n";    }
};

struct NodoAlias : Nodo {
    std::string nombre;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "ALIAS(" << nombre << ") [linea " << linea << "]\n";    }
};

struct NodoBinop : Nodo { // aritmetic or logic
    OpBinaria op;
    NodoPtr izq;
    NodoPtr der;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "BINOP(" << op_str(op) << ") [linea " << linea << "]\n";
        if (izq) izq->imprimir(nivel + 1);
        if (der) der->imprimir(nivel + 1);
    }
};

struct NodoUnop : Nodo {
    OpUnaria op;
    NodoPtr hijo;
    void imprimir(int nivel) const override {
        std::string op_s = (op == OpUnaria::NEGACION) ? "-" : "!";
        std::cout << sangria(nivel) << "UNOP(" << op_s << ") [linea " << linea << "]\n";
        if (hijo) hijo->imprimir(nivel + 1);
    }
};

struct NodoLlamada : Nodo {
    std::string nombre;
    std::vector<NodoPtr> args;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "LLAMADA(" << nombre << ") [linea " << linea << "]\n";
        for (const auto &a : args) if (a) a->imprimir(nivel + 1);
    }
};

struct NodoDecl : Nodo {
    Tipo tipo;
    std::string nombre;
    NodoPtr valor;          // nullptr si if not initialized
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "DECL(" << tipo_str(tipo) << " " << nombre << ") [linea " << linea << "]\n";
        if (valor) valor->imprimir(nivel + 1);
    }
};

struct NodoAsignacion : Nodo {
    std::string nombre;
    NodoPtr valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "ASIGNACION(" << nombre << ") [linea " << linea << "]\n";
        if (valor) valor->imprimir(nivel + 1);
    }
};

struct NodoCuando : Nodo {
    NodoPtr condicion; // can be any boolean or aritmetic expresion
    std::vector<NodoPtr> entonces;
    std::vector<NodoPtr> sino;      // empty if there's no "sino"
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "CUANDO [linea " << linea << "]\n";
        std::cout << sangria(nivel + 1) << "CONDICION:\n";
        if (condicion) condicion->imprimir(nivel + 2);
        std::cout << sangria(nivel + 1) << "ENTONCES:\n";
        for (const auto &o : entonces) if (o) o->imprimir(nivel + 2);
        if (!sino.empty()) {
            std::cout << sangria(nivel + 1) << "SINO:\n";
            for (const auto &o : sino) if (o) o->imprimir(nivel + 2);
        }
    }
};

struct NodoMientras : Nodo {
    NodoPtr condicion;
    std::vector<NodoPtr> cuerpo;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "MIENTRAS [linea " << linea << "]\n";
        std::cout << sangria(nivel + 1) << "CONDICION:\n";
        if (condicion) condicion->imprimir(nivel + 2);
        std::cout << sangria(nivel + 1) << "CUERPO:\n";
        for (const auto &o : cuerpo) if (o) o->imprimir(nivel + 2);
    }
};

struct NodoDevuelve : Nodo {
    NodoPtr valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "DEVUELVE [linea " << linea << "]\n";
        if (valor) valor->imprimir(nivel + 1);
    }
};

struct NodoMuestra : Nodo {
    NodoPtr valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "MUESTRA [linea " << linea << "]\n";
        if (valor) valor->imprimir(nivel + 1);
    }
};

struct NodoLee : Nodo {
    std::string nombre;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "LEE(" << nombre << ") [linea " << linea << "]\n";    }
};

struct NodoRompe : Nodo {
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "ROMPE [linea " << linea << "]\n";
    }
};

struct NodoContinua : Nodo {
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "CONTINUA [linea " << linea << "]\n";    }
};

struct NodoParam : Nodo {
    Tipo tipo;
    std::string nombre;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "PARAM(" << tipo_str(tipo) << " " << nombre << ") [linea " << linea << "]\n";
    }
};

struct NodoFuncion : Nodo {
    Tipo tipo_retorno;
    std::string nombre;
    std::vector<NodoPtr> params;
    std::vector<NodoPtr> cuerpo;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "FUNCION(" << tipo_str(tipo_retorno) << " " << nombre << ") [linea " << linea << "]\n";
        if (!params.empty()) {
            std::cout << sangria(nivel + 1) << "PARAMS:\n";
            for (const auto &p : params) if (p) p->imprimir(nivel + 2);
        }
        std::cout << sangria(nivel + 1) << "CUERPO:\n";
        for (const auto &o : cuerpo) if (o) o->imprimir(nivel + 2);
    }
};

struct NodoPrograma : Nodo {
    std::string destinatario; // alias after "hola,"
    std::string firma; // alias after "atentamente,"
    int linea_firma;    // línea del atentamente
    std::vector<NodoPtr> parrafos;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "PROGRAMA [linea " << linea << "]\n";
        std::cout << sangria(nivel + 1) << "destinatario: " << destinatario << "\n";
        std::cout << sangria(nivel + 1) << "firma: " << firma << " [linea " << linea_firma << "]\n";
        std::cout << sangria(nivel + 1) << "PARRAFOS:\n";
        for (const auto &p : parrafos) if (p) p->imprimir(nivel + 2);
    }
};

Nodo* hacer_entero   (int val,         int linea);
Nodo* hacer_flotante (double val,      int linea);
Nodo* hacer_letra    (char val,        int linea);
Nodo* hacer_texto    (const char* val, int linea);
Nodo* hacer_alias    (const char* nombre, int linea);

Nodo* hacer_binop (OpBinaria op, Nodo* izq, Nodo* der, int linea);
Nodo* hacer_unop  (OpUnaria  op, Nodo* hijo,            int linea);

Nodo* hacer_llamada (const char* nombre, std::vector<Nodo*>* args, int linea);

Nodo* hacer_decl       (Tipo tipo, const char* nombre, Nodo* valor, int linea); // valor puede ser nullptr
Nodo* hacer_asignacion (const char* nombre, Nodo* valor, int linea);

Nodo* hacer_cuando  (Nodo* cond, std::vector<Nodo*>* entonces, std::vector<Nodo*>* sino, int linea);
Nodo* hacer_mientras (Nodo* cond, std::vector<Nodo*>* cuerpo, int linea);
Nodo* hacer_devuelve (Nodo* valor, int linea);
Nodo* hacer_rompe    (int linea);
Nodo* hacer_continua (int linea);


Nodo* hacer_muestra (Nodo* valor,          int linea);
Nodo* hacer_lee     (const char* nombre,   int linea);

Nodo* hacer_param (Tipo tipo, const char* nombre, int linea);

Nodo* hacer_funcion (Tipo retorno, const char* nombre,
                     std::vector<Nodo*>* params,
                     std::vector<Nodo*>* cuerpo, int linea);

Nodo* hacer_programa (const char* destinatario, const char* firma,
                      std::vector<Nodo*>* parrafos, int linea);