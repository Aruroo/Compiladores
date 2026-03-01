#ifndef SHUNTING_YARD_H
#define SHUNTING_YARD_H

int precedence(char op);
void shunting_yard(char* input, char* output);
void add_explicit_concat(char* input, char* output);

#endif