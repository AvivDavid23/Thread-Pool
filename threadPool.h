#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <unistd.h>
#include "stdlib.h"
#include "osqueue.h"
#include "pthread.h"

#define ERROR "Error in system call\n"

/**
 * info about a task(function pointer and arguments)
 */
typedef struct task{
    void (*function_pointer)(void *);
    void *param;
} task;

typedef struct thread_pool {
    OSQueue* task_queue; // saves tasks
    pthread_t ** threads; // saves threads
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int run_permission; // can threads run tasks?(0/1)
    int add_permission; // can we add tasks to the queue?(0/1)
    int threads_count; //  number of threads
} ThreadPool;

ThreadPool *tpCreate(int numOfThreads);

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param);

#endif
