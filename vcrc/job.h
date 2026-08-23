#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

#define MAX_JOBS 64

typedef enum {
    JOB_RUNNING,
    JOB_DONE
} JobStatus;

typedef struct {
    int job_id;
    pid_t pid;
    char task_name[128];
    JobStatus status;
    int exit_code; // válido só quando status == JOB_DONE
} Job;

typedef struct {
    Job jobs[MAX_JOBS];
    int count;
    int next_job_id;
} JobList;

void job_list_init(JobList *list);

// Adiciona um novo job em execução. Retorna o job_id atribuído, ou -1 se a lista estiver cheia.
int job_add(JobList *list, pid_t pid, const char *task_name);

// Busca um job pelo job_id. Retorna ponteiro para o Job ou NULL se não encontrado.
Job *job_find(JobList *list, int job_id);

// Verifica (sem bloquear) se algum job em execução já terminou, atualizando seu status.
void job_check_all(JobList *list);

#endif