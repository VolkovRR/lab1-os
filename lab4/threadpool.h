#pragma once

#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>

// function prototypes
void execute(void (*somefunction)(void* p), void* p);
int pool_submit(void (*somefunction)(void* p), void* p);
void* worker(void* param);
void pool_init(void);
void pool_shutdown(void);

// this represents work that has to be completed by a thread in the pool
typedef struct {
    void (*function)(void* p);
    void* data;
} task;

typedef struct {
    pthread_mutex_t mutex;
    sem_t sem;
    pthread_t* threads;
    queue_t queue;
} threadpool;