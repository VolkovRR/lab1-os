#include "schedulers.h"
#include "list.h"
#include "stdlib.h"
#include "cpu.h"

struct node* tasks = NULL;

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

    struct node* pointer = tasks->next;
    Task* max_task_priority = tasks->task;

    while (pointer != NULL) {
        if (pointer->task->priority > max_task_priority->priority) {
            max_task_priority = pointer->task;
        } 
        pointer = pointer->next;
    }

    return max_task_priority;
}

void schedule() {
    selected_task = pick_next_task();
    while (selected_task != NULL){
        run(selected_task, selected_task->burst);
        selected_task = pick_next_task();
    } 
}
