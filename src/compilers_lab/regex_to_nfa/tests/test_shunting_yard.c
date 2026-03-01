#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "shunting_yard.h"

int tests_passed = 0;
int tests_failed = 0;
#define MAX 100

void assert_equal(char* test_name, char* expected, char* actual) {
    if (strcmp(expected, actual) == 0) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("  Expected: %s\n", expected);
        printf("  Got:      %s\n", actual);
        tests_failed++;
    }
}

void run_tests() {
    char output[MAX];

    printf("\n=== Running Explicit Concatenation Tests ===\n\n");

    add_explicit_concat("ab", output);
    assert_equal("Test 01: ab -> a.b", "a.b", output);

    add_explicit_concat("abc", output);
    assert_equal("Test 02: abc -> a.b.c", "a.b.c", output);

    add_explicit_concat("a|b", output);
    assert_equal("Test 03: a|b -> a|b", "a|b", output);

    // single operators

    add_explicit_concat("a*b", output);
    assert_equal("Test 04: a*b -> a*.b", "a*.b", output);

    add_explicit_concat("a+b", output);
    assert_equal("Test 05: a+b -> a+.b", "a+.b", output);

    add_explicit_concat("a?b", output);
    assert_equal("Test 06: a?b -> a?.b", "a?.b", output);

    // parentheses
    add_explicit_concat("a(b|c)", output);
    assert_equal("Test 07: a(b|c) -> a.(b|c)", "a.(b|c)", output);

    add_explicit_concat("(a|b)c", output);
    assert_equal("Test 08: (a|b)c -> (a|b).c", "(a|b).c", output);

    add_explicit_concat("(a)(b)", output);
    assert_equal("Test 09: (a)(b) -> (a).(b)", "(a).(b)", output);

    add_explicit_concat("(a|b)*c", output);
    assert_equal("Test 10: (a|b)*c -> (a|b)*.c", "(a|b)*.c", output);

    // union
    add_explicit_concat("a|bc", output);
    assert_equal("Test 11: a|bc -> a|b.c", "a|b.c", output);

    add_explicit_concat("ab|cd", output);
    assert_equal("Test 12: ab|cd -> a.b|c.d", "a.b|c.d", output);

    // complex
    add_explicit_concat("(a|b)(c|d)", output);
    assert_equal("Test 13: (a|b)(c|d) -> (a|b).(c|d)", "(a|b).(c|d)", output);

    add_explicit_concat("a*b+c?d", output);
    assert_equal("Test 14: a*b+c?d -> a*.b+.c?.d", "a*.b+.c?.d", output);

    // by its own
    add_explicit_concat("a", output);
    assert_equal("Test 15: a -> a", "a", output);

    add_explicit_concat("a*", output);
    assert_equal("Test 16: a* -> a*", "a*", output);

    
    printf("\n=== Running Shunting-Yard Tests ===\n\n");

    shunting_yard("a.b|c*", output);
    assert_equal("Test 1: a.b|c* -> ab.c*|", "ab.c*|", output);

    shunting_yard("(a|b)*", output);
    assert_equal("Test 2: (a|b)* -> ab|*", "ab|*", output);
    
    shunting_yard("h.o.l.a", output);
    assert_equal("Test 3: h.o.l.a -> ho.l.a.", "ho.l.a.", output);

    shunting_yard("a+.b", output);
    assert_equal("Test 4: a+.b -> a+b.", "a+b.", output);
    
    shunting_yard("a?.b", output);
    assert_equal("Test 5: a?.b -> a?b.", "a?b.", output);
    
    shunting_yard("((a|b).c)*", output);
    assert_equal("Test 6: ((a|b).c)* -> ab|c.*", "ab|c.*", output);
    
    shunting_yard("a|b|c", output);
    assert_equal("Test 7: a|b|c -> ab|c|", "ab|c|", output);
    
    shunting_yard("a.b*|c+", output);
    assert_equal("Test 8: a.b*|c+ -> ab*.c+|", "ab*.c+|", output);
    
    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total: %d\n", tests_passed + tests_failed);
}

int main() {
    run_tests();
    return 0;
}