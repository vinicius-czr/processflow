#ifndef TASK_H
#define TASK_H

#define MAX_TASKS 64
#define MAX_TASK_NAME 128
#define MAX_TASK_ARGS 32

typedef struct {
    char name[MAX_TASK_NAME];
    char *argv[MAX_TASK_ARGS]; 
    int argc;
    char input_file[256];
    char output_file[256];
    int append_output;      // 1 = usar O_APPEND, 0 = truncar (sobrescrever)
} Task;

typedef struct {
    Task tasks[MAX_TASKS];
    int count;
} TaskRegistry;

void task_registry_init(TaskRegistry *registry);

int task_register(TaskRegistry *registry, char **tokens, int token_count);

Task *task_find(TaskRegistry *registry, const char *name);

// Configura o arquivo de entrada de uma tarefa já cadastrada. Retorna 0 em sucesso, -1 se a tarefa não existir.
int task_set_input(TaskRegistry *registry, const char *name, const char *filename);

// Configura o arquivo de saída de uma tarefa já cadastrada (append = 0 sobrescreve, 1 acrescenta).
int task_set_output(TaskRegistry *registry, const char *name, const char *filename, int append);

void task_registry_free(TaskRegistry *registry);

#endif