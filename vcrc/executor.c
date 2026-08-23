#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "executor.h"

#define WORKDIR_MAX 512
static char current_workdir[WORKDIR_MAX] = ""; // "" = usa o diretório atual do processo (sem troca)

int executor_set_workdir(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "processflow: diretório '%s' não existe\n", path);
        return -1;
    }

    strncpy(current_workdir, path, WORKDIR_MAX - 1);
    current_workdir[WORKDIR_MAX - 1] = '\0';
    return 0;
}

// Aplica o workdir configurado no processo filho, antes do execvp.
static void apply_workdir(void) {
    if (current_workdir[0] != '\0') {
        if (chdir(current_workdir) != 0) {
            fprintf(stderr, "processflow: não foi possível acessar o diretório '%s'\n", current_workdir);
            _exit(EXIT_FAILURE);
        }
    }
}

// Aplica os redirecionamentos de entrada/saída configurados na tarefa.
// Retorna 0 em sucesso, -1 se algum arquivo não pôde ser aberto.
static int apply_redirections(Task *task) {
    if (task->input_file[0] != '\0') {
        int fd = open(task->input_file, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "processflow: não foi possível abrir o arquivo de entrada '%s'\n", task->input_file);
            return -1;
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (task->output_file[0] != '\0') {
        int flags = O_WRONLY | O_CREAT | (task->append_output ? O_APPEND : O_TRUNC);
        int fd = open(task->output_file, flags, 0644);
        if (fd < 0) {
            fprintf(stderr, "processflow: não foi possível abrir o arquivo de saída '%s'\n", task->output_file);
            return -1;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    return 0;
}

int executor_run_single(Task *task) {
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "processflow: falha ao criar processo para a tarefa '%s'\n", task->name);
        return -1;
    }

    if (pid == 0) {
        apply_workdir();

        if (apply_redirections(task) != 0) {
            _exit(EXIT_FAILURE);
        }

        execvp(task->argv[0], task->argv);

        fprintf(stderr, "processflow: não foi possível executar o programa '%s'\n", task->argv[0]);
        _exit(EXIT_FAILURE);
    }

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

    for (int i = 0; i < count; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            fprintf(stderr, "processflow: falha ao criar processo para a tarefa '%s'\n", tasks[i]->name);
            pids[i] = -1;
            continue;
        }

        if (pid == 0) {
            apply_workdir();

            execvp(tasks[i]->argv[0], tasks[i]->argv);
            fprintf(stderr, "processflow: não foi possível executar o programa '%s'\n", tasks[i]->argv[0]);
            _exit(EXIT_FAILURE);
        }

        pids[i] = pid;
    }

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

void executor_run_pipe(Task **tasks, int count) {
    int num_pipes = count - 1;
    int pipefds[num_pipes][2];

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
            apply_workdir();

            if (i > 0) {
                dup2(pipefds[i - 1][0], STDIN_FILENO);
            }
            if (i < num_pipes) {
                dup2(pipefds[i][1], STDOUT_FILENO);
            }

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

    for (int i = 0; i < num_pipes; i++) {
        close(pipefds[i][0]);
        close(pipefds[i][1]);
    }

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