#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "task.h"
#include "job.h"

// Executa uma única tarefa via fork/exec/waitpid, aguardando seu término.
int executor_run_single(Task *task);

// Executa uma lista de tarefas em sequência: só inicia a próxima após a anterior terminar.
void executor_run_sequential(Task **tasks, int count);

// Executa uma lista de tarefas em paralelo: inicia todas antes de aguardar qualquer uma.
void executor_run_parallel(Task **tasks, int count);

// Executa uma lista de tarefas encadeadas via pipe: saída de tasks[i] vira entrada de tasks[i+1].
void executor_run_pipe(Task **tasks, int count);

// Define o diretório de trabalho a ser usado pelas tarefas executadas a partir de agora.
// Retorna 0 em sucesso, -1 se o diretório não existir/não puder ser acessado.
int executor_set_workdir(const char *path);

int executor_start_background(Task *task, JobList *jobs);

#endif