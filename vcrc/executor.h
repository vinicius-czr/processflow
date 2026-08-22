#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "task.h"

// Executa uma única tarefa via fork/exec/waitpid, aguardando seu término.
// Retorna 0 se a tarefa foi executada e -1 se não foi possível criar o processo.
int executor_run_single(Task *task);

#endif