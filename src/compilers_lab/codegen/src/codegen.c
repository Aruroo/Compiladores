#include "codegen.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>
#include <sstream>

/* Intermediate code generator FIS-25 with optimization.
 *
 * The code is first built as a list of three-address instructions
 * (struct Instr) and then four optimization passes are applied:
 *
 *   1. LICM  (loop-invariant code motion): hoists computations whose
 *            operands do not change inside the loop out of it.
 *   2. CSE   (common subexpression elimination): within each basic block,
 *            reuses the result of an already-computed expression.
 *   3. DCE   (dead code elimination): removes instructions whose result
 *            (a temporary) is never read anywhere.
 *   4. Temporary reuse: assigns multiple temporaries to the same memory
 *            cell when their live ranges do not overlap (like a recycled
 *            register), reducing the number of VAR declarations.
 *
 * VM conventions are the same as in the non-optimized version:
 * integer division via MOD/SUB/DIV, booleans without logic instructions,
 * calls with PARAM (left->right) retrieved by PARAM_GET (LIFO), and return
 * via the global __ret.
 */

namespace {

struct Instr {
    std::string op;                 // ADD, ASSIGN, LABEL, GOTO, IFFALSE, ...
    std::vector<std::string> arg;   // operands in order
};

/* ---------- operand classification utilities ---------- */

bool es_temp(const std::string& s) {
    if (s.size() < 2 || s[0] != 't') return false;
    for (size_t i = 1; i < s.size(); ++i)
        if (!isdigit((unsigned char)s[i])) return false;
    return true;
}

bool es_literal(const std::string& s) {
    if (s.empty()) return false;
    if (s[0] == '"' || s[0] == '\'') return true;     // string or char literal
    size_t i = (s[0] == '-' || s[0] == '+') ? 1 : 0;
    if (i >= s.size()) return false;
    bool digito = false;
    for (; i < s.size(); ++i) {
        if (isdigit((unsigned char)s[i])) digito = true;
        else if (s[i] != '.') return false;
    }
    return digito;
}

bool es_arit3(const std::string& op) {
    return op=="ADD"||op=="SUB"||op=="MUL"||op=="DIV"||op=="MOD"||op=="POW"||
           op=="EQ"||op=="NEQ"||op=="LT"||op=="LTE"||op=="GT"||op=="GTE";
}

bool es_def_pura(const std::string& op) {  // candidate for CSE/DCE
    return es_arit3(op) || op=="ASSIGN";
}

// name that the instruction writes, or "" if it writes nothing
std::string destino(const Instr& in) {
    if (es_arit3(in.op))       return in.arg.size()>=3 ? in.arg[2] : "";
    if (in.op=="ASSIGN")       return in.arg.size()>=2 ? in.arg[1] : "";
    if (in.op=="PARAM_GET")    return in.arg.size()>=1 ? in.arg[0] : "";
    if (in.op=="INPUT")        return in.arg.size()>=1 ? in.arg[0] : "";
    return "";
}

// positions of arg that are read operands (not labels)
std::vector<int> pos_fuente(const Instr& in) {
    if (es_arit3(in.op))     return {0,1};
    if (in.op=="ASSIGN")     return {0};
    if (in.op=="IF" || in.op=="IFFALSE" ||
        in.op=="PRINT" || in.op=="PARAM") return {0};
    return {};
}

// positions of arg that are memory cells (read or write)
std::vector<int> pos_memoria(const Instr& in) {
    if (es_arit3(in.op))     return {0,1,2};
    if (in.op=="ASSIGN")     return {0,1};
    if (in.op=="IF" || in.op=="IFFALSE" ||
        in.op=="PRINT" || in.op=="PARAM" ||
        in.op=="PARAM_GET" || in.op=="INPUT") return {0};
    return {};
}

std::vector<std::string> fuentes(const Instr& in) {
    std::vector<std::string> r;
    for (int p : pos_fuente(in))
        if (p < (int)in.arg.size()) r.push_back(in.arg[p]);
    return r;
}

bool es_control(const std::string& op) {
    return op=="LABEL"||op=="GOTO"||op=="IF"||op=="IFFALSE"||
           op=="GOSUB"||op=="RETURN";
}

/* =====================  PASS 1: LICM  ===================== */
// moves a loop-invariant instruction out of the loop; returns true if anything moved
bool licm_un_paso(std::vector<Instr>& code) {
    std::map<std::string,int> lbl;
    for (int i = 0; i < (int)code.size(); ++i)
        if (code[i].op=="LABEL") lbl[code[i].arg[0]] = i;

    for (int b = 0; b < (int)code.size(); ++b) {
        if (code[b].op != "GOTO") continue;
        auto it = lbl.find(code[b].arg[0]);
        if (it == lbl.end()) continue;
        int h = it->second;
        if (h >= b) continue;                 // not a back edge

        // Conservative: if there is a call inside the loop, don't move anything
        // (the subroutine could modify global variables).
        bool hay_gosub = false;
        for (int j = h+1; j < b; ++j)
            if (code[j].op=="GOSUB") { hay_gosub = true; break; }
        if (hay_gosub) continue;

        // set of names modified inside the loop
        std::set<std::string> mod;
        for (int j = h+1; j < b; ++j) {
            std::string d = destino(code[j]);
            if (!d.empty()) mod.insert(d);
        }

        for (int j = h+1; j < b; ++j) {
            const std::string& op = code[j].op;
            bool hoist = op=="ADD"||op=="SUB"||op=="MUL"||
                         op=="EQ"||op=="NEQ"||op=="LT"||op=="LTE"||
                         op=="GT"||op=="GTE";   // pure and side-effect-free
            if (!hoist) continue;
            std::string d = destino(code[j]);
            if (!es_temp(d)) continue;          // only temporaries (SSA)

            bool invariante = true;
            for (const auto& s : fuentes(code[j]))
                if (!es_literal(s) && mod.count(s)) { invariante = false; break; }
            if (!invariante) continue;

            Instr movida = code[j];
            code.erase(code.begin() + j);
            code.insert(code.begin() + h, movida);  // to the preheader
            return true;
        }
    }
    return false;
}

void licm(std::vector<Instr>& code) {
    int guarda = 0;
    while (licm_un_paso(code) && guarda++ < 100000) {}
}

/* =====================  PASS 2: local CSE  ===================== */
std::string clave_expr(const Instr& in) {
    std::string a = in.arg[0], b = in.arg[1];
    if (in.op=="ADD"||in.op=="MUL"||in.op=="EQ"||in.op=="NEQ")
        if (a > b) std::swap(a, b);            // commutative: normalized order
    return in.op + "#" + a + "#" + b;
}

// true if 'name' is read at any index >= from
bool usado_desde(const std::vector<Instr>& code, const std::string& nombre, int desde) {
    for (int i = desde; i < (int)code.size(); ++i)
        for (const auto& s : fuentes(code[i]))
            if (s == nombre) return true;
    return false;
}

void cse(std::vector<Instr>& code, std::vector<bool>& muerta) {
    int n = code.size();
    // basic block leaders
    std::vector<bool> lider(n, false);
    if (n) lider[0] = true;
    for (int i = 0; i < n; ++i) {
        if (code[i].op=="LABEL") lider[i] = true;
        if (es_control(code[i].op) && i+1 < n) lider[i+1] = true;
    }

    int i = 0;
    while (i < n) {
        int fin = i + 1;
        while (fin < n && !lider[fin]) ++fin;   // bloque [i, fin)

        std::map<std::string,std::string> disponible;  // key -> temporary
        for (int k = i; k < fin; ++k) {
            if (muerta[k]) continue;
            Instr& in = code[k];

            if (es_arit3(in.op) && es_temp(destino(in))) {
                std::string key = clave_expr(in);
                auto it = disponible.find(key);
                if (it != disponible.end()) {
                    std::string prev = it->second;     // already computed
                    std::string d = destino(in);
                    // only deleted if 'd' is not used outside the block
                    if (!usado_desde(code, d, fin)) {
                        for (int m = k+1; m < fin; ++m)        // redirect uses
                            for (int p : pos_fuente(code[m]))
                                if (p < (int)code[m].arg.size() && code[m].arg[p]==d)
                                    code[m].arg[p] = prev;
                        muerta[k] = true;
                        continue;
                    }
                } else {
                    disponible[key] = destino(in);
                }
            }
            // invalidate expressions that depend on the just-written name
            std::string d = destino(in);
            if (!d.empty()) {
                std::vector<std::string> borrar;
                for (auto& par : disponible)
                    if (par.first.find("#"+d+"#") != std::string::npos ||
                        par.first.size() >= d.size()+1 &&
                        par.first.compare(par.first.size()-d.size()-1, d.size()+1, "#"+d) == 0 ||
                        par.second == d)
                        borrar.push_back(par.first);
                for (auto& b : borrar) disponible.erase(b);
            }
        }
        i = fin;
    }
}

/* =====================  PASS 3: DCE  ===================== */
void dce(std::vector<Instr>& code, std::vector<bool>& muerta) {
    bool cambio = true;
    while (cambio) {
        cambio = false;
        std::set<std::string> usados;
        for (int i = 0; i < (int)code.size(); ++i) {
            if (muerta[i]) continue;
            for (const auto& s : fuentes(code[i]))
                usados.insert(s);
        }
        for (int i = 0; i < (int)code.size(); ++i) {
            if (muerta[i]) continue;
            if (!es_def_pura(code[i].op)) continue;   // PARAM_GET/INPUT have side effects
            std::string d = destino(code[i]);
            if (es_temp(d) && !usados.count(d)) {
                muerta[i] = true;
                cambio = true;
            }
        }
    }
}

/* =====================  PASS 4: temporary reuse  ===================== */
void reusar_temporales(std::vector<Instr>& code) {
    int n = code.size();

    // single definition and last use of each temporary
    std::map<std::string,int> def_idx, uso_idx;
    for (int i = 0; i < n; ++i) {
        std::string d = destino(code[i]);
        if (es_temp(d) && !def_idx.count(d)) def_idx[d] = i;
        for (int p : pos_memoria(code[i])) {
            if (p >= (int)code[i].arg.size()) continue;
            const std::string& a = code[i].arg[p];
            if (es_temp(a)) uso_idx[a] = i;            // updated to the last occurrence
        }
    }

    // temporary "crosses" control if there is a jump/label between def and last use
    auto atraviesa = [&](const std::string& t) {
        int d = def_idx[t], u = uso_idx.count(t) ? uso_idx[t] : d;
        for (int k = d+1; k < u; ++k)
            if (es_control(code[k].op)) return true;
        return false;
    };

    // reusable temporaries sorted by their definition
    std::vector<std::string> orden;
    for (auto& par : def_idx)
        if (!atraviesa(par.first)) orden.push_back(par.first);
    std::sort(orden.begin(), orden.end(),
              [&](const std::string& a, const std::string& b){
                  return def_idx[a] < def_idx[b];
              });

    std::map<std::string,std::string> mapa;            // temporary -> cell
    std::vector<std::pair<int,std::string>> activos;   // (last_use, cell)
    std::vector<std::string> libres;
    int siguiente = 0;

    for (const auto& t : orden) {
        int d = def_idx[t];
        // free cells whose temporaries have already expired before this definition
        std::vector<std::pair<int,std::string>> quedan;
        for (auto& a : activos) {
            if (a.first < d) libres.push_back(a.second);
            else quedan.push_back(a);
        }
        activos = quedan;

        std::string celda;
        if (!libres.empty()) { celda = libres.back(); libres.pop_back(); }
        else celda = "r" + std::to_string(siguiente++);

        mapa[t] = celda;
        int u = uso_idx.count(t) ? uso_idx[t] : d;
        activos.push_back({u, celda});
    }

    // rename throughout the code
    for (auto& in : code)
        for (int p : pos_memoria(in)) {
            if (p >= (int)in.arg.size()) continue;
            auto it = mapa.find(in.arg[p]);
            if (it != mapa.end()) in.arg[p] = it->second;
        }
}

/* ---------- cell collection and printing ---------- */
std::vector<std::string> recolectar_celdas(const std::vector<Instr>& code) {
    std::vector<std::string> orden;
    std::set<std::string> visto;
    auto add = [&](const std::string& s) {
        if (!s.empty() && !es_literal(s) && !visto.count(s)) {
            visto.insert(s); orden.push_back(s);
        }
    };
    for (const auto& in : code) {
        add(destino(in));
        for (const auto& s : fuentes(in)) add(s);
    }
    return orden;
}

void imprimir_instr(FILE* out, const Instr& in) {
    const std::string& op = in.op;
    if (op=="LABEL")        fprintf(out, "LABEL %s\n", in.arg[0].c_str());
    else if (op=="GOTO")    fprintf(out, "GOTO %s\n", in.arg[0].c_str());
    else if (op=="GOSUB")   fprintf(out, "GOSUB %s\n", in.arg[0].c_str());
    else if (op=="RETURN")  fprintf(out, "RETURN\n");
    else if (op=="IF")      fprintf(out, "IF %s GOTO %s\n", in.arg[0].c_str(), in.arg[1].c_str());
    else if (op=="IFFALSE") fprintf(out, "IFFALSE %s GOTO %s\n", in.arg[0].c_str(), in.arg[1].c_str());
    else if (op=="PRINT")   fprintf(out, "PRINT %s\n", in.arg[0].c_str());
    else if (op=="INPUT")   fprintf(out, "INPUT %s\n", in.arg[0].c_str());
    else if (op=="PARAM")   fprintf(out, "PARAM %s\n", in.arg[0].c_str());
    else if (op=="PARAM_GET") fprintf(out, "PARAM_GET %s\n", in.arg[0].c_str());
    else if (op=="ASSIGN")  fprintf(out, "ASSIGN %s %s\n", in.arg[0].c_str(), in.arg[1].c_str());
    else                    fprintf(out, "%s %s %s %s\n", op.c_str(),
                                    in.arg[0].c_str(), in.arg[1].c_str(), in.arg[2].c_str());
}

/* ---------- code generator (AST -> IR traversal) ---------- */
struct Generador {
    int contador_temp = 0, contador_label = 0, contador_var = 0;
    bool hay_funciones = false;

    std::vector<Instr> codigo_funciones;
    std::vector<Instr> codigo_main;
    std::vector<Instr>* destino = &codigo_main;

    std::vector<std::map<std::string,std::string>> bloques;
    std::vector<std::pair<std::string,std::string>> ciclos;

    std::string nuevo_temp()  { return "t" + std::to_string(contador_temp++); }
    std::string nueva_etiqueta() { return "L" + std::to_string(contador_label++); }
    std::string nueva_var(const std::string& nom) {
        return "v" + std::to_string(contador_var++) + "_" + nom;
    }

    void emit(const std::string& op, std::vector<std::string> a = {}) {
        destino->push_back(Instr{op, std::move(a)});
    }

    void abrir_bloque()  { bloques.push_back({}); }
    void cerrar_bloque() { bloques.pop_back(); }
    std::string declarar_var(const std::string& nom) {
        std::string v = nueva_var(nom);
        bloques.back()[nom] = v;
        return v;
    }
    std::string resolver(const std::string& nom) {
        for (auto it = bloques.rbegin(); it != bloques.rend(); ++it) {
            auto f = it->find(nom);
            if (f != it->end()) return f->second;
        }
        return nom;
    }
    std::string instr_cmp(OpBinaria op) {
        switch (op) {
            case OpBinaria::EQ:  return "EQ";
            case OpBinaria::NEQ: return "NEQ";
            case OpBinaria::LT:  return "LT";
            case OpBinaria::LE:  return "LTE";
            case OpBinaria::GT:  return "GT";
            case OpBinaria::GE:  return "GTE";
            default:             return "EQ";
        }
    }

    std::string gen_expr(Nodo* nodo);
    void gen_oracion(Nodo* nodo);
    void gen_bloque(const std::vector<NodoPtr>& os) { for (auto& o : os) if (o) gen_oracion(o.get()); }
    void gen_funcion(NodoFuncion* f);
    void gen_programa(NodoPrograma* p);
};

std::string Generador::gen_expr(Nodo* nodo) {
    if (!nodo) return "0";

    if (auto* n = dynamic_cast<NodoEntero*>(nodo))   return std::to_string(n->valor);
    if (auto* n = dynamic_cast<NodoFlotante*>(nodo)) {
        std::ostringstream oss;
        oss << n->valor;
        std::string s = oss.str();
        if (s.find('.') == std::string::npos) s += ".0";  // 4 -> 4.0
        return s;
    }
    if (auto* n = dynamic_cast<NodoLetra*>(nodo)) { std::string s="'"; s+=n->valor; s+="'"; return s; }
    if (auto* n = dynamic_cast<NodoTexto*>(nodo))    return n->valor;
    if (auto* n = dynamic_cast<NodoAlias*>(nodo))    return resolver(n->nombre);

    if (auto* n = dynamic_cast<NodoUnop*>(nodo)) {
        std::string h = gen_expr(n->hijo.get());
        std::string t = nuevo_temp();
        if (n->op == OpUnaria::NEGACION) emit("SUB", {"0", h, t});
        else                             emit("EQ",  {h, "0", t});
        return t;
    }

    if (auto* n = dynamic_cast<NodoBinop*>(nodo)) {
        std::string a = gen_expr(n->izq.get());
        std::string b = gen_expr(n->der.get());
        std::string t = nuevo_temp();
        switch (n->op) {
            case OpBinaria::SUMA:  emit("ADD", {a,b,t}); break;
            case OpBinaria::RESTA: emit("SUB", {a,b,t}); break;
            case OpBinaria::MUL:   emit("MUL", {a,b,t}); break;
            case OpBinaria::MOD:   emit("MOD", {a,b,t}); break;
            case OpBinaria::DIV:
                if (n->tipo_evaluado == Tipo::ENTERO) {
                    std::string r = nuevo_temp(), num = nuevo_temp();
                    emit("MOD", {a,b,r});
                    emit("SUB", {a,r,num});
                    emit("DIV", {num,b,t});
                } else {
                    emit("DIV", {a,b,t});
                }
                break;
            case OpBinaria::AND:   emit("MUL", {a,b,t}); break;
            case OpBinaria::OR: {
                std::string suma = nuevo_temp();
                emit("ADD", {a,b,suma});
                emit("GT",  {suma,"0",t});
                break;
            }
            default: emit(instr_cmp(n->op), {a,b,t}); break;
        }
        return t;
    }

    if (auto* n = dynamic_cast<NodoLlamada*>(nodo)) {
        std::vector<std::string> args;
        for (auto& a : n->args) args.push_back(gen_expr(a.get()));
        for (auto& a : args) emit("PARAM", {a});
        emit("GOSUB", {"f_" + n->nombre});
        std::string t = nuevo_temp();
        emit("ASSIGN", {"__ret", t});
        return t;
    }
    return "0";
}

void Generador::gen_oracion(Nodo* nodo) {
    if (!nodo) return;

    if (auto* n = dynamic_cast<NodoDecl*>(nodo)) {
        std::string v = declarar_var(n->nombre);
        if (n->valor) emit("ASSIGN", {gen_expr(n->valor.get()), v});
        return;
    }
    if (auto* n = dynamic_cast<NodoAsignacion*>(nodo)) {
        emit("ASSIGN", {gen_expr(n->valor.get()), resolver(n->nombre)});
        return;
    }
    if (auto* n = dynamic_cast<NodoMuestra*>(nodo)) { emit("PRINT", {gen_expr(n->valor.get())}); return; }
    if (auto* n = dynamic_cast<NodoLee*>(nodo))     { emit("INPUT", {resolver(n->nombre)}); return; }
    if (auto* n = dynamic_cast<NodoDevuelve*>(nodo)) {
        emit("ASSIGN", {gen_expr(n->valor.get()), "__ret"});
        emit("RETURN");
        return;
    }
    if (auto* n = dynamic_cast<NodoCuando*>(nodo)) {
        std::string cond = gen_expr(n->condicion.get());
        std::string lelse = nueva_etiqueta();
        emit("IFFALSE", {cond, lelse});
        abrir_bloque(); gen_bloque(n->entonces); cerrar_bloque();
        if (!n->sino.empty()) {
            std::string lfin = nueva_etiqueta();
            emit("GOTO", {lfin});
            emit("LABEL", {lelse});
            abrir_bloque(); gen_bloque(n->sino); cerrar_bloque();
            emit("LABEL", {lfin});
        } else {
            emit("LABEL", {lelse});
        }
        return;
    }
    if (auto* n = dynamic_cast<NodoMientras*>(nodo)) {
        std::string lini = nueva_etiqueta(), lfin = nueva_etiqueta();
        emit("LABEL", {lini});
        std::string cond = gen_expr(n->condicion.get());
        emit("IFFALSE", {cond, lfin});
        ciclos.push_back({lini, lfin});
        abrir_bloque(); gen_bloque(n->cuerpo); cerrar_bloque();
        ciclos.pop_back();
        emit("GOTO", {lini});
        emit("LABEL", {lfin});
        return;
    }
    if (dynamic_cast<NodoRompe*>(nodo))   { if (!ciclos.empty()) emit("GOTO", {ciclos.back().second}); return; }
    if (dynamic_cast<NodoContinua*>(nodo)){ if (!ciclos.empty()) emit("GOTO", {ciclos.back().first});  return; }
    if (dynamic_cast<NodoLlamada*>(nodo)) { gen_expr(nodo); return; }
    if (auto* n = dynamic_cast<NodoFuncion*>(nodo)) { gen_funcion(n); return; }
}

void Generador::gen_funcion(NodoFuncion* f) {
    hay_funciones = true;
    auto* previo = destino;
    destino = &codigo_funciones;

    emit("LABEL", {"f_" + f->nombre});
    abrir_bloque();
    std::vector<std::string> params;
    for (auto& p : f->params)
        if (auto* pm = dynamic_cast<NodoParam*>(p.get()))
            params.push_back(declarar_var(pm->nombre));
    for (auto it = params.rbegin(); it != params.rend(); ++it)  // LIFO
        emit("PARAM_GET", {*it});
    gen_bloque(f->cuerpo);
    emit("RETURN");
    cerrar_bloque();
    destino = previo;
}

void Generador::gen_programa(NodoPrograma* p) {
    abrir_bloque();
    destino = &codigo_main;
    for (auto& par : p->parrafos) {
        if (!par) continue;
        if (auto* f = dynamic_cast<NodoFuncion*>(par.get())) gen_funcion(f);
        else gen_oracion(par.get());
    }
    cerrar_bloque();
}

}  // namespace

void generar_codigo(Nodo* raiz, FILE* out) {
    auto* prog = dynamic_cast<NodoPrograma*>(raiz);
    if (!prog) return;

    Generador g;
    g.gen_programa(prog);

    // assemble the code with the subroutine layout
    std::vector<Instr> code;
    if (g.hay_funciones) {
        code.push_back(Instr{"GOTO", {"__main"}});
        for (auto& in : g.codigo_funciones) code.push_back(in);
        code.push_back(Instr{"LABEL", {"__main"}});
    }
    for (auto& in : g.codigo_main) code.push_back(in);

    // optimization passes
    licm(code);
    std::vector<bool> muerta(code.size(), false);
    cse(code, muerta);
    dce(code, muerta);

    // compact (remove dead-marked instructions)
    std::vector<Instr> compacto;
    for (size_t i = 0; i < code.size(); ++i)
        if (!muerta[i]) compacto.push_back(code[i]);

    reusar_temporales(compacto);

    // emit VAR declarations + code
    for (const auto& c : recolectar_celdas(compacto))
        fprintf(out, "VAR %s\n", c.c_str());
    for (const auto& in : compacto)
        imprimir_instr(out, in);
}