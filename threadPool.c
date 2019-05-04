

#include "threadPool.h"

// write error to stderr and exit
static void writeErrorAndEXit() {
    write(2, ERROR, sizeof(ERROR));
    _exit(1);
}

/**
 * all created threads will run this function
 * @param param current Thread pool
 * @return NULL
 */
void *executeThread(void *param) {
    ThreadPool *current_pool = (ThreadPool *) param;
    while (current_pool->run_permission) { // run as long we can run tasks from queue
        if (osIsQueueEmpty(current_pool->task_queue) && !current_pool->add_permission) // if empty and cannot add tasks
            break;
        pthread_mutex_lock(&current_pool->mutex);
        if (osIsQueueEmpty(current_pool->task_queue) && current_pool->add_permission)
            pthread_cond_wait(&current_pool->cond, &current_pool->mutex); // wait for signal that the queue is not empty
        task *next = osDequeue(current_pool->task_queue); // get next task from queue
        pthread_mutex_unlock(&current_pool->mutex);
        if (!next)
            continue;
        next->function_pointer(next->param); // run task
        free(next); // free memory allocated for task
    }
    return NULL;
}

ThreadPool *tpCreate(int numOfThreads) {
    ThreadPool *threadPool = (ThreadPool *) malloc(sizeof(ThreadPool));
    if (!threadPool) {
        writeErrorAndEXit();
    } else {
        threadPool->threads_count = numOfThreads;
        pthread_mutex_init(&threadPool->mutex, NULL);
        pthread_cond_init(&threadPool->cond, NULL);
        threadPool->run_permission = 1;
        threadPool->add_permission = 1;
        threadPool->task_queue = osCreateQueue();
        // create n threads
        threadPool->threads = (pthread_t **) malloc(sizeof(pthread_t *) * numOfThreads);
        int i = 0;
        for (; i < numOfThreads; ++i) {
            threadPool->threads[i] = (pthread_t *) malloc(sizeof(pthread_t));
        }
        if (!threadPool->task_queue || !threadPool->threads) {
            writeErrorAndEXit();
        }
        // run al threads
        i = 0;
        for (; i < numOfThreads; ++i) {
            pthread_create(threadPool->threads[i], NULL, executeThread, threadPool);
        }
    }
    return threadPool;
}

void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks) {
    pthread_mutex_lock(&threadPool->mutex);
    if (!threadPool->add_permission) { // make threads that called this function go back if pool is destroyed
        pthread_mutex_unlock(&threadPool->mutex);
        return;
    }
    threadPool->add_permission = 0;
    pthread_mutex_unlock(&threadPool->mutex);
    if (!shouldWaitForTasks)
        threadPool->run_permission = 0;
    pthread_cond_broadcast(&threadPool->cond); // wake up all waiting threads
    int i = 0;
    for (; i < threadPool->threads_count; ++i) {
        pthread_join(*threadPool->threads[i], NULL);
    }
    // free all resources
    pthread_cond_destroy(&threadPool->cond);
    pthread_mutex_destroy(&threadPool->mutex);
    i = 0;
    for (; i < threadPool->threads_count; ++i) {
        free(threadPool->threads[i]);
    }
    free(threadPool->threads);
    task* task;
    while ((task = osDequeue(threadPool->task_queue))) free(task); // free allocated tasks
    osDestroyQueue(threadPool->task_queue);
    free(threadPool);
}

int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param) {
    if (!threadPool->add_permission)
        return -1;
    task *func = (task *) malloc(sizeof(task)); // create task
    func->function_pointer = computeFunc;
    func->param = param;
    pthread_mutex_lock(&threadPool->mutex);
    osEnqueue(threadPool->task_queue, func); // push task to queue
    pthread_mutex_unlock(&threadPool->mutex);
    pthread_cond_broadcast(&threadPool->cond); // notify all threads that the queue is not empty
    return 0;
}