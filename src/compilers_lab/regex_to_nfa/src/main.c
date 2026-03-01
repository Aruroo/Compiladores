#include "shunting_yard.h"
#include "nfa.h"
#include "simulate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define MAX 1024

void print_postfix(const char *regex_str)
{
    char with_concat[MAX];
    char postfix[MAX];

    add_explicit_concat(regex_str, with_concat);
    shunting_yard(with_concat, postfix);

    printf("%s\n", postfix);
}

void test_strings_stdin(const char *regex_str)
{
    char with_concat[MAX];
    char postfix[MAX];

    add_explicit_concat(regex_str, with_concat);
    shunting_yard(with_concat, postfix);

    State* start = build_nfa(postfix);

    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin))
    {
        buf[strcspn(buf, "\r\n")] = '\0';

        int result = simulate(start, buf);
        printf("%d", result ? 1 : 0);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int opt;
    char regex_str[MAX];

    while ((opt = getopt(argc, argv, "rt")) != -1)
    {
        switch (opt)
        {
            case 'r':
                if (!fgets(regex_str, sizeof(regex_str), stdin))
                    return 1;

                regex_str[strcspn(regex_str, "\r\n")] = '\0';
                print_postfix(regex_str);
                return 0;

            case 't':
                if (!fgets(regex_str, sizeof(regex_str), stdin))
                    return 1;

                regex_str[strcspn(regex_str, "\r\n")] = '\0';
                test_strings_stdin(regex_str);
                return 0;

            default:
                fprintf(stderr, "Usage: %s -r | -t\n", argv[0]);
                return 1;
        }
    }

    fprintf(stderr, "Usage: %s -r | -t\n", argv[0]);
    return 1;
}