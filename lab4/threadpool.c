#include "threadpool.h"

#define QUEUE_SIZE 10
#define NUMBER_OF_THREADS 3

threadpool pool;

// initialize the thread pool
void pool_init(void) {
    pool.threads = (pthread_t*)malloc(sizeof(pthread_t) * NUMBER_OF_THREADS);

    if (pool.threads == NULL) {
        perror("Error allocating memory for threads");
        exit(EXIT_FAILURE);
    }

    initialize_queue(&pool.queue);

    if (pthread_mutex_init(&pool.mutex, NULL) != 0) {
        perror("Error initializing mutex");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&pool.sem, 0, 0) != 0) {
        perror("Error initializing semaphore");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        if (pthread_create(&pool.threads[i], NULL, worker, NULL) != 0) {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
    }
}

// the worker thread in the thread pool
void* worker(void* param) {
    while (1) {
        sem_wait(&pool.sem);

        pthread_mutex_lock(&pool.mutex);
        task* current_task = (task*)dequeue(&pool.queue);
        pthread_mutex_unlock(&pool.mutex);

        if (current_task == NULL) {
            pthread_exit(NULL);
        }

        execute(current_task->function, current_task->data);
    }
}


// Executes the task provided to the thread pool
void execute(void (*somefunction)(void* p), void* p) {
    (*somefunction)(p);
}

// Submits work to the pool.
int pool_submit(void (*somefunction)(void* p), void* p) {
    task* t = (task*)malloc(sizeof(task));
    t->function = somefunction;
    t->data = p;

    pthread_mutex_lock(&pool.mutex);
    if (enqueue(&pool.queue, t) != 0) {
        return 1;
    }
    pthread_mutex_unlock(&pool.mutex);

    sem_post(&pool.sem);

    return 0;
}



// shutdown the thread pool
void pool_shutdown(void) {

    while (pool.queue.size > 0) { }

    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        sem_post(&pool.sem);
    }

    for (int i = 0; i < NUMBER_OF_THREADS; ++i) {
        pthread_join(pool.threads[i], NULL);
    }

    free(pool.threads);

    pthread_mutex_destroy(&pool.mutex);
    sem_destroy(&pool.sem);
}
