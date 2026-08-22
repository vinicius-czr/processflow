#ifndef TASK_H
#define TASK_H

#define MAX_TASKS 64
#define MAX_TASK_NAME 128
#define MAX_TASK_ARGS 32

typedef struct {
    char name[MAX_TASK_NAME];
    char *argv[MAX_TASK_ARGS];
    int argc;
} Task;

typedef struct {
    Task tasks[MAX_TASKS];
    int count;
} TaskRegistry;

void task_registry_init(TaskRegistry *registry);

int task_register(TaskRegistry *registry, char **tokens, int token_count);

Task *task_find(TaskRegistry *registry, const char *name);

void task_registry_free(TaskRegistry *registry);

#endif