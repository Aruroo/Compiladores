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
        std::cout << sangria(nivel) << "ENTERO(" << valor << ")\n";
    }
};

struct NodoFlotante : Nodo {
    double valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "FLOTANTE(" << valor << ")\n";
    }
};

struct NodoLetra : Nodo {
    char valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "LETRA('" << valor << "')\n";
    }
};

struct NodoTexto : Nodo {
    std::string valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "TEXTO(\"" << valor << "\")\n";
    }
};

struct NodoAlias : Nodo {
    std::string nombre;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "ALIAS(" << nombre << ")\n";
    }
};

struct NodoBinop : Nodo { // aritmetic or logic
    OpBinaria op;
    NodoPtr izq;
    NodoPtr der;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "BINOP(" << op_str(op) << ")\n";
        if (izq) izq->imprimir(nivel + 1);
        if (der) der->imprimir(nivel + 1);
    }
};

struct NodoUnop : Nodo {
    OpUnaria op;
    NodoPtr hijo;
    void imprimir(int nivel) const override {
        std::string op_s = (op == OpUnaria::NEGACION) ? "-" : "!";
        std::cout << sangria(nivel) << "UNOP(" << op_s << ")\n";
        if (hijo) hijo->imprimir(nivel + 1);
    }
};

struct NodoLlamada : Nodo {
    std::string nombre;
    std::vector<NodoPtr> args;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "LLAMADA(" << nombre << ")\n";
        for (const auto &a : args) if (a) a->imprimir(nivel + 1);
    }
};

struct NodoDecl : Nodo {
    Tipo tipo;
    std::string nombre;
    NodoPtr valor;          // nullptr si if not initialized
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "DECL(" << tipo_str(tipo) << " " << nombre << ")\n";
        if (valor) valor->imprimir(nivel + 1);
    }
};

struct NodoAsignacion : Nodo {
    std::string nombre;
    NodoPtr valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "ASIGNACION(" << nombre << ")\n";
        if (valor) valor->imprimir(nivel + 1);
    }
};

struct NodoCuando : Nodo {
    NodoPtr condicion; // can be any boolean or aritmetic expresion
    std::vector<NodoPtr> entonces;
    std::vector<NodoPtr> sino;      // empty if there's no "sino"
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "CUANDO\n";
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
        std::cout << sangria(nivel) << "MIENTRAS\n";
        std::cout << sangria(nivel + 1) << "CONDICION:\n";
        if (condicion) condicion->imprimir(nivel + 2);
        std::cout << sangria(nivel + 1) << "CUERPO:\n";
        for (const auto &o : cuerpo) if (o) o->imprimir(nivel + 2);
    }
};

struct NodoDevuelve : Nodo {
    NodoPtr valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "DEVUELVE\n";
        if (valor) valor->imprimir(nivel + 1);
    }
};

struct NodoMuestra : Nodo {
    NodoPtr valor;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "MUESTRA\n";
        if (valor) valor->imprimir(nivel + 1);
    }

};

struct NodoLee : Nodo {
    std::string nombre;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "LEE(" << nombre << ")\n";
    }
};

struct NodoRompe : Nodo {
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "ROMPE\n";
    }
};

struct NodoContinua : Nodo {
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "CONTINUA\n";
    }
};

struct NodoParam : Nodo {
    Tipo tipo;
    std::string nombre;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "PARAM(" << tipo_str(tipo) << " " << nombre << ")\n";
    }
};

struct NodoFuncion : Nodo {
    Tipo tipo_retorno;
    std::string nombre;
    std::vector<NodoPtr> params;
    std::vector<NodoPtr> cuerpo;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "FUNCION(" << tipo_str(tipo_retorno) << " " << nombre << ")\n";
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
    std::vector<NodoPtr> parrafos;
    void imprimir(int nivel) const override {
        std::cout << sangria(nivel) << "PROGRAMA\n";
        std::cout << sangria(nivel + 1) << "destinatario: " << destinatario << "\n";
        std::cout << sangria(nivel + 1) << "firma: " << firma << "\n";
        std::cout << sangria(nivel + 1) << "PARRAFOS:\n";
        for (const auto &p : parrafos) if (p) p->imprimir(nivel + 2);
    }
};