#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>


int main(int argc,char *argv[])
{
   
    char myfile[1024];
    if(argc == 2)
      strcpy(myfile,argv[1]);
   else if(argc == 3)
      strcpy(myfile,argv[2]);
   int words = 0;
   char buffer[1];
   int file = open(myfile,O_RDONLY);
   enum states { WHITESPACE, WORD };
   int state = WHITESPACE;
   if(file == -1){
      printf("can not find :%s\n",myfile);
   }
   else{
      char last = ' '; 
      while (read(file,buffer,1) ==1 )
      {
         if ( buffer[0]== ' ' || buffer[0] == '\t'  )
         {
            state = WHITESPACE;
         }
         else if (buffer[0]=='\n')
         {
            state = WHITESPACE;
         }
         else 
         {
            if ( state == WHITESPACE )
            {
               words++;
            }
            state = WORD;
         }
         last = buffer[0];
      }        
      printf("%d = word count - > file name %s\n",words,myfile);        
   } 

}