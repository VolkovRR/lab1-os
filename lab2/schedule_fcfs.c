#include "schedulers.h"
#include "list.h"
#include <stdlib.h>
#include "cpu.h"

struct node* tasks = NULL;

// add a task to the list 
void add(Task* task) {
    insert(&tasks, task);
}

Task* selected_task = NULL;

Task* pick_next_task() {
    if (selected_task != NULL) {
        delete(&tasks, selected_task);
    }
    if (tasks == NULL) {
        return NULL;
    }
    return tasks->task;
}

void schedule() {
    selected_task = pick_next_task();
    while (selected_task != NULL) {
        run(selected_task, selected_task->burst);
        selected_task = pick_next_task();
    } 
}