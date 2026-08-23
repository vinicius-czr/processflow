#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "executor.h"

int executor_run_single(Task *task) {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "processflow: falha ao criar processo para a tarefa '%s'\n", task->name);
        return -1;
    }

    if (pid == 0) {
        // Processo filho: substitui a imagem do processo pelo programa da tarefa.
        execvp(task->argv[0], task->argv);

        // Só chega aqui se execvp falhar (programa não existe / não pode ser executado).
        fprintf(stderr, "processflow: não foi possível executar o programa '%s'\n", task->argv[0]);
        _exit(EXIT_FAILURE); // _exit evita flush duplicado de buffers herdados do pai
    }

    // Processo pai: espera o filho terminar.
    int status;
    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "processflow: erro ao aguardar a tarefa '%s'\n", task->name);
        return -1;
    }

    if (WIFEXITED(status)) {
        printf("processflow: tarefa '%s' finalizada com código %d\n", task->name, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("processflow: tarefa '%s' finalizada pelo sinal %d\n", task->name, WTERMSIG(status));
    }

    return 0;
}

void executor_run_sequential(Task **tasks, int count) {
    for (int i = 0; i < count; i++) {
        executor_run_single(tasks[i]);
    }
}

void executor_run_parallel(Task **tasks, int count) {
    pid_t pids[count];

    // inicia todas as tarefas (fork + exec) sem esperar nenhuma.
    for (int i = 0; i < count; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "processflow: falha ao criar processo para a tarefa '%s'\n", tasks[i]->name);
            pids[i] = -1;
            continue;
        }

        if (pid == 0) {
            execvp(tasks[i]->argv[0], tasks[i]->argv);
            fprintf(stderr, "processflow: não foi possível executar o programa '%s'\n", tasks[i]->argv[0]);
            _exit(EXIT_FAILURE);
        }

        pids[i] = pid;
    }

    // aguarda todas terminarem (em qualquer ordem que finalizem).
    for (int i = 0; i < count; i++) {
        if (pids[i] < 0) {
            continue; // fork já tinha falhado, nada a esperar
        }

        int status;
        if (waitpid(pids[i], &status, 0) < 0) {
            fprintf(stderr, "processflow: erro ao aguardar a tarefa '%s'\n", tasks[i]->name);
            continue;
        }

        if (WIFEXITED(status)) {
            printf("processflow: tarefa '%s' finalizada com código %d\n", tasks[i]->name, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("processflow: tarefa '%s' finalizada pelo sinal %d\n", tasks[i]->name, WTERMSIG(status));
        }
    }
}

void executor_run_pipe(Task **tasks, int count) {
    int num_pipes = count - 1;
    int pipefds[num_pipes][2];

    // Cria todos os pipes antes de qualquer fork.
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds[i]) < 0) {
            fprintf(stderr, "processflow: falha ao criar pipe\n");
            return;
        }
    }

    pid_t pids[count];

    for (int i = 0; i < count; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "processflow: falha ao criar processo para a tarefa '%s'\n", tasks[i]->name);
            pids[i] = -1;
            continue;
        }

        if (pid == 0) {
            // Se não é a primeira tarefa, lê do pipe anterior em vez do stdin.
            if (i > 0) {
                dup2(pipefds[i - 1][0], STDIN_FILENO);
            }
            // Se não é a última tarefa, escreve no próximo pipe em vez do stdout.
            if (i < num_pipes) {
                dup2(pipefds[i][1], STDOUT_FILENO);
            }

            // Fecha todas as extremidades de pipe no filho (já duplicadas onde precisava).
            for (int j = 0; j < num_pipes; j++) {
                close(pipefds[j][0]);
                close(pipefds[j][1]);
            }

            execvp(tasks[i]->argv[0], tasks[i]->argv);
            fprintf(stderr, "processflow: não foi possível executar o programa '%s'\n", tasks[i]->argv[0]);
            _exit(EXIT_FAILURE);
        }

        pids[i] = pid;
    }

    // O processo pai fecha todas as extremidades de pipe (ele não lê nem escreve nelas).
    for (int i = 0; i < num_pipes; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

    // Aguarda todos os processos filhos.
    for (int i = 0; i < count; i++) {
        if (pids[i] < 0) {
            continue;
        }

        int status;
        if (waitpid(pids[i], &status, 0) < 0) {
            fprintf(stderr, "processflow: erro ao aguardar a tarefa '%s'\n", tasks[i]->name);
            continue;
        }

        if (WIFEXITED(status)) {
            printf("processflow: tarefa '%s' finalizada com código %d\n", tasks[i]->name, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("processflow: tarefa '%s' finalizada pelo sinal %d\n", tasks[i]->name, WTERMSIG(status));
        }
    }
}