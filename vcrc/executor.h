#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "task.h"

// Executa uma única tarefa via fork/exec/waitpid, aguardando seu término.
int executor_run_single(Task *task);

// Executa uma lista de tarefas em sequência: só inicia a próxima após a anterior terminar.
void executor_run_sequential(Task **tasks, int count);

// Executa uma lista de tarefas em paralelo: inicia todas antes de aguardar qualquer uma.
void executor_run_parallel(Task **tasks, int count);

#endif