#include <stdio.h>
#include <assert.h>
#include "../src/analyzer.c"
#include "../src/grammar.c"

int find_terminal_id(const grammar *g, const char *name);

static grammar *g;

void test_existent_terminals()
{
    int id_a = find_terminal_id(g, "a");
    int id_b = find_terminal_id(g, "b");
    int id_c = find_terminal_id(g, "c");

    printf("a -> %d\n", id_a);
    printf("b -> %d\n", id_b);
    printf("c -> %d\n", id_c);

    assert(id_a != -1);
    assert(id_b != -1);
    assert(id_c != -1);
}

void test_inexistent_terminal()
{
    int id_x = find_terminal_id(g, "x");
    printf("x -> %d\n", id_x);
    assert(id_x == -1);
}

void test_null_inputs()
{
    assert(find_terminal_id(NULL, "a") == -1);
    printf("NULL grammar -> -1\n");

    assert(find_terminal_id(g, NULL) == -1);
    printf("NULL name -> -1\n");
}

int main()
{
    const char *grammar_text =
        "Non-terminals: S A\n"
        "Terminals: a b c\n"
        "S -> A a\n"
        "S -> c\n"
        "A -> b\n";

    g = create_grammar(grammar_text);
    if (g == NULL) {
        printf("[-] Error creating grammar\n");
        return 1;
    }

    const char *line = "[+] Testing find_terminal_id with grammar\n";
    const char *separator = "==========================================\n";
    const char *separator2 = "------------------------------------------\n";
    printf("%s%s%s", separator, line, separator);
    printf("%sGrammar\n%s", separator2, separator2);
    print_grammar(g);
    printf("%s", separator2);
    
    printf("Testing existent terminals\n%s", separator2);
    test_existent_terminals();
    printf("%sTesting inexistent terminal\n%s", separator2, separator2);
    test_inexistent_terminal();
    printf("%sTesting null inputs\n%s", separator2, separator2);
    test_null_inputs();

    printf("%s", separator);

    printf("[+] All tests passed\n");
    return 0;
}