#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include "job.h"

void job_list_init(JobList *list) {
    list->count = 0;
    list->next_job_id = 1;
}

int job_add(JobList *list, pid_t pid, const char *task_name) {
    if (list->count >= MAX_JOBS) {
        fprintf(stderr, "processflow: número máximo de jobs em background atingido (%d)\n", MAX_JOBS);
        return -1;
    }

    Job *j = &list->jobs[list->count];
    j->job_id = list->next_job_id++;
    j->pid = pid;
    strncpy(j->task_name, task_name, sizeof(j->task_name) - 1);
    j->task_name[sizeof(j->task_name) - 1] = '\0';
    j->status = JOB_RUNNING;
    j->exit_code = -1;

    list->count++;
    return j->job_id;
}

Job *job_find(JobList *list, int job_id) {
    for (int i = 0; i < list->count; i++) {
        if (list->jobs[i].job_id == job_id) {
            return &list->jobs[i];
        }
    }
    return NULL;
}

void job_check_all(JobList *list) {
    for (int i = 0; i < list->count; i++) {
        Job *j = &list->jobs[i];
        if (j->status == JOB_RUNNING) {
            int status;
            pid_t result = waitpid(j->pid, &status, WNOHANG);
            if (result == j->pid) {
                j->status = JOB_DONE;
                j->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
            }
        }
    }
}