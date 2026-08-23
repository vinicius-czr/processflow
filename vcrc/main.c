#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "task.h"
#include "job.h"
#include "executor.h"

#define MAX_LINE 1024

static int process_line(const char *line, TaskRegistry *registry, JobList *jobs) {
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
    } else if (strcmp(parsed.tokens[0], "input") == 0) {
        if (parsed.count < 3) {
            fprintf(stderr, "processflow: uso correto: input <tarefa> <arquivo>\n");
        } else {
            task_set_input(registry, parsed.tokens[1], parsed.tokens[2]);
        }
    } else if (strcmp(parsed.tokens[0], "output") == 0) {
        if (parsed.count < 3) {
            fprintf(stderr, "processflow: uso correto: output <tarefa> <arquivo>\n");
        } else {
            task_set_output(registry, parsed.tokens[1], parsed.tokens[2], 0);
        }
    } else if (strcmp(parsed.tokens[0], "append") == 0) {
        if (parsed.count < 3) {
            fprintf(stderr, "processflow: uso correto: append <tarefa> <arquivo>\n");
        } else {
            task_set_output(registry, parsed.tokens[1], parsed.tokens[2], 1);
        }
    } else if (strcmp(parsed.tokens[0], "workdir") == 0) {
        if (parsed.count < 2) {
            fprintf(stderr, "processflow: uso correto: workdir <diretório>\n");
        } else {
            executor_set_workdir(parsed.tokens[1]);
        }
    } else if (strcmp(parsed.tokens[0], "start") == 0) {
        if (parsed.count < 2) {
            fprintf(stderr, "processflow: uso correto: start <tarefa>\n");
        } else {
            Task *t = task_find(registry, parsed.tokens[1]);
            if (t == NULL) {
                fprintf(stderr, "processflow: tarefa '%s' não encontrada\n", parsed.tokens[1]);
            } else {
                executor_start_background(t, jobs);
            }
        }
    } else if (strcmp(parsed.tokens[0], "run") == 0) {
        if (parsed.count < 2) {
            fprintf(stderr, "processflow: uso correto: run <nome> | run sequential <t1> <t2>... | run parallel <t1> <t2>... | run pipe <t1> <t2>...\n");
        } else if (strcmp(parsed.tokens[1], "sequential") == 0 ||
                   strcmp(parsed.tokens[1], "parallel") == 0 ||
                   strcmp(parsed.tokens[1], "pipe") == 0) {
            const char *mode = parsed.tokens[1];
            int n = parsed.count - 2;

            if (n <= 0) {
                fprintf(stderr, "processflow: informe ao menos uma tarefa para run %s\n", mode);
            } else {
                Task *tasks[n];
                int valid = 1;

                for (int i = 0; i < n; i++) {
                    tasks[i] = task_find(registry, parsed.tokens[2 + i]);
                    if (tasks[i] == NULL) {
                        fprintf(stderr, "processflow: tarefa '%s' não encontrada\n", parsed.tokens[2 + i]);
                        valid = 0;
                    }
                }

                if (valid) {
                    if (strcmp(mode, "parallel") == 0) {
                        executor_run_parallel(tasks, n);
                    } else if (strcmp(mode, "pipe") == 0) {
                        executor_run_pipe(tasks, n);
                    } else {
                        executor_run_sequential(tasks, n);
                    }
                }
            }
        } else {
            Task *t = task_find(registry, parsed.tokens[1]);
            if (t == NULL) {
                fprintf(stderr, "processflow: tarefa '%s' não encontrada\n", parsed.tokens[1]);
            } else {
                executor_run_single(t);
            }
        }
    } else {
        printf("[debug] comando não reconhecido ainda: '%s'\n", parsed.tokens[0]);
    }

    free_parsed_line(&parsed);
    return should_exit;
}

static void run_interactive(TaskRegistry *registry, JobList *jobs) {
    char line[MAX_LINE];

    while (1) {
        printf("processflow> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (process_line(line, registry, jobs)) {
            break;
        }
    }
}

static void run_workflow(const char *filename, TaskRegistry *registry, JobList *jobs) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "processflow: não foi possível abrir o arquivo workflow '%s'\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        printf("%s\n", line);

        if (process_line(line, registry, jobs)) {
            break;
        }
    }

    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc > 2) {
        fprintf(stderr, "processflow: número incorreto de argumentos\n");
        fprintf(stderr, "uso: %s [workflowFile]\n", argv[0]);
        return EXIT_FAILURE;
    }

    TaskRegistry registry;
    task_registry_init(&registry);

    JobList jobs;
    job_list_init(&jobs);

    if (argc == 2) {
        run_workflow(argv[1], &registry, &jobs);
    } else {
        run_interactive(&registry, &jobs);
    }

    task_registry_free(&registry);
    return EXIT_SUCCESS;
}