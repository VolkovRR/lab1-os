/**
 * Driver.c
 *
 * Schedule is in the format
 *
 *  [name] [priority] [CPU burst]
 */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "task.h"
#include "list.h"
#include "schedulers.h"

#define SIZE 10000

int main(int argc, char *argv[]) {
    FILE *in;
    char *temp;
    char task[SIZE];

    char *name;
    int priority;
    int burst;

    in = fopen(argv[1],"r");

    Task* tasks_array[MAX_TASKS];
    int tasks_count = 0;
    struct node* temp_list;
    while (fgets(task,SIZE,in) != NULL) {
        temp = strdup(task);
        name = strsep(&temp,",");
        priority = atoi(strsep(&temp,","));
        burst = atoi(strsep(&temp,","));

        // add the task to the scheduler's list of tasks
        add_task(&temp_list, name, priority, burst);
        tasks_array[tasks_count] = temp_list->task;
        tasks_count++;

        free(temp);
    }

    while (temp_list != NULL) {
        add(temp_list->task);
        delete(&temp_list, temp_list->task);
    }

    fclose(in);

    // invoke the scheduler
    schedule();

    float average_turnaround_time = 0;
    float average_wait_time = 0;
    float average_response_time = 0;
    for(int x = 0; x < tasks_count; x++) {
        average_turnaround_time += tasks_array[x]->time_end - tasks_array[x]->time_start;
        average_wait_time += tasks_array[x]->wait_time;
        average_response_time += tasks_array[x]->time_start;
    }
    average_turnaround_time /= tasks_count;
    average_wait_time /= tasks_count;
    average_response_time /= tasks_count;
        
    printf("\nAverage turnaround time: %f", average_turnaround_time);
    printf("\nAverage wait time: %f", average_wait_time);
    printf("\nAverage response time: %f\n", average_response_time);

    return 0;
}
