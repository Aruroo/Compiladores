#include <stdio.h>
#include <assert.h>
#include "../src/analyzer.c"
#include "../src/grammar.c"

static bool compute_first_tables(const grammar *g, bool **first_table, bool **nullable, int *epsilon_id);
static grammar *g;

// Helper: checks if terminal 'name' is in FIRST of non-terminal 'nt_id'
static bool in_first(bool *first_table, int nt_id, int t_id, int num_terminals)
{
    return first_table[nt_id * num_terminals + t_id];
}

void test_null_inputs()
{
    bool *ft = NULL, *nl = NULL;
    int ep = -1;

    assert(compute_first_tables(NULL, &ft, &nl, &ep) == false);
    printf("NULL grammar -> false\n");

    assert(compute_first_tables(g, NULL, &nl, &ep) == false);
    printf("NULL first_table -> false\n");

    assert(compute_first_tables(g, &ft, NULL, &ep) == false);
    printf("NULL nullable -> false\n");

    assert(compute_first_tables(g, &ft, &nl, NULL) == false);
    printf("NULL epsilon_id -> false\n");
}

void test_simple_terminal()
{
    // Grammar: S -> a
    // FIRST(S) = {a}
    bool *ft = NULL, *nl = NULL;
    int ep = -1;

    assert(compute_first_tables(g, &ft, &nl, &ep) == true);
    printf("compute_first_tables returned true\n");

    int id_S = 0; // S is first non-terminal
    int id_a = find_terminal_id(g, "a");

    assert(in_first(ft, id_S, id_a, g->num_terminals));
    printf("a in FIRST(S) -> true\n");

    free(ft);
    free(nl);
}

void test_nullable()
{
    // Grammar: S A, Terminals: a b epsilon, S -> A b, A -> a, A -> epsilon
    // nullable[A] should be true, nullable[S] should be false
    const char *grammar_text =
        "Non-terminals: S A\n"
        "Terminals: a b epsilon\n"
        "S -> A b\n"
        "A -> a\n"
        "A -> epsilon\n";

    grammar *g2 = create_grammar(grammar_text);
    assert(g2 != NULL);

    bool *ft = NULL, *nl = NULL;
    int ep = -1;

    assert(compute_first_tables(g2, &ft, &nl, &ep) == true);

    int id_A = 1;
    int id_S = 0;

    assert(nl[id_A] == true);
    printf("nullable[A] -> true\n");

    assert(nl[id_S] == false);
    printf("nullable[S] -> false\n");

    free(ft);
    free(nl);
}

void test_first_propagation()
{
    // Grammar: S -> A b, A -> a, A -> epsilon
    // FIRST(S) = {a, b}  (a from A, b because A is nullable)
    // FIRST(A) = {a}
    const char *grammar_text =
        "Non-terminals: S A\n"
        "Terminals: a b epsilon\n"
        "S -> A b\n"
        "A -> a\n"
        "A -> epsilon\n";

    grammar *g2 = create_grammar(grammar_text);
    assert(g2 != NULL);

    bool *ft = NULL, *nl = NULL;
    int ep = -1;

    assert(compute_first_tables(g2, &ft, &nl, &ep) == true);

    int id_S = 0, id_A = 1;
    int id_a = find_terminal_id(g2, "a");
    int id_b = find_terminal_id(g2, "b");

    assert(in_first(ft, id_A, id_a, g2->num_terminals));
    printf("a in FIRST(A) -> true\n");

    assert(in_first(ft, id_S, id_b, g2->num_terminals));
    printf("b in FIRST(S) -> true\n");

    assert(in_first(ft, id_S, id_a, g2->num_terminals));
    printf("a in FIRST(S) -> true\n");

    free(ft);
    free(nl);
}

void test_epsilon_id()
{
    // Grammar with epsilon: epsilon_id should not be -1
    bool *ft = NULL, *nl = NULL;
    int ep = -1;

    const char *grammar_text =
        "Non-terminals: S\n"
        "Terminals: a epsilon\n"
        "S -> a\n"
        "S -> epsilon\n";

    grammar *g2 = create_grammar(grammar_text);
    assert(g2 != NULL);

    compute_first_tables(g2, &ft, &nl, &ep);
    assert(ep != -1);
    printf("epsilon_id -> %d (not -1)\n", ep);

    free(ft);
    free(nl);
}

int main()
{
    const char *grammar_text =
        "Non-terminals: S\n"
        "Terminals: a\n"
        "S -> a\n";

    g = create_grammar(grammar_text);
    if (g == NULL) {
        printf("[-] Error creating grammar\n");
        return 1;
    }

    const char *separator  = "==========================================\n";
    const char *separator2 = "------------------------------------------\n";

    printf("%s[+] Testing compute_first_tables\n%s", separator, separator);
    print_grammar(g);
    printf("%s", separator2);

    printf("Testing null inputs\n%s", separator2);
    test_null_inputs();

    printf("%sTesting simple terminal\n%s", separator2, separator2);
    test_simple_terminal();

    printf("%sTesting nullable\n%s", separator2, separator2);
    test_nullable();

    printf("%sTesting FIRST propagation\n%s", separator2, separator2);
    test_first_propagation();

    printf("%sTesting epsilon_id\n%s", separator2, separator2);
    test_epsilon_id();

    printf("%s[+] All tests passed\n", separator);
    return 0;
}