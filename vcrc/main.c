#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define MAX_LINE 1024

// Modo interativo
static void run_interactive(void) {
    char line[MAX_LINE];

    while (1){
        printf("processflow> ");
        fflush(stdout);

        if(fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if(strlen(line) == 0) {
            continue;
        }

        if(strcmp(line, "exit") == 0) {
            break;
        }

        ParsedLine parsed;
        if (parse_line(line, &parsed) != 0) {
            continue;
        }

        printf("[debug] tokens (%d): ", parsed.count);
        for (int i = 0; i < parsed.count; i++) {
            printf("'%s' ", parsed.tokens[i]);
        }
        printf("\n");

        free_parsed_line(&parsed);
    }
}

// Modo workflow
static void run_workflow(const char *filename) {
    FILE *fp = fopen(filename, "r");

    if(fp == NULL) {
        fprintf(stderr, "processflow: não foi possível abrir o arquivo workflow '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        // Imprime a linha antes de processá-la
        printf("%s\n", line);

        if(strlen(line) == 0) {
            continue;
        }

        if(strcmp(line, "exit") == 0) {
            break;
        }

        printf("[debug] processando: '%s'\n", line);
    }

    fclose(fp);
}

int main(int argc, char *argv[]) {
    if(argc > 2){
        fprintf(stderr, "uso: %s [workflowFile]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if(argc == 2){
        run_workflow(argv[1]);
    }else {
        run_interactive();
    }

    return EXIT_SUCCESS;
}