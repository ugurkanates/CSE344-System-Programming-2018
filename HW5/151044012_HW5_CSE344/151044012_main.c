#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define MAX_THREADS 64
#define MAX_QUEUE 65536
#define TH_INV -1
#define TH_LOCK -2
#define TH_FULL -3
#define TH_DOWN -4
#define TH_FAIL -5


#define MAX_CLIENT_SIZE 50
typedef struct threadpool_t threadpool_t;

/*
 *
 * UGURKAN ATES SYSTEM HOMEWORK 5
 * THREADPOOL  AS FLORIST
 * CLIENT AS CLIENT
 * CLIENT + THREADPOOL AS argument to send during TpoolTask
 * (client as struct variable , tpool as struct * variable)
 * time calculated like
 * 10-50 random + DISTANCE/SPEED  = full time (prepare+delivery)
 * used MUTEX,queue,threads for making and sync threads
 *
 *
 */





pthread_mutex_t mutlex;

struct client {
    char name[50];
    char flower[50];
    int activated;
    int x,y;

};

struct argumental {
    struct client clientpointer;
    struct threadpool_t* tpoolpointer;
    char florist[50];
};

typedef struct {
    void (*function)(void *);
    struct argumental argument;
} threadpool_task_t;


struct threadpool_t {
    char threadpoolname[50];
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *threads;
    threadpool_task_t *queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    int shutdown;
    int started;
    double speed; // speed in terms of MS
    double totalTime; // number of total time in terms of MS
    int numberofSales; // number of total sales
    char flowers[3][50]; // will keep flower types
    int x,y;
    double distancer;

};



static void *threadpool_thread(void *threadpool);

int threadpool_free(threadpool_t *pool);

int checkedAll(int length);

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t *pool;
    int i;
    (void) flags;

    if(thread_count <= 0 || thread_count > MAX_THREADS || queue_size <= 0 || queue_size > MAX_QUEUE) {
        return NULL;
    }

    if((pool = (threadpool_t *)malloc(sizeof(threadpool_t))) == NULL) {
        goto err;
    }

    pool->thread_count = 0;
    pool->queue_size = queue_size;
    pool->head = pool->tail = pool->count = 0;
    pool->shutdown = pool->started = 0;

    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);
    pool->queue = (threadpool_task_t *)malloc
            (sizeof(threadpool_task_t) * queue_size);

    if((pthread_mutex_init(&(pool->lock), NULL) != 0) ||
       (pthread_cond_init(&(pool->notify), NULL) != 0) ||
       (pool->threads == NULL) ||
       (pool->queue == NULL)) {
        goto err;
    }

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

    err:
    if(pool) {
        threadpool_free(pool);
    }
    return NULL;
}

int threadpool_add(threadpool_t *pool, void (*function)(void *),
                   struct argumental argument, int flags)
{
    int err = 0;
    int next;
    (void) flags;

    if(pool == NULL || function == NULL) {
        return TH_INV;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return TH_LOCK;
    }

    next = (pool->tail + 1) % pool->queue_size;

    do {
        if(pool->count == pool->queue_size) {
            err = TH_FULL;
            break;
        }

        if(pool->shutdown) {
            err = TH_DOWN;
            break;
        }

        pool->queue[pool->tail].function = function;
        pool->queue[pool->tail].argument = argument;
        pool->tail = next;
        pool->count += 1;

        if(pthread_cond_signal(&(pool->notify)) != 0) {
            err = TH_LOCK;
            break;
        }
    } while(0);

    if(pthread_mutex_unlock(&pool->lock) != 0) {
        err = TH_LOCK;
    }

    return err;
}

int threadpool_destroy(threadpool_t *pool, int flags)
{
    int i, err = 0;

    if(pool == NULL) {
        return TH_INV;
    }

    if(pthread_mutex_lock(&(pool->lock)) != 0) {
        return TH_LOCK;
    }

    do {
        if(pool->shutdown) {
            err = TH_DOWN;
            break;
        }

        pool->shutdown = (flags & 1) ?
                         2 : 1;

        if((pthread_cond_broadcast(&(pool->notify)) != 0) ||
           (pthread_mutex_unlock(&(pool->lock)) != 0)) {
            err = TH_LOCK;
            break;
        }

        for(i = 0; i < pool->thread_count; i++) {
            if(pthread_join(pool->threads[i], NULL) != 0) {
                err = TH_FAIL;
            }
        }
    } while(0);

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

    if(pool->threads) {
        free(pool->threads);
        free(pool->queue);


        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
    }
    free(pool);
    return 0;
}


void floristCalculation(struct argumental arguman){
    //pthread_mutex_lock(&mutlex);

    int timer = 0;
    //10 - 50 random making +  distance/speed=time = total delivery time
    timer = rand()%40+10;
    if(strcmp(arguman.tpoolpointer->threadpoolname,"temp")!=0) {
        timer += arguman.tpoolpointer->distancer / arguman.tpoolpointer->speed;
        arguman.tpoolpointer->totalTime += timer;
        printf("Florist %s has delivered a %s to %s in %d ms\n ",
               arguman.florist, arguman.clientpointer.flower, arguman.clientpointer.name, timer);
        arguman.tpoolpointer->numberofSales++;
    }
    //pthread_mutex_unlock(&mutlex);


}

static void *threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    for(;;) {
        pthread_mutex_lock(&(pool->lock));


        while((pool->count == 0) && (!pool->shutdown)) {
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if((pool->shutdown == 1) ||
           ((pool->shutdown == 2) &&
            (pool->count == 0))) {
            break;
        }

        task.function = pool->queue[pool->head].function;
        task.argument = pool->queue[pool->head].argument;
        pool->head = (pool->head + 1) % pool->queue_size;
        pool->count -= 1;

        pthread_mutex_unlock(&(pool->lock));
        //FLORIST FONKSIYON CAGRI
        ((floristCalculation))(task.argument);
    }

    pool->started--;

    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
}
double distance(int x1, int y1, int x2, int y2){
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
int readFromFile(char *string,struct client obje[50]){
    FILE *fp = fopen(string,"r");
    if(fp==NULL){
        perror("file null \n");
        exit(-1);
    }
    char ch;
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
    fgets(buffer,255,fp);
    printf("%s",buffer);
    fgets(buffer,255,fp);
    printf("%s",buffer);
    fgets(buffer,255,fp);
    printf("%s",buffer);

    //clients
    for(int i=0;i<length;i++){
        fscanf(fp,"%s (%d,%d): %s\n",obje[i].name,&obje[i].x,&obje[i].y,obje[i].flower);
    }

    fclose(fp);

    return length;
}

double smallest(double x, double y, double z){
    return x < y ? (x < z ? x : z) : (y < z ? y : z);
}

int main(int argc,char *argv[]) {
    srand (time(NULL));
    pthread_mutex_init(&mutlex, NULL);

    threadpool_t *ayse = threadpool_create(1, 50, 0);
    usleep(10);

    ayse->x =10;
    ayse->y= 25;
    ayse->speed =1.5;
    strcpy(ayse->flowers[0],"orchid");
    strcpy(ayse->flowers[1],"rose");
    strcpy(ayse->flowers[2],"violet");
    strcpy(ayse->threadpoolname,"ayse");

    threadpool_t *fatma = threadpool_create(1, 50, 0);
    usleep(10);

    fatma->x =-10;
    fatma->y=-15;
    fatma->speed =1.3;
    strcpy(fatma->flowers[0],"clove");
    strcpy(fatma->flowers[1],"rose");
    strcpy(fatma->flowers[2],"daffodil");
    strcpy(fatma->threadpoolname,"fatma");



    threadpool_t *murat = threadpool_create(1, 50, 0);
    threadpool_t *temp = threadpool_create(1, 50, 0);

    usleep(10);

    murat->x =-10;
    murat->y= 8;
    murat->speed =1.1;
    strcpy(murat->flowers[0],"violet");
    strcpy(murat->flowers[1],"daffodil");
    strcpy(murat->flowers[2],"orchid");
    strcpy(murat->threadpoolname,"murat");


    if(argc!=2) {
        perror("Try running program like ./floristApp data.dat");
        exit(1);
    }
    struct client obje[MAX_CLIENT_SIZE];

    printf("Florist application initializing from file:%s\n",argv[1]);
    printf("3 florists have been created\n");
    int length=readFromFile(argv[1],obje);

    printf("Processing requests:\n");
    struct argumental arguman,arguman2;
    int i=0;
    int making = 0;
    strcpy(temp->threadpoolname,"temp");
    arguman2.tpoolpointer=temp;

    while(i<length && checkedAll(length)){
        int itemp = i;
        if(i<10) {
            making = rand() % 10;
            while(obje[making].activated == 1)
                making =rand()%10;
        }
        else{
            making = rand() % length;
            while(obje[making].activated == 1)
                making =rand()%length;
        }
        i = making;
        double ayseu=distance(obje[i].x,obje[i].y,ayse->x,ayse->y); //ayse en yakin
        double muratu=distance(obje[i].x,obje[i].y,murat->x,murat->y);
        double fatmau=distance(obje[i].x,obje[i].y,fatma->x,fatma->y);
        double small = smallest(ayseu,muratu,fatmau);
        if(ayseu == small){

            arguman.clientpointer = obje[i];
            arguman.tpoolpointer = ayse;
            strcpy(arguman.florist,"ayse");
            ayse->distancer=ayseu;
            threadpool_add(temp, &floristCalculation, arguman2, 0);
            floristCalculation(arguman);
        }
        if(muratu == small){
            arguman.clientpointer = obje[i];
            arguman.tpoolpointer = murat;
            strcpy(arguman.florist,"murat");
            murat->distancer=muratu;

            threadpool_add(temp, &floristCalculation, arguman2, 0);
            floristCalculation(arguman);


        }
        if(fatmau == small){
            arguman.clientpointer = obje[i];
            arguman.tpoolpointer = fatma;
            strcpy(arguman.florist,"fatma");
            murat->distancer=fatmau;

            threadpool_add(temp, &floristCalculation, arguman2, 0);
            floristCalculation(arguman);


        }
        i = itemp;
        i++;
    }

    printf("All requests processed.\n");
    printf("Ayse closing shop.\n");
    printf("Murat closing shop\n");
    printf("Fatma closing shop.\n");
    printf("Sale statistics for today:\n");
    printf("-------------------------------------------------\n");
    printf("Florist -> SALES -> Total time \n");
    printf("Ayse -> %d -> %lf\n",ayse->numberofSales,ayse->totalTime);
    printf("Murat -> %d -> %lf\n",murat->numberofSales,murat->totalTime);
    printf("Fatma -> %d -> %lf\n",fatma->numberofSales,fatma->totalTime);
    pthread_mutex_destroy(&mutlex);



    return 0;
}

int checkedAll(int length) {

}