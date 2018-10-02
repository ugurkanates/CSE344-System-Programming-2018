#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file threadpool.h
 * @brief Threadpool Header File
 */

#define MAX_THREADS 64
#define MAX_QUEUE 65536
typedef struct provider provider;

typedef struct threadpool_t threadpool_t;


typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
} threadpool_error_t;

typedef enum {
    threadpool_graceful       = 1
} threadpool_destroy_flags_t;


threadpool_t *threadpool_create(int thread_count, int queue_size);
void threadSelect(threadpool_t *pool,char select,float *homework,char *clientName,pthread_mutex_t *lock);
char *getStringFromPool(threadpool_t *pThreadpool);
char *getProviderName(int i,threadpool_t *pThreadpool);
void print_statics(threadpool_t *pool);
void initStruct(threadpool_t *pool);


int threadpool_add(threadpool_t *pool, void (*routine)(void *),
                   void *arg, int flags);

int threadpool_destroy(threadpool_t *pool, int flags);
int readFromFile(char *string,threadpool_t *pool);

#ifdef __cplusplus
}
#endif

#endif /* _THREADPOOL_H_ */