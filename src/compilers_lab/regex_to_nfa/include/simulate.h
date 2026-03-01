#ifndef SIMULATE_H
#define SIMULATE_H

#include <stdbool.h>
#include "nfa.h"

#define MAX_STATES 1000

typedef struct {
    State* states[MAX_STATES];
    int count;
} StateSet;

char forward(char* input, int* pos);
void epsilon_closure(State* s, StateSet* set, bool visited[MAX_STATES]);
StateSet step(StateSet* current, char c);
bool simulate(State* start, char* input);

#endif