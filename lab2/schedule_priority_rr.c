#include "schedulers.h"
#include "list.h"
#include "stdlib.h"
#include "cpu.h"
#include "sys/param.h"
#include "string.h"

struct node* tasks = NULL;
struct node* first = NULL;

void add(Task* task) {
    insert(&tasks, task);
    if (first == NULL){
        first = tasks;
    } 
}

Task* pick_next() {
    struct node* pointer = tasks->prev;
    struct node* max_task_priority_node = tasks->task->burst ? tasks : tasks->next;
    while (pointer && pointer != tasks) {
        if (pointer->task->burst != 0 && pointer->task->priority >= max_task_priority_node->task->priority) {
            max_task_priority_node = pointer;
        }
        pointer = pointer->prev;
    } 
    if (tasks != tasks->next){
        if (tasks->task->burst == 0){
            tasks->prev->next = tasks->next;
            tasks->next->prev = tasks->prev;
        }
        tasks = max_task_priority_node;
        return tasks->task;
    }
    return NULL;
}

void schedule() {
    first->next = tasks;
    tasks->prev = first;

    while (pick_next() != NULL) {
        run(tasks->task, MIN(tasks->task->burst, TIME_QUANTUM));
        tasks->task->burst -= MIN(tasks->task->burst, TIME_QUANTUM);
    }
}