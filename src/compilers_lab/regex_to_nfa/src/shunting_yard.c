#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "shunting_yard.h"

#define MAX 100

int precedence(char op) {
    if (op == '*' || op == '+' || op == '?') return 3;
    if (op == '.') return 2; // Let's use . for concatenation operation
    if (op == '|') return 1;
    return 0;
}

// Stack operations
char stack[MAX];
int top = -1;

void push(char c) {
    if (top < MAX - 1) {
        stack[++top] = c;
    }
}

char pop() {
    if (top >= 0) {
        return stack[top--];
    }
    return '\0';
}

char peek() {
    if (top >= 0) {
        return stack[top];
    }
    return '\0';
}

bool is_empty() {
    return top == -1;
}

bool is_operator(char c) {
    return (c == '*' || c == '+' || c == '?' || c == '.' || c == '|');
}

// Returns true if a '.' should be inserted afeter this char
bool needs_concat_right(char c) {
    return (c != '(' && c != '|');
}

// Returns true if a '.' should be inserted before this char
bool needs_concat_left(char c) {
    return (c != ')' && c != '|' && c != '*' && c != '+' && c != '?');
}

// Preprocess the input to add explicit concatenation operators
void add_explicit_concat(const char* input, char* output) {
    int out_idx = 0;
    int len = strlen(input);

    for (int i = 0; i < len; i++) {
        char curr = input[i];
        output[out_idx++] = curr;

        // Check if we need to insert a '.' between current char and next
        if (i + 1 < len) {
            char next = input[i + 1];
            if (needs_concat_right(curr) && needs_concat_left(next)) {
                output[out_idx++] = '.';
            }
        }
    }

    output[out_idx] = '\0';
}

void shunting_yard(char* input, char* output) {
    int output_index = 0;
    int i = 0;
    
    while (input[i] != '\0') {
        char c = input[i];
        
        // If it's an operand
        if (!is_operator(c) && c != '(' && c != ')') {
            output[output_index++] = c;
        }
        // If it's '('
        else if (c == '(') {
            push(c);
        }
        // If it's ')'
        else if (c == ')') {
            while (!is_empty() && peek() != '(') {
                output[output_index++] = pop();
            }
            pop(); // Remove the '('
        }
        // If its an operator
        else if (is_operator(c)) {
            while (!is_empty() && peek() != '(' && 
                   precedence(peek()) >= precedence(c)) {
                output[output_index++] = pop();
            }
            push(c);
        }
        
        i++;
    }
    
    // Pop remaining operators
    while (!is_empty()) {
        output[output_index++] = pop();
    }
    
    output[output_index] = '\0';
}