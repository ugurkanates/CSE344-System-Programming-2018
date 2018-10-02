#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 *
 * CSE344 – System Programming - Homework #4
Submission deadline: 23:55, May 13 th , 2018
Let’s assume for the sake of simplicity that one needs exactly 4 ingredients for preparing a portion of
şekerpare:
- eggs
- flour
- butter
- sugar

 -> In this homework we need
 Multiple child processes
 Signal Handler for killing all proccesses with error checking/inputoutput sym closers.
 Semaphores for main structure bone of chief - wholesaler system.
 minimum Combination (4,2) process+Chief Process+ Parent process
 Shared memory  usage for getting ingreditens of Wholesaler everyturn

 this program will run without interrupts forever until CTRL+C or SIGTERM arrives.

 ALL ERROR Check indeed.
 works no problem with Valgrind
 compiled on Virtual machine providede by school.

 */
#define SEM_NAME1 "/semaphore_chief1"
#define SEM_NAME2 "/semaphore_chief2"
#define SEM_NAME3 "/semaphore_chief3"
#define SEM_NAME4 "/semaphore_chief4"
#define SEM_NAME5 "/semaphore_chief5"
#define SEM_NAME6 "/semaphore_chief6"
#define SEM_NAME7 "/semaphore_saler"
// name of the shared memory object
#define SHMOBJ_PATH1 "/buffer_seg1"
#define SHMOBJ_PATH2 "/buffer_seg2"


#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 0
#define PROCESS_N 7

#define EF 1
#define EB 2
#define ES 3
#define FB 4
#define FS 5
#define BS 6

#define SHMBUF_LENGTH 100


/*
 * Chief Process Categorization
 * 1- EF need
 * 2- EB need
 * 3- ES need
 * 4- FB need
 * 5- FS need
 * 6- BS need
 * 7- Whole Seller ->
 *
 *
 */

/*
 * suraya string tanimla 6 cumleli bide string[i] ekrana bastırma deneyek artık wtf?
 *
 */
void    unlink_semaphores(){
    sem_unlink(SEM_NAME1) < 0;
       // perror("sem_unlink(3) failed");

    sem_unlink(SEM_NAME2) < 0;
        //perror("sem_unlink(3) failed");
    sem_unlink(SEM_NAME3) < 0;
       // perror("sem_unlink(3) failed");
    sem_unlink(SEM_NAME4) < 0;
       // perror("sem_unlink(3) failed");
    sem_unlink(SEM_NAME5) < 0;
        //perror("sem_unlink(3) failed");
    sem_unlink(SEM_NAME6) < 0;
        //perror("sem_unlink(3) failed");

    sem_unlink(SEM_NAME7) < 0;
        //perror("sem_unlink(3) failed");
    if (shm_unlink(SHMOBJ_PATH1) != 0) {
       // perror("In shm_unlink() of buffer 1");
        exit(1);
    }
}
void sinyal_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        //if(signo == SIGINT) printf("received Ctrl+C\n");
        //if(signo == SIGTERM) printf("received SIGTERM \n");

        //printf("closing semaphores and giving shared maps...\n");
        unlink_semaphores();
        _exit(EXIT_FAILURE);
    }


}
int isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return 1;
    }
    return 0;
}
void randomizer(int array[]){
    for(int i=0;i<PROCESS_N-1;i++){
        int a = rand()%6+1;
        while(isvalueinarray(a,array,PROCESS_N-1)==1){
            a=rand()%6+1;
        }
        array[i] = a ;
    }

}
void print_whatseller(int ra) {
    switch (ra){
        case 1:
            printf("Whole Saler has EGG--FLOUR THIS TIME\n");
            break;
        case 2:
            printf("Whole Saler has EGG--BUTTER THIS TIME\n");
            break;
        case 3:
            printf("Whole Saler has EGG--SUGAR THIS TIME\n");
            break;
        case 4:
            printf("Whole Saler has FLOUR--BUTTER THIS TIME\n");
            break;
        case 5:
            printf("Whole Saler has BUTTER--SUGAR THIS TIME\n");
            break;
        case 6:
            printf("Whole Saler has BUTTER--SUGAR THIS TIME\n");
            break;
        default:
            break;

    }
}


int main() {
    signal(SIGINT,&sinyal_handler);
    signal(SIGTERM,&sinyal_handler);


    sem_t *semi1 = sem_open(SEM_NAME1, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *semi2 = sem_open(SEM_NAME2, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *semi3 = sem_open(SEM_NAME3, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *semi4 = sem_open(SEM_NAME4, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *semi5 = sem_open(SEM_NAME5, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *semi6 = sem_open(SEM_NAME6, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *semi7 = sem_open(SEM_NAME7, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *semiArray[] = {semi1,semi2,semi3,semi4,semi5,semi6,semi7};

    int randomChief[6];
    //
    srand(time(NULL));
    randomizer(randomChief); // for each run Chiefs should be randomly ->


    /* pointers to shared memory segments */
    float* shmptr1;
    int shmfd1;
    int shared_seg_size = (SHMBUF_LENGTH * sizeof(float));

    /* creating the shared memory objects    --  shm_open()  */
    shmfd1 = shm_open(SHMOBJ_PATH1, O_CREAT | O_RDONLY, S_IRWXU | S_IRWXG);
    if (shmfd1 < 0) {
        perror("In shm_open() of buffer 1.");
        exit(1);
    }

    /* requesting the shared segment    --  mmap() */
    shmptr1 = (float *)mmap(NULL, shared_seg_size, PROT_READ, MAP_SHARED, shmfd1, 0);
    if (shmptr1 == NULL) {
        perror("In mmap() requesting buffer 1.");
        exit(1);
    }

    if (semi1 == SEM_FAILED || semi2 ==  SEM_FAILED || semi3 ==  SEM_FAILED
        || semi4 ==  SEM_FAILED || semi5 ==  SEM_FAILED || semi6 ==  SEM_FAILED) {
        perror("sem_open -> (3) error");
        exit(EXIT_FAILURE);
    }

    int i,ra;
    for (i = 0; i < PROCESS_N; i++) {
        if (fork() == 0) {
            // do the job specific to the child process
            //random shared variable -> ra

            while(1){
                if(i == EF){
                    int sem_value;
                    sem_getvalue(semi7,&sem_value);
                    printf("Chief 1 have Butter - Sugar -> Needs Egg-Flour \n");
                    sem_wait(semi1);

                    //  printf("sem71 value -> %d\n",sem_value);


                    sem_getvalue(semi7,&sem_value);
                    //printf("sem71 d value -> %d\n",sem_value);



                    printf("Chief 1 took items, making the cake ..\n");
                    printf("Cake is done delievered to Wholesaler \n");
                    sem_post(semi7);
                    // sem_wait(semi1);

                    /* sem_post(semi1);
                     sem_post(semi2);
                     sem_post(semi3);
                     sem_post(semi4);
                     sem_post(semi5);
                     sem_post(semi6);
                     */

                }
                else if(i == EB){
                    int sem_value;
                    sem_getvalue(semi7,&sem_value);
                    printf("Chief 2 have Flour - Sugar -> Needs Egg-Butter \n");
                    sem_wait(semi2);



                    printf("Chief 2 took items, making the cake ..\n");
                    printf("Cake is done delievered to Wholesaler \n");
                    sem_post(semi7);
                    //sem_wait(semi2);



                }
                else if(i == ES){
                    int sem_value;
                    printf("Chief 3 have Flour - Butter -> Needs Egg-Sugar \n");
                    sem_wait(semi3);


                    sem_getvalue(semi7,&sem_value);


                    printf("Chief 3 took items, making the cake ..\n");
                    printf("Cake is done delievered to Wholesaler \n");
                    sem_post(semi7);
                    // sem_wait(semi3);





                }
                else if(i == FB){
                    int sem_value;
                    sem_getvalue(semi7,&sem_value);
                    printf("Chief 4 have Egg - Sugar -> Needs Flour-Butter\n ");
                    sem_wait(semi4);




                    printf("Chief 4 took items, making the cake ..\n");
                    printf("Cake is done delievered to Wholesaler \n");
                    sem_post(semi7);
                    //sem_wait(semi4);



                }
                else if(i == FS){
                    int sem_value;
                    sem_getvalue(semi7,&sem_value);
                    printf("Chief 5 have Egg - Butter -> Needs Flour-Sugar \n");
                    sem_wait(semi5);




                    printf("Chief 5 took items, making the cake ..\n");
                    printf("Cake is done delievered to Wholesaler \n");
                    sem_post(semi7);
                    // sem_wait(semi5);




                }
                else if(i == BS){
                    int sem_value;
                    sem_getvalue(semi7,&sem_value);
                    printf("Chief 6 have Egg - Flour -> Needs Butter-Sugar\n ");
                    sem_wait(semi6);



                    printf("Chief 6 took items, making the cake ..\n");
                    printf("Cake is done delievered to Wholesaler \n");
                    sem_post(semi7);
                    // sem_wait(semi6);

                }




                else{

                    int sem_value = sem_getvalue(semi7,&sem_value);
                    //  printf("DEBUG FIRST SEM --> %d \n",sem_value);
                    while(sem_value !=0 && sem_value>0) {
                        sem_wait(semi7);
                    }
                    //printf("DEBUG SECOND SEM  --> %d \n",sem_value);

                    printf("Whole Saler Arrived \n");
                    ra = rand()%6+1;
                    print_whatseller(ra);
                    //<

                    printf("Waiting For the cake ...\n");



                    sem_post(semiArray[ra-1]);
                    sem_value = sem_getvalue(semi7,&sem_value);
                    //printf("DEBUG  SEM  --> %d \n",sem_value);
                    /*while(sem_value>0) {
                        sem_wait(semi7);
                    }
                     */
                    sem_wait(semi7);
                    printf("wholesaler has obtained the dessert and left to sell it\n");
                    /*sem_post(semi7);
                    //for not getting confused
                    for(int a=0;a<PROCESS_N-1;a++){
                     //   if(a==ra){}
                       // else
                            sem_post(semiArray[a]);
                    }
                     */

                }
            }
            // don't forget to exit from the child - < never accure if error
            //stops childs spawning more childs.

            exit(0);
        }

    }


    /*
     * Not arrive  here cause its in while 1 loop ->
     */
    int status;
    for (i = 0; i < PROCESS_N; ++i)
        wait(&status);




    return 0;
}
/*
 *
 * eggs
 * flor
 * buter
 * sugar
 * = sekerpare
 *  N chef,endless sonsuz of 2 tane.(herkes kendine ait 2 tanesinden sonsuz var)
 *  kalan ikisi eksik
 *
 *
 */