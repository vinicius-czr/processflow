#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

int parse_line(const char *line, ParsedLine *parsed) {
    parsed->count = 0;
    char *copy = strdup(line);
    if(copy == NULL) {
        fprintf(stderr, "processflow: erro de alocação de memória\n");
        return -1;
    }
    char *saveptr;
    char *token = strtok_r(copy, " \t", &saveptr);

    while(token != NULL && parsed->count < MAX_TOKENS) {
        parsed->tokens[parsed->count] = strdup(token);
        if (parsed->tokens[parsed->count] == NULL) {
            fprintf(stderr, "processflow: erro de alocação de memória\n");
            free(copy);
            free_parsed_line(parsed);
            return -1;
        }
        parsed->count++;
        token = strtok_r(NULL, " \t", &saveptr);
    }
    free(copy);
    return 0;
}

void free_parsed_line(ParsedLine *parsed) {
    for(int i = 0; i < parsed->count; i++) {
        free(parsed->tokens[i]);
        parsed->tokens[i] = NULL;
    }
    parsed->count = 0;
}