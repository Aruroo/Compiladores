#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "simulate.h"

char forward(char* input, int* pos) {
    if (input[*pos] == '\0') return '\0';
    return input[(*pos)++];
}

void epsilon_closure(State* s, StateSet* set, bool visited[MAX_STATES]) {
    if (s == NULL || visited[s->id]) return;
    visited[s->id] = true;

    if (s->c == SPLIT) {
        epsilon_closure(s->out1, set, visited);
        epsilon_closure(s->out2, set, visited);
    }
    else if (s->c == EPSILON) {
        epsilon_closure(s->out1, set, visited);
    }
    else {
        set->states[set->count++] = s;
    }
}

StateSet step(StateSet* current, char c) {
    StateSet next;
    next.count = 0;

    for (int i = 0; i < current->count; i++) {
        State* s = current->states[i];
        if (s->c == c) {
            next.states[next.count++] = s->out1;
        }
    }

    return next;
}

bool simulate(State* start, char* input) {
    int pos = 0;

    // Initialize visited and compute epsilon_closure of start
    bool visited[MAX_STATES] = {false};
    StateSet current;
    current.count = 0;
    epsilon_closure(start, &current, visited);

    char c = forward(input, &pos);
    while (c != '\0') {
        // Delta function: move on character c
        StateSet next = step(&current, c);

        // Epsilon closure of the resulting set
        bool visited[MAX_STATES] = {false};
        StateSet expanded;
        expanded.count = 0;
        for (int i = 0; i < next.count; i++) {
            epsilon_closure(next.states[i], &expanded, visited);
        }

        current = expanded;
        c = forward(input, &pos);
    }

    // Check if any state in current is ACCEPT
    for (int i = 0; i < current.count; i++) {
        if (current.states[i]->c == ACCEPT) return true;
    }
    return false;
}