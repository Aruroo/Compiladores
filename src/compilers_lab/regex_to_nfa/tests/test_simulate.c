#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "shunting_yard.h"
#include "nfa.h"
#include "simulate.h"

int tests_passed = 0;
int tests_failed = 0;

#define MAX 200

void assert_match(char* test_name, bool expected, char* regex, char* input) {
    char with_concat[MAX];
    char postfix[MAX];

    add_explicit_concat(regex, with_concat);
    shunting_yard(with_concat, postfix);
    
    State* nfa = build_nfa(postfix);
    bool result = simulate(nfa, input);

    if (result == expected) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("  Regex:    %s\n", regex);
        printf("  Input:    %s\n", input);
        printf("  Expected: %s\n", expected ? "match" : "no match");
        printf("  Got:      %s\n", result ? "match" : "no match");
        tests_failed++;
    }
}

void run_tests() {
    printf("\n=== Running Simulation Tests ===\n\n");

    printf("-- Literals --\n");

    assert_match("Test 01: 'a' matches 'a'", true, "a", "a");
    assert_match("Test 02: 'a' rejects 'b'", false, "a", "b");
    assert_match("Test 03: 'a' rejects empty", false, "a", "");

    printf("\n-- Concatenation --\n");

    assert_match("Test 04: 'ab' matches 'ab'", true, "ab", "ab");
    assert_match("Test 05: 'ab' rejects 'a'", false, "ab", "a");
    assert_match("Test 06: 'ab' rejects 'abc'", false, "ab", "abc");

    printf("\n-- Union --\n");

    assert_match("Test 07: 'a|b' matches 'a'", true, "a|b", "a");
    assert_match("Test 08: 'a|b' matches 'b'", true, "a|b", "b");
    assert_match("Test 09: 'a|b' rejects 'c'", false, "a|b", "c");

    printf("\n-- Kleene (*) --\n");

    assert_match("Test 10: 'a*' matches empty", true, "a*", "");
    assert_match("Test 11: 'a*' matches 'a'", true, "a*", "a");
    assert_match("Test 12: 'a*' matches 'aaa'", true, "a*", "aaa");
    assert_match("Test 13: 'a*' rejects 'b'", false, "a*", "b");

    printf("\n-- Plus (+) --\n");

    assert_match("Test 14: 'a+' rejects empty", false, "a+", "");
    assert_match("Test 15: 'a+' matches 'a'", true, "a+", "a");
    assert_match("Test 16: 'a+' matches 'aaa'", true, "a+", "aaa");

    printf("\n-- Question (?) --\n");

    assert_match("Test 17: 'a?' matches empty", true, "a?", "");
    assert_match("Test 18: 'a?' matches 'a'", true, "a?", "a");
    assert_match("Test 19: 'a?' rejects 'aa'", false, "a?", "aa");

    printf("\n-- Combined --\n");

    assert_match("Test 20: 'a|bc' matches 'a'", true, "a|bc", "a");
    assert_match("Test 21: 'a|bc' matches 'bc'", true, "a|bc", "bc");
    assert_match("Test 22: 'a|bc' rejects 'b'", false, "a|bc", "b");
    assert_match("Test 23: '(a|b)*' matches empty", true, "(a|b)*","");
    assert_match("Test 24: '(a|b)*' matches 'abba'", true, "(a|b)*","abba");
    assert_match("Test 25: '(a|b)*' rejects 'abc'", false, "(a|b)*","abc");
    assert_match("Test 26: 'a+b?' matches 'a'", true, "a+b?", "a");
    assert_match("Test 27: 'a+b?' matches 'aaab'", true, "a+b?", "aaab");
    assert_match("Test 28: 'a+b?' rejects empty", false, "a+b?", "");
    assert_match("Test 29: '(ab)*' matches 'abab'", true, "(ab)*", "abab");
    assert_match("Test 30: '(ab)*' rejects 'aba'", false, "(ab)*", "aba");
    assert_match("Test 31: '(a|b)c' matches 'ac'", true, "(a|b)c", "ac");
    assert_match("Test 32: '(a|b)c' matches 'bc'", true, "(a|b)c", "bc");
    assert_match("Test 33: '(a|b)c' rejects 'abc'", false, "(a|b)c", "abc");
    assert_match("Test 34: 'a|aa' matches 'aa'", true, "a|aa", "aa");

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
}

int main() {
    run_tests();
    return 0;
}