/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <time.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // f
 //Client Hileci is requesting Q 45 from server 127.0.0.1:5555
//usage -> clientApp Hileci1 C 45 127.0.0.1 5555
// string char float/int interestinstring port

int main(int argc , char *argv[])
{
    if(argc!=6){

        printf("usage is ./clientApp Hileci1 C 45 127.0.0.1 5555");
        exit(0);
    }
       time_t start_t, end_t;
          time(&start_t);
    

    int sock;
    struct sockaddr_in server;
    char message[1000] , server_reply[2000];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
   // puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[5]));
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
   //puts("Connected\n");
     
    //keep communicating with server
    
        sprintf(message,"Client %s is requesting %c %d from server %s %d\n",argv[1],argv[2][0],atoi(argv[3]),argv[4],atoi(argv[5]));
        printf("%s",message);
        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
         
        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            puts("recv failed");
            return 1;
        }
         
       //puts("Server reply :");
        //puts(server_reply);
       printf("%s",server_reply );
           time(&end_t);
       double diff_t = difftime(end_t, start_t);

        printf("total time spent %lf seconds\n",diff_t);
    
     
    close(sock);
    return 0;
}