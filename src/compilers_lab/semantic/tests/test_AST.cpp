#include "../src/AST.h"

int main() {
    // Simulates: 
    // hola, mi_programa {
    // entero x = 5 + 3
    // }
    // atentamente, ssofiss

    // Create the node for the integer literal 5
    auto cinco = std::make_unique<NodoEntero>();
    cinco->valor = 5;
    cinco->linea = 1;

    // Create the node for the integer literal 3
    auto tres = std::make_unique<NodoEntero>();
    tres->valor = 3;
    tres->linea = 1;

    // Create the node for the binary operation 5 + 3
    // izq and der are moved, so cinco and tres are no longer valid after this
    auto suma = std::make_unique<NodoBinop>();
    suma->op = OpBinaria::SUMA;
    suma->izq = std::move(cinco);
    suma->der = std::move(tres);
    suma->linea = 1;

    // Create the node for the declaration: entero x = 5 + 3
    // valor is moved, so suma is no longer valid after this
    auto decl = std::make_unique<NodoDecl>();
    decl->tipo = Tipo::ENTERO;
    decl->nombre = "x";
    decl->valor = std::move(suma);
    decl->linea = 1;

    // Create the root program node and add the declaration as a paragraph
    // parrafos takes ownership of decl via move
    auto programa = std::make_unique<NodoPrograma>();
    programa->destinatario = "mi_programa";
    programa->firma = "ssofiss";
    programa->parrafos.push_back(std::move(decl));

    // Print the ast starting at indentation level 0
    programa->imprimir(0);

    return 0;
}