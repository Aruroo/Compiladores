#ifndef NFA_H
#define NFA_H

#define MAX_STATES 1000
#define EPSILON     0
#define SPLIT      -1
#define ACCEPT     -2

typedef struct State {
    int c;
    struct State* out1;
    struct State* out2;
    int id; // for debugging we set id to the order of creation
} State;

typedef struct {
    State* start;
    State* accept;
} Automaton;

State* create_state(int c, State* out1, State* out2);

Automaton aut_literal (char c);
Automaton aut_concat  (Automaton a1, Automaton a2);
Automaton aut_union   (Automaton a1, Automaton a2);
Automaton aut_kleene  (Automaton a);
Automaton aut_plus    (Automaton a);
Automaton aut_question(Automaton a);

State* build_nfa(char* postfix);

#endif