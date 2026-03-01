#include <stdio.h>
#include <stdlib.h>
#include "nfa.h"

static State state_pool[MAX_STATES]; // does not use malloc to avoid leaks
static int state_count = 0;

State* create_state(int c, State* out1, State* out2) {
    if (state_count >= MAX_STATES) {
        fprintf(stderr, "Error: state limit reached\n");
        exit(1);
    }
    State* s = &state_pool[state_count];
    s->c = c;
    s->out1 = out1;
    s->out2 = out2;
    s->id = state_count++;
    return s;
}

Automaton aut_literal(char c) {
    State* accept = create_state(EPSILON, NULL, NULL);
    State* start = create_state(c, accept, NULL);
    return (Automaton){ start, accept };
}

Automaton aut_concat(Automaton a1, Automaton a2) {
    a1.accept->c = EPSILON;
    a1.accept->out1 = a2.start;
    return (Automaton){ a1.start, a2.accept };
}

Automaton aut_union(Automaton a1, Automaton a2) {
    State* accept = create_state(EPSILON, NULL, NULL);
    State* split = create_state(SPLIT, a1.start, a2.start);
    a1.accept->out1 = accept;
    a2.accept->out1 = accept;
    return (Automaton){ split, accept };
}

Automaton aut_kleene(Automaton a) {
    State* accept = create_state(EPSILON, NULL, NULL);
    State* split = create_state(SPLIT, a.start, accept);
    a.accept->out1 = split;
    return (Automaton){ split, accept };
}

Automaton aut_plus(Automaton a) {
    Automaton k = aut_kleene(a);
    return (Automaton){ a.start, k.accept };
}

Automaton aut_question(Automaton a) {
    State* accept = create_state(EPSILON, NULL, NULL);
    State* split = create_state(SPLIT, a.start, accept);
    a.accept->out1 = accept;
    return (Automaton){ split, accept };
}

State* build_nfa(char* postfix) {
    state_count = 0;
    Automaton stack[MAX_STATES];
    int top = -1;

    for (int i = 0; postfix[i] != '\0'; i++) {
        char c = postfix[i];
        switch (c) {
            case '.': {
                Automaton a2 = stack[top--];
                Automaton a1 = stack[top--];
                stack[++top] = aut_concat(a1, a2);
                break;
            }
            case '|': {
                Automaton a2 = stack[top--];
                Automaton a1 = stack[top--];
                stack[++top] = aut_union(a1, a2);
                break;
            }
            case '*': stack[top] = aut_kleene(stack[top]); break;
            case '+': stack[top] = aut_plus(stack[top]); break;
            case '?': stack[top] = aut_question(stack[top]); break;
            default: stack[++top] = aut_literal(c); break;
        }
    }

    State* start = stack[top].start;
    stack[top].accept->c = ACCEPT; // actual finish state
    return start; // only the start as we know now where it ends
}