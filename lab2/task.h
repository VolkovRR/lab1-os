/**
 * Representation of a task in the system.
 */

#ifndef TASK_H
#define TASK_H

#define MAX_TASKS 64

// representation of a task
typedef struct task {
    char *name;
    int tid;
    int priority;
    int burst;
    int time_start;
    int time_end;
    int last_call_time;
    int wait_time;
} Task;

#endif
