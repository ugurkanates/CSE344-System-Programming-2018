#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "threadpool.h"
typedef struct provider *ptr;

pthread_mutex_t *lock2;
typedef enum {
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} threadpool_shutdown_t;

/**
 *  @struct threadpool_task
 *  @brief the work struct
 *
 *  @var function Pointer to the function that will perform the task.
 *  @var argument Argument to be passed to the function.
 */

typedef struct {
    void (*function)(void *);
    void *argument;
} threadpool_task_t;

/**
 *  @struct threadpool
 *  @brief The threadpool struct
 *
 *  @var notify       Condition variable to notify worker threads.
 *  @var threads      Array containing worker threads ID.
 *  @var thread_count Number of threads
 *  @var queue        Array containing the task queue.
 *  @var queue_size   Size of the task queue.
 *  @var head         Index of the first element.
 *  @var tail         Index of the next element.
 *  @var count        Number of pending tasks
 *  @var shutdown     Flag indicating if the pool is shutting down
 *  @var started      Number of started threads
 */
struct provider {
    char name[50];
    int quality;
    int price;
    int logtime;
    int taskCompleted; //init of all this struct ! unutmayak
    int queueProvider[2]; // baslangic 0 0 , doldukca 1le giderken 0



};
struct threadpool_t {
  pthread_mutex_t lock;
  pthread_cond_t notify;
  pthread_t *threads;
    provider *providerList;
    threadpool_task_t *queue;
  int thread_count;
  int queue_size;
  int head;
  int tail;
  int count;
  int shutdown;
  int started;
    int initThreadCount;
    char willSendString[200];

    //
};

/**
 * @function void *threadpool_thread(void *threadpool)
 * @brief the worker thread
 * @param threadpool the pool which own the thread
 */
static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);
float taylorSeries(float *y){
    usleep(10000);
//    pthread_mutex_lock((pthread_mutex_t *) &lock2);

    float x = *y;
    //printf("%f x kac *x kac%f\n",x,*y);
    int n,i,j;
    float sign,cosx,fact;

    n=5;



    x= (float) (x * (3.142 / 180.0)); /* Degrees to radians*/

    cosx=1;

    sign=-1;

    for(i=2;i<=n;i=i+2)
    {
        fact=1;

        for(j=1;j<=i;j++)
        {
            fact=fact*j;
        }

        cosx= (float) (cosx + (pow(x, i) / fact) * sign);
        sign=sign*(-1);
    }
   // printf("Taylor Series%lf \n",(float)cosx);
   // pthread_mutex_unlock((pthread_mutex_t *) &lock2);

    return cosx;

}
threadpool_t *threadpool_create(int thread_count, int queue_size)
{
    threadpool_t *pool;
    int i;

    if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
        return NULL;
    }

    if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        exit(9);
    }

    /* Initialize */
    pool->providerList = (provider*)malloc(sizeof(provider)*50);
    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;

    /* Allocate thread and task queue */
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (threadpool_task_t *)malloc
        (sizeof(threadpool_task_t) * queue_size);

    /* Initialize mutex and conditional variable first */
    if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
       (pthread_cond_init(&(pool->notify), NULL) != 0) ||
       (pool->threads == NULL) ||
       (pool->queue == NULL)) {
        exit(9);
    }

    /* Start worker threads */
    for(i = 0; i < thread_count; i++) {
        if(pthread_create(&(pool->threads[i]), NULL,
                          threadpool_thread, (void*)pool) != 0) {
            threadpool_destroy(pool, 0);
            return NULL;
        }
        pool->thread_count++;
        pool->started++;
    }

    return pool;


}

int threadpool_add(threadpool_t *pool, void (*function)(void *),
                   void *argument, int flags)
{
    int err = 0;
    int next;
    (void) flags;

    if(pool == NULL || function == NULL) {
        return threadpool_invalid;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    next = (pool->tail + 1) % pool->queue_size;

    do {
        /* Are we full ? */
        if(pool->count == pool->queue_size) {
            err = threadpool_queue_full;
            break;
        }

        /* Are we shutting down ? */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        /* Add task to queue */
        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;

        /* pthread_cond_broadcast */
        if(pthread_cond_signal(&(pool->notify)) != 0) {
            err = threadpool_lock_failure;
            break;
        }
    } while(0);

    if(pthread_mutex_unlock(&pool->lock) != 0) {
        err = threadpool_lock_failure;
    }

    return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
    int i, err = 0;

    if(pool == NULL) {
        return threadpool_invalid;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return threadpool_lock_failure;
    }

    do {
        /* Already shutting down */
        if(pool->shutdown) {
            err = threadpool_shutdown;
            break;
        }

        pool->shutdown = (flags & threadpool_graceful) ?
            graceful_shutdown : immediate_shutdown;

        /* Wake up all worker threads */
        if((pthread_cond_broadcast(&(pool->notify)) != 0) ||
           (pthread_mutex_unlock(&(pool->lock)) != 0)) {
            err = threadpool_lock_failure;
            break;
        }

        /* Join all worker thread */
        for(i = 0; i < pool->thread_count; i++) {
            if(pthread_join(pool->threads[i], NULL) != 0) {
                err = threadpool_thread_failure;
            }
        }
    } while(0);

    /* Only if everything went well do we deallocate the pool */
    if(!err) {
        threadpool_free(pool);
    }
    return err;
}

int threadpool_free(threadpool_t *pool)
{
    if(pool == NULL || pool->started > 0) {
        return -1;
    }

    /* Did we manage to allocate ? */
    if(pool->threads) {
        free(pool->threads);
        free(pool->queue);
        free(pool->providerList);
 
        /* Because we allocate pool->threads after initializing the
           mutex and condition variable, we're sure they're
           initialized. Let's lock the mutex just in case. */
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
    }
    free(pool);    
    return 0;
}


static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    for(;;) {
        /* Lock must be taken to wait on conditional variable */
        pthread_mutex_lock(&(pool->lock));

        /* Wait on condition variable, check for spurious wakeups.
           When returning from pthread_cond_wait(), we own the lock. */
        while((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if((pool->shutdown == immediate_shutdown) ||
           ((pool->shutdown == graceful_shutdown) &&
            (pool->count == 0))) {
            break;
        }


        /* Grab our task */
        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        /* Unlock */
        pthread_mutex_unlock(&(pool->lock));

        /* Get to work */
        (taylorSeries)(task.argument);
    }

    pool->started--;

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return(NULL);
}


int readFromFile(char *string,threadpool_t *pool){
    FILE *fp = fopen(string,"r");
    if(fp==NULL){
        perror("file null \n");
        exit(-1);
    }
    int ch;
    int length=0;
    while(!feof(fp))
    {
        ch = fgetc(fp);
        if(ch == '\n')
        {
            length++;
        }
    }
    rewind(fp);

    // FIRST 4 strings are not related to Client so skip them
    char buffer[100];
    fgets(buffer,255,fp);
    printf("%s",buffer);

    //PROVIDERS
    for(int i=0;i<length;i++){
        fscanf(fp,"%s %d %d %d\n",pool->providerList[i].name,&pool->providerList[i].quality,
               &pool->providerList[i].price,&pool->providerList[i].logtime);
        printf("%s  %d        %d       %d\n",pool->providerList[i].name,pool->providerList[i].quality,pool->providerList[i].price,
               pool->providerList[i].logtime);
    }

    fclose(fp);
    pool->initThreadCount=length;
    return length;
}
char *getStringFromPool(threadpool_t *pThreadpool) {
    return pThreadpool->willSendString;

}
void initStruct(threadpool_t *pool){
    for(int i=0;i<pool->thread_count;i++){
        pool->providerList[i].taskCompleted = 0;
    }
}
void checkStruct(threadpool_t *pool){
    for(int i=0;i<pool->initThreadCount;i++){
        if(pool->providerList[i].taskCompleted >9999)
            pool->providerList[0].taskCompleted = 0;
    }
}
char *getProviderName(int i,threadpool_t *pThreadpool) {
    return pThreadpool->providerList[i].name;
}
void threadSelect(threadpool_t *pool,char select,float *homework,char *clientName,pthread_mutex_t *lock){
    //According to choice , find best index for it,if its not full
    srand ((unsigned int) time(NULL));
    lock2 = lock;
    int lowCostIndex = -1,qualityIndex = -1,timeIndex = -1;
    int temp = 0;
    int tempPrice = 9999;
    for(int i=0;i<pool->initThreadCount;i++){
        if(select == 'C'){
            // IF its better quality and IF 1 or 2 queue is free  for assigning to this
            if(pool->providerList[i].price<tempPrice && (pool->providerList->queueProvider[0] == 0 ||
                                                      pool->providerList->queueProvider[1] == 0)){
                tempPrice = pool->providerList[i].price;
                lowCostIndex = i;
            }
        }
        else if(select == 'Q'){
            // IF its better quality and IF 1 or 2 queue is free  for assigning to this
            if(pool->providerList[i].quality>temp && (pool->providerList->queueProvider[0] == 0 ||
                                                     pool->providerList->queueProvider[1] == 0)){
                temp = pool->providerList[i].quality;
                qualityIndex = i;
            }

        }
        else if(select == 'T'){
            // 0 sa hemen bulmusuz breakle
            //1se daha 0lıyı bulabiliriz devam
                if(pool->providerList[i].queueProvider[0] == 0){
                    timeIndex = i;
                    break;
                }
                else if(pool->providerList[i].queueProvider[1] == 0){
                    timeIndex = i;
                }
        }
        else{
            perror("wrong choice of Q,T,C");
        }
    }
    int randomSleep = rand()%11+5;

    //AFTER SELECTION JUST SEND STRING TO CLIENT ->
    if(select == 'C'){

        if(pool->providerList[lowCostIndex].queueProvider[0] == 0){
            printf("Provider %s is processing task :0 %f",pool->providerList[lowCostIndex].name,*homework);

            pool->providerList[lowCostIndex].queueProvider[0] = 1;
            pool->providerList[lowCostIndex].taskCompleted++;
        }
        else if(pool->providerList[lowCostIndex].queueProvider[1] == 0){
            printf("Provider %s is processing task :1 %f",pool->providerList[lowCostIndex].name,*homework);

            pool->providerList[lowCostIndex].queueProvider[1] = 1;
            pool->providerList[lowCostIndex].taskCompleted++;
        }
        else{
            perror("SOMETHING WENT WRONG WITH SELECTION of Cheap chosing\n");
        }

        sprintf(pool->willSendString,"%s 's task completed by %s in %d seconds,cos(%f) = %f,cost is %d TL",
                clientName,pool->providerList[lowCostIndex].name,randomSleep,*homework,taylorSeries(homework),pool->providerList[lowCostIndex].price);
    }
    if(select == 'Q'){
        if(pool->providerList[qualityIndex].queueProvider[0] == 0){
            printf("Provider %s is processing task :0 %f",pool->providerList[qualityIndex].name,*homework);

            pool->providerList[qualityIndex].queueProvider[0] = 1;
            pool->providerList[qualityIndex].taskCompleted++;
        }
        else if(pool->providerList[qualityIndex].queueProvider[1] == 0){
            printf("Provider %s is processing task :1 %f",pool->providerList[qualityIndex].name,*homework);

            pool->providerList[qualityIndex].queueProvider[1] = 1;
            pool->providerList[qualityIndex].taskCompleted++;
        }
        else{
            perror("SOMETHING WENT WRONG WITH SELECTION of Quality choosi\n");
        }
        sprintf(pool->willSendString,"%s 's task completed by %s in %d seconds,cos(%f) = %f,cost is %d TL",
                clientName,pool->providerList[qualityIndex].name,randomSleep,*homework,taylorSeries(homework),pool->providerList[qualityIndex].price
        );
    }
    if(select == 'T'){
            if(pool->providerList[timeIndex].queueProvider[0] == 0){
                printf("Provider %s is processing task :0 %f",pool->providerList[timeIndex].name,*homework);

                pool->providerList[timeIndex].queueProvider[0] = 1;
                pool->providerList[timeIndex].taskCompleted++;
            }
            else if(pool->providerList[timeIndex].queueProvider[1] == 0){
                printf("Provider %s is processing task :1 %f",pool->providerList[qualityIndex].name,*homework);

                pool->providerList[timeIndex].queueProvider[1] = 1;
                pool->providerList[timeIndex].taskCompleted++;
            }
            else{
                perror("SOMETHING WENT WRONG WITH SELECTION of Time choosi\n");
            }

        sprintf(pool->willSendString,"%s 's task completed by %s in %d seconds,cos(%lf) = %f,cost is %d TL",
                clientName,pool->providerList[timeIndex].name,randomSleep,*homework,taylorSeries(homework),pool->providerList[timeIndex].price
        );

        }
    printf("\nsleeping for %d seconds ! DOING HOMEWORK \n",randomSleep);
    sleep((unsigned int) randomSleep);
    printf("Homework Completed.\n");
    //printf("%s -> sentence \n",pool->willSendString);
    threadpool_add(pool,(void (*)(void *))&taylorSeries, homework, 0);

}
void print_statics(threadpool_t *pool) {
  //  checkStruct(pool);
    printf("Termination signal received\n");
    printf("Termination all clients \n");
    printf("Termination all providers \n");
    printf("Statistics \n");
    printf("Name  --------Number of clients served\n");

    for(int i=0;i<pool->initThreadCount;i++){
        printf("%s  ---- %d\n",pool->providerList[i].name,pool->providerList[i].taskCompleted);
        }
}
