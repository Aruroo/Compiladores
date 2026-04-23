#include <stdio.h>

#include "scanner.h"

extern int yylex(void);
extern char *yytext;

int main(void)
{
	int token;

	while((token = yylex()) != TOK_EOF) {
		printf("[%s:%s]\n", scanner_token_name(token), yytext);
	}
	return 0;
}

const char *scanner_token_name(int token) {
    switch (token) {
        case TOK_EOF:               return "TOK_EOF";
        case TOK_ERROR:             return "TOK_ERROR";

        case TOK_KW_HOLA:           return "TOK_KW_HOLA";
        case TOK_KW_ATENTAMENTE:    return "TOK_KW_ATENTAMENTE";
        case TOK_KW_QUERIDO:        return "TOK_KW_QUERIDO";
        case TOK_KW_MUESTRA:        return "TOK_KW_MUESTRA";
        
        case TOK_KW_TEXTO:        return "TOK_KW_TEXTO";
        case TOK_KW_LEE:          return "TOK_KW_LEE";

        case TOK_KW_ENTERO:         return "TOK_KW_ENTERO";
        case TOK_KW_FLOTANTE:       return "TOK_KW_FLOTANTE";
        case TOK_KW_LETRA:          return "TOK_KW_LETRA";
        case TOK_KW_NADA:           return "TOK_KW_NADA";

        case TOK_KW_CUANDO:         return "TOK_KW_CUANDO";
        case TOK_KW_SINO:           return "TOK_KW_SINO";
        case TOK_KW_MIENTRAS:       return "TOK_KW_MIENTRAS";
        case TOK_KW_DEVUELVE:       return "TOK_KW_DEVUELVE";
        case TOK_KW_ROMPE:          return "TOK_KW_ROMPE";
        case TOK_KW_CONTINUA:       return "TOK_KW_CONTINUA";

        case TOK_ALIAS:             return "TOK_ALIAS";
        case TOK_ENTERO_LITERAL:    return "TOK_ENTERO_LITERAL";
        case TOK_FLOTANTE_LITERAL:  return "TOK_FLOTANTE_LITERAL";
        case TOK_TEXTO_LITERAL:    return "TOK_TEXTO_LITERAL";
        case TOK_LETRA_LITERAL:      return "TOK_LETRA_LITERAL";

        case TOK_ASSIGN:            return "TOK_ASSIGN";

        case TOK_EQ:                return "TOK_EQ";
        case TOK_NEQ:               return "TOK_NEQ";
        case TOK_LT:                return "TOK_LT";
        case TOK_LE:                return "TOK_LE";
        case TOK_GT:                return "TOK_GT";
        case TOK_GE:                return "TOK_GE";

        case TOK_AND:               return "TOK_AND";
        case TOK_OR:                return "TOK_OR";
        case TOK_NOT:               return "TOK_NOT";

        case TOK_PLUS:              return "TOK_PLUS";
        case TOK_MINUS:             return "TOK_MINUS";
        case TOK_MUL:               return "TOK_MUL";
        case TOK_DIV:               return "TOK_DIV";
        case TOK_MOD:               return "TOK_MOD";

        case TOK_LPAREN:            return "TOK_LPAREN";
        case TOK_RPAREN:            return "TOK_RPAREN";
        case TOK_LBRACE:            return "TOK_LBRACE";
        case TOK_RBRACE:            return "TOK_RBRACE";
        case TOK_COMMA:             return "TOK_COMMA";

        default:                    return "TOK_DESCONOCIDO";
    }
}