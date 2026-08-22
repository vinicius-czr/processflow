#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "task.h"

#define MAX_LINE 1024

// Processa uma linha já lida (de stdin ou do arquivo workflow).
// Retorna 1 se o programa deve encerrar (comando exit), 0 caso contrário.
static int process_line(const char *line, TaskRegistry *registry) {
    if (strlen(line) == 0) {
        return 0;
    }

    ParsedLine parsed;
    if (parse_line(line, &parsed) != 0) {
        return 0;
    }

    if (parsed.count == 0) {
        free_parsed_line(&parsed);
        return 0;
    }

    int should_exit = 0;

    if (strcmp(parsed.tokens[0], "exit") == 0) {
        should_exit = 1;
    } else if (strcmp(parsed.tokens[0], "task") == 0) {
        task_register(registry, parsed.tokens, parsed.count);
    } else {
        printf("[debug] comando não reconhecido ainda: '%s'\n", parsed.tokens[0]);
    }

    free_parsed_line(&parsed);
    return should_exit;
}

// Modo interativo
static void run_interactive(TaskRegistry *registry) {
    char line[MAX_LINE];

    while (1) {
        printf("processflow> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (process_line(line, registry)) {
            break;
        }
    }
}

// Modo workflow
static void run_workflow(const char *filename, TaskRegistry *registry) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "processflow: não foi possível abrir o arquivo workflow '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        printf("%s\n", line);

        if (process_line(line, registry)) {
            break;
        }

    }

    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "uso: %s [workflowFile]\n", argv[0]);
        return EXIT_FAILURE;
    }

    TaskRegistry registry;
    task_registry_init(&registry);

    if (argc == 2) {
        run_workflow(argv[1], &registry);
    } else {
        run_interactive(&registry);
    }
    
    task_registry_free(&registry);
    return EXIT_SUCCESS;
}
