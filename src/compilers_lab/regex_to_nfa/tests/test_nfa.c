#include <stdio.h>
#include <string.h>
#include "nfa.h"

int tests_passed = 0;
int tests_failed = 0;

void assert_state(char* test_name, int expected_c, int expected_id, State* s) {
    if (s != NULL && s->c == expected_c && s->id == expected_id) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("  Expected: c=%d, id=%d\n", expected_c, expected_id);
        printf("  Got:      c=%d, id=%d\n", s ? s->c : -99, s ? s->id : -99);
        tests_failed++;
    }
}

void assert_not_null(char* test_name, State* s) {
    if (s != NULL) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("  Expected: non-null state\n");
        printf("  Got:      NULL\n");
        tests_failed++;
    }
}

void assert_equal_ptr(char* test_name, State* expected, State* actual) {
    if (expected == actual) {
        printf("PASS: %s\n", test_name);
        tests_passed++;
    } else {
        printf("FAIL: %s\n", test_name);
        printf("  Expected id: %d\n", expected ? expected->id : -99);
        printf("  Got id:      %d\n", actual   ? actual->id   : -99);
        tests_failed++;
    }
}

void run_tests() {
    printf("\n=== Running NFA Construction Tests ===\n\n");

    printf("-- aut_literal('a') --\n");

    Automaton a = aut_literal('a');

    assert_not_null("Test 01: literal start is not null", a.start);
    assert_not_null("Test 02: literal accept is not null", a.accept);
    assert_state("Test 03: literal start consumes 'a'", 'a', a.start->id, a.start);
    assert_state("Test 04: literal accept is EPSILON", EPSILON, a.accept->id, a.accept);
    assert_not_null("Test 05: literal start points to accept", a.start->out1);

    printf("\n-- aut_concat('a', 'b') --\n");

    Automaton ca = aut_literal('a');
    Automaton cb = aut_literal('b');
    Automaton c = aut_concat(ca, cb);

    assert_not_null("Test 06: concat start is not null", c.start);
    assert_not_null("Test 07: concat accept is not null", c.accept);
    assert_state("Test 08: concat start consumes 'a'", 'a', c.start->id, c.start);
    assert_state("Test 09: a.accept is EPSILON", EPSILON, ca.accept->id, ca.accept);
    assert_equal_ptr("Test 10: a.accept epsilon goes to b.start", cb.start, ca.accept->out1);
    assert_equal_ptr("Test 11: concat accept is b.accept", cb.accept, c.accept);

    printf("\n-- aut_union('a', 'b') --\n");

    Automaton ua = aut_literal('a');
    Automaton ub = aut_literal('b');
    Automaton u = aut_union(ua, ub);

    assert_state("Test 12: union start is SPLIT", SPLIT, u.start->id, u.start);
    assert_state("Test 13: split out1 leads to 'a'", 'a', ua.start->id, u.start->out1);
    assert_state("Test 14: split out2 leads to 'b'", 'b', ub.start->id, u.start->out2);
    assert_state("Test 15: a's accept leads to shared", EPSILON, u.accept->id, ua.accept->out1);
    assert_state("Test 16: b's accept leads to shared", EPSILON, u.accept->id, ub.accept->out1);

    printf("\n-- aut_kleene('a') --\n");

    Automaton ka = aut_literal('a');
    Automaton k = aut_kleene(ka);

    assert_state("Test 17: kleene start is SPLIT", SPLIT, k.start->id, k.start);
    assert_state("Test 18: split out1 leads to 'a'", 'a', ka.start->id, k.start->out1);
    assert_state("Test 19: split out2 leads to accept", EPSILON, k.accept->id, k.start->out2);
    assert_state("Test 20: a's accept loops to split", SPLIT, k.start->id, ka.accept->out1);

    printf("\n-- aut_plus('a') --\n");

    Automaton pa = aut_literal('a');
    Automaton p = aut_plus(pa);

    assert_state("Test 21: plus start consumes 'a'", 'a', pa.start->id, p.start);
    assert_not_null ("Test 22: plus accept is not null", p.accept);

    printf("\n-- aut_question('a') --\n");

    Automaton qa = aut_literal('a');
    Automaton q = aut_question(qa);

    assert_state("Test 23: question start is SPLIT", SPLIT, q.start->id, q.start);
    assert_state("Test 24: split out1 leads to 'a'",'a', qa.start->id, q.start->out1);
    assert_state("Test 25: split out2 skips to accept", EPSILON, q.accept->id, q.start->out2);

    printf("\n-- build_nfa --\n");

    State* nfa1 = build_nfa("ab.");

    assert_state("Test 26: start consumes 'a'", 'a', nfa1->id, nfa1);

    State* after_a = nfa1->out1;

    assert_state("Test 27: after 'a' is EPSILON", EPSILON, after_a->id, after_a);
    
    State* b_state = after_a->out1;

    assert_state("Test 28: then consumes 'b'", 'b', b_state->id, b_state);
    
    State* final = b_state->out1;
    
    assert_state("Test 29: final state is ACCEPT", ACCEPT, final->id, final);

    State* nfa2 = build_nfa("ab|");

    assert_state("Test 30: union start is SPLIT", SPLIT, nfa2->id, nfa2);
    
    State* left  = nfa2->out1;
    State* right = nfa2->out2;
    
    assert_state("Test 31: left branch consumes 'a'", 'a', left->id, left);
    assert_state("Test 32: right branch consumes 'b'", 'b', right->id, right);

    State* left_accept  = left->out1;
    State* right_accept = right->out1;
    
    assert_equal_ptr("Test 33: both branches share accept", left_accept->out1, right_accept->out1);
    
    printf("\n=== Test Summary ===\n");
    
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
}

int main() {
    run_tests();
    return 0;
}