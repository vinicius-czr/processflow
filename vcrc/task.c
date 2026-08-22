#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "task.h"

void task_registry_init(TaskRegistry *registry) {
    registry->count = 0;
}

int task_register(TaskRegistry *registry, char **tokens, int token_count) {
    // tokens[0] = "task", tokens[1] = nome, tokens[2] = programa, tokens[3..] = argumentos
    if (token_count < 3) {
        fprintf(stderr, "processflow: uso correto: task <nome> <programa> [argumentos...]\n");
        return -1;
    }

    if (registry->count >= MAX_TASKS) {
        fprintf(stderr, "processflow: número máximo de tarefas atingido (%d)\n", MAX_TASKS);
        return -1;
    }

    const char *name = tokens[1];

    if (task_find(registry, name) != NULL) {
        fprintf(stderr, "processflow: tarefa '%s' já cadastrada\n", name);
        return -1;
    }

    Task *t = &registry->tasks[registry->count];
    strncpy(t->name, name, MAX_TASK_NAME - 1);
    t->name[MAX_TASK_NAME - 1] = '\0';

    // argv[0] = programa (tokens[2]), demais args (tokens[3..token_count-1]), terminado em NULL
    int argi = 0;
    for (int i = 2; i < token_count && argi < MAX_TASK_ARGS - 1; i++) {
        t->argv[argi] = strdup(tokens[i]);
        if (t->argv[argi] == NULL) {
            fprintf(stderr, "processflow: erro de alocação de memória\n");
            return -1;
        }
        argi++;
    }
    t->argv[argi] = NULL;
    t->argc = argi;

    registry->count++;
    printf("processflow: tarefa '%s' cadastrada\n", name);
    return 0;
}

Task *task_find(TaskRegistry *registry, const char *name) {
    for (int i = 0; i < registry->count; i++) {
        if (strcmp(registry->tasks[i].name, name) == 0) {
            return &registry->tasks[i];
        }
    }
    return NULL;
}

void task_registry_free(TaskRegistry *registry) {
    for (int i = 0; i < registry->count; i++) {
        for (int j = 0; j < registry->tasks[i].argc; j++) {
            free(registry->tasks[i].argv[j]);
        }
    }
    registry->count = 0;
}