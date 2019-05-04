# Thread-Pool
- A thread pool that can execute up to N tasks at the same time
- Works only with pthreads

------------
### Compile:
`gcc *.c *.h -lpthread`

------------

### API:
| Function  | Description  |
| ------------ | ------------ |
| __ThreadPool *tpCreate(int numOfThreads)__  |  Create and return a pointer to a new pool, with `numOfThreads` working threads|
| **__void tpDestroy(ThreadPool *threadPool, int shouldWaitForTasks)__**  |  Destroy the pool and free al resources. if  `shouldWaitForTasks` is not 0, the pool will wait for all the tasks in the queue to be finished. otherwise it will wait for current running tasks to be finish. You won't be able to insert tasks to the queue after this function is called |
|__int tpInsertTask(ThreadPool *threadPool, void (*computeFunc)(void *), void *param)__ | Insert a new task to the thread pool. returns 0 on success |

