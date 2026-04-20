#ifndef SCANNER_H
#define SCANNER_H

typedef enum ScannerToken {
    TOK_EOF = 0,
    TOK_ERROR = 256,

    /* Palabras clave de estructura del programa */
    TOK_KW_HOLA,
    TOK_KW_ATENTAMENTE,
    TOK_KW_QUERIDO,
    TOK_KW_MUESTRA,

    /* Palabras clave de tipos */
    TOK_KW_ENTERO,
    TOK_KW_FLOTANTE,
    TOK_KW_LETRA,
    TOK_KW_NADA,

    /* Palabras clave de control de flujo */
    TOK_KW_CUANDO,
    TOK_KW_SINO,
    TOK_KW_MIENTRAS,
    TOK_KW_ITERA,
    TOK_KW_DEVUELVE,
    TOK_KW_ROMPE,
    TOK_KW_CONTINUA,

    /* Literales e identificadores */
    TOK_ALIAS,
    TOK_ENTERO_LITERAL,
    TOK_FLOTANTE_LITERAL,
    TOK_STRING_LITERAL,
    TOK_CHAR_LITERAL,

    /* Operadores de asignación */
    TOK_INC,
    TOK_DEC,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,
    TOK_MOD_ASSIGN,
    TOK_ASSIGN,

    /* Operadores de comparación */
    TOK_EQ,
    TOK_NEQ,
    TOK_LT,
    TOK_LE,
    TOK_GT,
    TOK_GE,

    /* Operadores lógicos */
    TOK_AND,
    TOK_OR,
    TOK_NOT,

    /* Operadores aritméticos */
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,

    /* Delimitadores */
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_COMMA,
    TOK_SEMICOLON
} ScannerToken;

const char *scanner_token_name(int token);

#endif // SCANNER_H