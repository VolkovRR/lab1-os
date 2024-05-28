#include "schedulers.h"
#include "list.h"
#include "stdlib.h"
#include "cpu.h"
#include "sys/param.h"

struct node* tasks = NULL;
struct node* first = NULL;

void add(Task* task) {
    insert(&tasks, task);
    if (first == NULL){
        first = tasks;
    } 
}

Task* pick_next() {
    if (tasks != tasks->next){
        if (tasks->task->burst == 0){
            tasks->prev->next = tasks->next;
            tasks->next->prev = tasks->prev;
        }
        tasks = tasks->next;
        return tasks->task;
    }
    return NULL;
}

void schedule() {
    first->next = tasks;
    tasks->prev = first;
    tasks = first;

    while (pick_next() != NULL) {
        run(tasks->task, MIN(tasks->task->burst, TIME_QUANTUM));
        tasks->task->burst -= MIN(tasks->task->burst, TIME_QUANTUM);
    }
}
