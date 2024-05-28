/**
 * list data structure containing the tasks in the system
 */

#include "task.h"

struct node {
    Task *task;
    struct node *next;
    struct node *prev;
};

// insert and delete operations.
void insert(struct node **head, Task *task);
struct node* delete(struct node **head, Task *task);
void traverse(struct node *head);
struct node* add_task(struct node **head, char* name, int priority, int burst);
