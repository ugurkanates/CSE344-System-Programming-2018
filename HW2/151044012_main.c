#include <stdio.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include<math.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <string.h>
struct flock fd_lock = { F_WRLCK, SEEK_SET, 0, 0, 0 };

const char* name = "file.dat";
const char* logparent ="logparent.log";
const char* logchild = "child.log";

FILE *fpchild; //global file pointer for log file of child
FILE *fpparent; // global file pointer for log file of parent

float *arrayReal;
float *arrayImage;
float *willReturn;

char *buffer;
char *sayi;
char *satir;

int writeOperation = 0 ;
int SINYAL_SET = -1;

int satirboyutu;

int fd; //file Descriptor must be global to closed within sig_handler ->global variable-method.


void sig_handler(int signo)
{
    if (signo == SIGINT) {
        printf("received Ctrl+C\n");
        printf("closing opened files...\n");
        close(fd); // this ensures file is opened before closed
        fclose(fpchild);
        fclose(fpparent);
        free(arrayImage);
        free(arrayReal);
        free(willReturn);
        free(buffer);
        free(sayi);
        free(satir);

        exit(0); //any memory leaks if exist will be killed ->

    }
    else if(signo == SIGUSR1) {
        SINYAL_SET = 0 ;
        writeOperation = 0; // this is must .
    }
    else if(signo == SIGUSR2) {
        SINYAL_SET = 1;
       // printf("my pidCHILD is %d ve SINYALSETIM %d \n",getpid(),SINYAL_SET);

    }
}

int DFTCalculator(float *array,int size){

    if(arrayReal == NULL)
        perror("tellme\n");
    if(arrayImage == NULL)
        perror("tellme\n");


    for(int k=0;k<size;k++)
    {
        arrayReal[k] = arrayImage[k]=0.0;
        for(int n=0;n<size;n++)
        {
            arrayReal[k]= (float) (arrayReal[k] + array[n] * cos((2 * M_PI * k * (n - size)) / size));
            arrayImage[k]= (float) (arrayImage[k] + array[n] * sin((2 * M_PI * k * (n - size)) / size));
        }
        arrayImage[k]= (float) (arrayImage[k] * (-1.0));
    }
    printf("Process B: the dft of line size of (");
    fprintf(fpchild,"Process B: the dft of line size of (");


    for(int m=0;m<size;m++) {
        printf("%1.0f ", array[m]);
        fprintf(fpchild,"%1.0f ",array[m]);
    }
    printf(")is :");
    fprintf(fpchild,")is :");

    for(int k=0;k<size;k++) {
        printf(" %.2f%.2fi ", arrayReal[k], arrayImage[k]);
        fprintf(fpchild," %.2f%.2fi ", arrayReal[k], arrayImage[k]);

    }

    printf("\n");
    fprintf(fpchild,"\n");

    return 0;

}
void mapToFloat(char *map,float* willReturn){

    int i = 0;
    int j = 0 ;
    int floatSize = 0;
    while(1) {
        if (map[i] == ' ') {
            sayi[j] = '\0';
            float item = (float) atof(sayi);
            j = 0;
            willReturn[floatSize] = item;
            floatSize++;
        } else if (map[i] == '\n') {
            sayi[j] = '\0';
            float item = (float) atof(sayi);
            willReturn[floatSize] = item;
            break;
        } else {
            sayi[j] = map[i];
            j++;
        }
        i++;
    }
}
void parentMain(int child,int maxAtFile,int sizem) {
    // process A will write random to file
    fpparent= fopen(logparent,"w"); // Open file for writing

    if (signal(SIGUSR1, sig_handler) == SIG_ERR)
        printf("\ncan't catch process A signal\n");
    while (1) {
        if (writeOperation < maxAtFile) {


           // printf("write Operation :%d\n", writeOperation);
            fcntl(fd, F_SETLKW, &fd_lock);
            //printf("writing should be -> %s \n",bufe);
            //satir = malloc((sizeof(char) + sizeof(int)) * sizem - 1 + sizeof(char) * 2 + sizeof(int));
            for (int j = 0; j < sizem; j++) {
                if (sizem - 1 != j && j != 0) {
                    sprintf(satir, "%s%c%d", satir, ' ', rand() % 10);
                } else if (j == 0) {
                    sprintf(satir, "%s%d", satir, rand() % 10);
                } else {
                    sprintf(satir, "%s%c%d%c", satir, ' ', rand() % 10, '\n');
                }
            }


            int howmany = write(fd, satir, satirboyutu);
            printf("Process A: I'm producing a random sequence for line:%s\n", satir);
            fprintf(fpparent,"Process A: I'm producing a random sequence for line:%s\n", satir);

            memset(satir,0,strlen(satir));

            //free(satir);
            //printf("how many -> %d\n",howmany);
            // sleep(3);
            fcntl(fd, F_UNLCK, &fd_lock);
            writeOperation++;
            sleep(2);
            kill(child, SIGUSR2);
        } else {

        }


    }
}



void childMain(char *argv[]) {
    char **buf;

    // process B will read delete and use mathematical function
    if (signal(SIGUSR2, sig_handler) == SIG_ERR)
        printf("\ncan't catch process B SIGNAL\n");
    //process b to start sinyal must be 1
    //while process b ends it sets its own signal to 0 again
    // and also sets signal to catch by other process to set their signal.3
    fpchild= fopen(logchild,"w"); // Open file for writing

    while(1){

        if(SINYAL_SET == 1) {


            fcntl(fd, F_SETLK, &fd_lock);

            lseek(fd,-satirboyutu,SEEK_END);
            read(fd,buffer,satirboyutu);

            mapToFloat(buffer,willReturn);

            DFTCalculator(willReturn,(abs(atof(argv[2]))));
           // willReturn = NULL;
            //willReturn='\0';
            /*free(willReturn);
            willReturn=malloc(sizeof(float)*(abs(atof(argv[2]))));
             */

            memset(buffer,0,strlen(buffer));

            lseek(fd,-satirboyutu,SEEK_END);
            write(fd,"0 0 0\n",satirboyutu);


            fcntl(fd, F_UNLCK, &fd_lock);
            SINYAL_SET = 0;
            kill(getppid(),SIGUSR1);
        }
    }
    free(willReturn);


}



int main(int argc, char *argv[]) {
    // ./multiprocess_dft -N -5 -X file.dat -M 100

    char *fileName = argv[4];

    if(argc!=7 || strcmp(argv[0],"-N")==0 || strcmp(argv[2],"-X")==0 || strcmp(argv[4],"-M")==0 ) {
        printf("Error.Usage : ./multiprocess_dft -N -5 -X file.dat -M 100\n");
        exit(-2);
    }
    srand(time(NULL));
    //initalize the random seed
    satirboyutu = (sizeof(char)+sizeof(int))*(abs((atoi(argv[2])))-1)+sizeof(char)*2+sizeof(int);

    arrayReal = malloc(abs(atof(argv[2]))*sizeof(float));
    arrayImage = malloc(abs(atof(argv[2]))*sizeof(float));
    willReturn = malloc(sizeof(float)*(abs(atof(argv[2]))));
    buffer = malloc ( sizeof(char)*50);
    sayi = malloc ( sizeof(char)*10);
    satir = malloc ( sizeof(char)*50);





    fd = open(fileName, O_RDWR | O_CREAT,S_IRWXU);


    if(fd == -1){
        printf("I couldn't open %s.Error happened\n",fileName);
        exit(-3);
    }
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");


    int child = 0;
    switch (child=fork()){
        case -1:
            exit(-1);
        case 0:
            childMain(argv); //acts as process B ->  read endless sequence of Numbers and do calculations
            break;
        default:
            parentMain(child,atoi(argv[6]),abs(atoi(argv[2]))); //acts as process A -> endless sequence of N random real numbers
            break;
    }


    return 0;
}



