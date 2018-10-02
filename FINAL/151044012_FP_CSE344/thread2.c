#define THREAD 4
#define QUEUE  2

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "threadpool.h"
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h> //for threading , link with lpthread
#include <signal.h>

typedef struct provider *ptr;
threadpool_t *pool; // for connection handler to access
int tasks = 0, done = 0;
pthread_mutex_t lock;


int fileSize(char *string){

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
    fclose(fp);
    return length;

}



/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    char *sendBack;

    //Send some messages to the client
   // message = "Greetings! I am your connection handler\n";
   // write(sock , message , strlen(message));



    //Receive a message from client
    while((read_size = (int) recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        char choice;
        char nameClient[100];
        float angle = 0;
        float port = 0;
        char gereksiz[100];
        int gereksiz1;
        printf(" cumle %s\n",client_message);

        sscanf(client_message,"Client %s is requesting %c %f from server %s %d\n",nameClient,&choice,&angle,gereksiz,&gereksiz1);
        //float * kasmt sıkıntı cikarabilir?
        threadSelect(pool, choice,&angle,nameClient, &lock);

        sendBack = getStringFromPool(pool);

        write(sock , sendBack , strlen(sendBack));
    }

    if(read_size == 0)
    {
      //  puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}


void sinyal_handler(int signo)
{
    if (signo == SIGINT || signo == SIGTERM) {
        printf("SIGINT-SIGTERM applied to program\n");
        print_statics(pool);
        threadpool_destroy(pool,0); // KILLING ALL PROVIDERS - free memory

        _exit(0); // CLOSING SOCKET AND EXIT
    }


}

int main(int argc, char **argv)
{
    if(argc!=4){
        printf("usage is ./homeworkServer 5555 data.dat log.dat");
        exit(0);
    }
    srand ((unsigned int) time(NULL));
    FILE *fp = fopen(argv[3],"w+");
    if(fp == NULL){
        perror("file corrupt");
        exit(3);
    }

    int providerSize=0;
    printf("Logs kept at log.dat\n");
    providerSize = fileSize(argv[2]);
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    signal(SIGINT,sinyal_handler);
    signal(SIGTERM,sinyal_handler);

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((uint16_t) atoi(argv[1]));
    //server.sin_port = htons(5555);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    //puts("bind done");

    //Listen
    listen(socket_desc ,7);
    //Accept and incoming connection
    //puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    pool = threadpool_create(providerSize, QUEUE*providerSize);
    initStruct(pool);
    providerSize=readFromFile(argv[2],pool);
    for(int i=0;i<providerSize;i++){
        printf("Provider %s waiting for tasks\n",getProviderName(i,pool));
        fprintf(fp,"Provider %s waiting for tasks\n",getProviderName(i,pool));

    }
    printf("%d provider threads created\n",providerSize);
    printf("Server is waiting for client connections at port %d\n",atoi(argv[1]));
    fprintf(fp,"%d provider threads created\n",providerSize);
    fprintf(fp,"Server is waiting for client connections at port %d\n",atoi(argv[1]));
    pthread_mutex_init(&lock, NULL);
    float sayi = 20;



    while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    threadpool_destroy(pool, 0);

    pthread_mutex_destroy(&lock);
    fclose(fp);
    return 0;

}