# CartaCode: Syntactic and Semantic Analyzer

Implementation of the syntactic analyzer and AST generator for **CartaCode**, a general-purpose typed imperative programming language with a formal letter-inspired syntax. The language keywords are written in Spanish.

The previous stage (Practice 6) built the syntactic analyzer and the AST generator using Flex and Bison. 

This stage (Practice 7) extends that work with the semantic analysis phase: symbol table management with nested scopes, static verification (declaration of identifiers, uniqueness within a scope), type checking on expressions and statements, and implicit type promotion (`entero` to `flotante`) for mixed arithmetic operations.

## Language Overview

A valid CartaCode program has the following structure:

```carta
hola, program_name {
    /* program body */
    entero x = 5
    muestra(x)
}
atentamente, author_name
```

The language supports five primitive types (`entero`, `flotante`, `letra`, `texto`, `nada`), arithmetic and boolean expressions, conditionals (`cuando`/ `sino`), loops (`mientras`), functions (`querido`), and I/O operations (`muestra`, `lee`).

## Requirements

- Docker (recommended)
- Or: `flex`, `bison`, `g++`, `make`

## Build and Run with Docker

```bash
# Build the image
docker build -t carta .

# Run with a .carta file
docker run -i carta < examples/valid/mientras.carta
```

## Build and Run Locally

```bash
cd src

# Build
make

# Run
./carta < ../examples/valid/mientras.carta

# Clean compiled files
make clean
```

## Output

- **Success**: prints the generated AST
- **Failure**: prints a precise error message with line number
  
## Examples

Valid programs are in `examples/valid/` and invalid programs for error testing are in `examples/invalid/`.