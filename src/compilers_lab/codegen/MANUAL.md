# CartaCode Documentation

CartaCode is an imperative, statically typed programming language. Its distinctive feature is that every program is written in the shape of a formal letter: it opens with a greeting, contains the body of instructions, and closes with a signature. The reserved words are in Spanish.

## Paradigm and purpose

The language follows the imperative paradigm with static typing. Its constructs are the classic ones: variable declaration, assignment, conditionals, loops, functions, and input/output. There are no objects, classes, lists, or garbage collection.

The purpose of the language is to be expressive enough for classical algorithms (traversals, counting, modular arithmetic, recursive calls, and so on) while staying narrow enough that all of its phases fit together cleanly: a lexer with Flex, a parser with Bison, an AST in C++, semantic analysis with a block-scoped symbol table, and intermediate code generation targeting the FIS-25 virtual machine.

## Design and program structure

A valid program has the form:

```
hola, program_name {
    /* statements and functions go here */
}
atentamente, author_name
```

The identifier after `hola,` is the recipient (program name) and the one after `atentamente,` is the signature (author). The braces delimit the body. Inside the body two kinds of paragraphs may appear: *statements* (executable sentences) and *functions*. Statements run in the order they appear; functions are declared and then called.

## Data types

Carta has five primitive types. `entero` for signed integers, `flotante` for floating-point real numbers, `letra` for a single character, `texto` for strings, and `nada` for functions that do not return a value. Semantic analysis applies implicit promotion from `entero` to `flotante` whenever they are mixed in an arithmetic operation, an assignment, or a function argument.

## Reserved words

The reserved words of the language are `hola`, `atentamente`, `querido`, `cuando`, `sino`, `mientras`, `devuelve`, `rompe`, `continua`, `muestra`, `lee`, `entero`, `flotante`, `letra`, `texto`, and `nada`. They cannot be used as names for variables or functions.

## Operators

The arithmetic operators are `+`, `-`, `*`, `/`, and `%`. Division `/` between two integers produces an integer (integer division); when at least one operand is a float, it produces a float. The modulo `%` operates only on integers.

The comparison operators are `==`, `!=`, `<`, `<=`, `>`, and `>=`. The logical operators are `&&`, `||`, and `!`. Comparisons and logical operators produce a boolean value that may only appear as the condition of a `cuando` or a `mientras`.

## Declaration and assignment

A variable is declared with its type followed by its name, optionally with an initial value:

```
entero x = 5
flotante y = 3.14
texto saludo = "hello"
letra inicial = 'S'
entero uninitialized
```

Assignment reuses an already declared variable:

```
x = x + 1
```

The language uses lexical block scoping. Every `cuando`, `sino`, `mientras`, and function body introduces a new block, and a variable declared inside shadows another with the same name from an enclosing block. When execution leaves the block, the inner variable is no longer visible.

## Control flow

The conditional uses `cuando` with an optional `sino`:

```
cuando (x > 0) {
    muestra("positive")
} sino {
    muestra("not positive")
}
```

The loop is `mientras`:

```
mientras (i < 10) {
    muestra(i)
    i = i + 1
}
```

Inside a `mientras` one can use `rompe` to break out of the loop or `continua` to jump to the next iteration.

## Functions

Functions are declared with `querido`, followed by the return type, the name, the parameters in parentheses, and the body in braces:

```
querido entero sumar (entero a, entero b) {
    devuelve a + b
}
```

A function with a return type other than `nada` must end with `devuelve`. The call syntax is the usual one: `sumar(3, 5)`. A call can appear inside an expression (its value is used) or as a statement on its own (its value is discarded).

## Input and output

Output is done with `muestra`, which takes an expression and prints it:

```
muestra("Result:")
muestra(resultado)
```

Input is done with `lee`, which takes the name of a previously declared variable and reads a value from the user into it:

```
entero edad
lee(edad)
```

## Comments

Line comments use `//` and block comments use `/* ... */`. Comments are discarded during lexical analysis.

## Examples

### Collatz conjecture

```
hola, collatz {
    entero n = 27
    entero pasos = 0
    mientras (n != 1) {
        cuando (n % 2 == 0) {
            n = n / 2
        } sino {
            n = 3 * n + 1
        }
        pasos = pasos + 1
    }
    muestra("Collatz steps for 27:")
    muestra(pasos)
}
atentamente, sofia
```

### N-th prime number

```
hola, nesimo_primo {
    querido entero es_primo (entero num) {
        cuando (num < 2) {
            devuelve 0
        }
        entero d = 2
        mientras (d * d <= num) {
            cuando (num % d == 0) {
                devuelve 0
            }
            d = d + 1
        }
        devuelve 1
    }
    entero objetivo = 10
    entero contador = 0
    entero candidato = 1
    entero resultado = 0
    mientras (contador < objetivo) {
        candidato = candidato + 1
        cuando (es_primo(candidato) == 1) {
            contador = contador + 1
            resultado = candidato
        }
    }
    muestra("The n-th prime is:")
    muestra(resultado)
}
atentamente, sofia
```

### Extended Euclidean algorithm

```
hola, euclides_extendido {
    entero a = 240
    entero b = 46
    entero viejo_r = a
    entero r = b
    entero viejo_s = 1
    entero s = 0
    entero viejo_t = 0
    entero t = 1
    mientras (r != 0) {
        entero q = viejo_r / r
        entero tmp = viejo_r - q * r
        viejo_r = r
        r = tmp
        tmp = viejo_s - q * s
        viejo_s = s
        s = tmp
        tmp = viejo_t - q * t
        viejo_t = t
        t = tmp
    }
    muestra("GCD:")
    muestra(viejo_r)
    muestra("Coefficient x:")
    muestra(viejo_s)
    muestra("Coefficient y:")
    muestra(viejo_t)
}
atentamente, sofia
```

## Toolchain

The compiler is built with Flex, Bison, and `g++`. From the `src/` folder:

```
make
./carta file.carta
```

The program produces an `outputs/<name>.txt` file with the intermediate code, ready to run on the FIS-25 virtual machine. A batch mode is also supported by passing several files in the same invocation.

## Credits

The FIS-25 virtual machine that runs the intermediate code generated by this compiler was developed by Adrián Martínez Manzo, the lab teaching assistant. It is available at [itch.io](https://amm-gdev.itch.io/fis-25).