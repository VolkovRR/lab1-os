#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define TIME_QUANTUM 10
#include "task.h"

// add a task to the list 
void add(Task* task);

Task* pick_next_task();

// invoke the scheduler
void schedule();
