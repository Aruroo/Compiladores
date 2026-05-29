# CartaCode

A compiler for **CartaCode**, an imperative, statically typed programming language with a formal-letter syntax. Programs open with `hola, name {` and close with `atentamente, author`. Keywords are written in Spanish.

The compiler is built with Flex, Bison and C++. It produces intermediate code for the FIS-25 virtual machine, with four optimization passes applied to the generated code: loop-invariant code motion, common subexpression elimination, dead code elimination, and temporary reuse.

For the full language reference (types, syntax, examples), see [MANUAL.md](MANUAL.md).

## Requirements

- `flex`, `bison`, `g++`, `make`
- Or Docker, using the provided `Dockerfile`

## Build and run with Docker

```bash
docker build -t carta .
docker run -i -v $(pwd)/outputs:/outputs carta < examples/tournament/collatz.carta
```

## Build and run locally

```bash
cd src
make clean && make
./carta ../examples/tournament/collatz.carta
```

The generated intermediate code is written to `outputs/collatz.txt`, ready to run on the FIS-25 virtual machine.


## Output

- **Success**: prints the AST, the symbol table and the implicit promotions; writes the intermediate code to `outputs/<name>.txt`.
- **Failure**: prints a precise error message with line number and skips code generation.

## Examples

Valid programs are in `examples/valid/` and invalid programs for error testing are in `examples/invalid/`. The five algorithms required for the tournament (Collatz, n-th prime, Monte Carlo approximation of Pi, Kaprekar's routine, and the extended Euclidean algorithm) are included as `.carta` files.