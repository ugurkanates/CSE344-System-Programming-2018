 // This is the lsd function, yet another C implement of the classic ls, using UNIX functions

// Featuring "stat", "opendir", and "readdir"
// Credits: Jalil Benayachi, ECE PARIS - under MIT license
// contact [at] thejals.com

// Also thanks to some contributors on Stackoverflow 

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


int main(int argc,char* argv[])
{   
    //printf("%s bu dosya aciliyor mu  ve argc kac %d\n",argv[1],argc);

    //Defining the different components of the program
        //The directory: it's the folder we're browsing (we'll use an argument (argv) in order to identify it)
    DIR *thedirectory;
        //The file: when a file is found in the directory readdir loop, it's going to be called this way.
    struct dirent *thefile;
        //The stat: It's how we'll retrieve the stats associated to the file. 
    struct stat thestat;
        //will be used to determine the file owner & group
    struct passwd *tf; 
    struct group *gf;

    //Creating a placeholder for the string. 
    //We create this so later it can be properly adressed.
    //It's reasonnable here to consider a 512 maximum lenght, as we're just going to use it to display a path to a file, 
    //but we could have used a strlen/malloc combo and declared a simple buf[] at this moment
    char buf[512];
        char currentDirectory[1024];


    //It's time to assign thedirectory to the argument: this way the user will be able to browse any folder simply by mentionning it 
    //when launching the lsd program.
    
    
    
    if (getcwd(currentDirectory, sizeof(currentDirectory)) != NULL){
                    thedirectory = opendir(currentDirectory);
    }
      else{
        printf("eror");
        perror("such a error acquired. -> LS takes either 0 argument or  1 argument other wise program will crash\n");
        exit(-1);
       }   
  
    
         
    
    printf("FILE TYPE --- ACCESS RIGHTS -- FILE SIZE  -- FILE NAME\n");
    printf("Current Directory is : %s \n",currentDirectory);

    //If a file is found (readdir returns a NOT NULL value), the loop starts/keep going until it has listed all of them. 
    while((thefile = readdir(thedirectory)) != NULL) 
    {   
        //We sprint "thedirectory/thefile" which defines the path to our file 
        sprintf(buf, "%s/%s",currentDirectory, thefile->d_name);
        //Then we use stat function in order to retrieve information about the file
        stat(buf, &thestat);

        //Now, we can print a few things !
        // Here's the right order
        // [file type] [permissions] [number of hard links] [owner] [group] [size in bytes] [time of last modification] [filename]

        // [file type]
        //Let's start with the file type
        //The stat manual is pretty complete and gives details about st_mode and S_IFMT: http://manpagesfr.free.fr/man/man2/stat.2.html
        //
        switch (thestat.st_mode & S_IFMT) {
            case S_IFBLK:  printf("b "); break;
            case S_IFCHR:  printf("c "); break; 
            case S_IFDIR:  printf("d "); break; //It's a (sub)directory 
            case S_IFIFO:  printf("p "); break; //fifo
            case S_IFLNK:  printf("l "); break; //Sym link
            case S_IFSOCK: printf("s "); break;
            //Filetype isn't identified
            default:       printf("- "); break;
                }
        //[permissions]
        //Same for the permissions, we have to test the different rights
        //READ http://linux.die.net/man/2/chmod 
        printf( (thestat.st_mode & S_IRUSR) ? " r" : " -");
        printf( (thestat.st_mode & S_IWUSR) ? "w" : "-");
        printf( (thestat.st_mode & S_IXUSR) ? "x" : "-");
        printf( (thestat.st_mode & S_IRGRP) ? "r" : "-");
        printf( (thestat.st_mode & S_IWGRP) ? "w" : "-");
        printf( (thestat.st_mode & S_IXGRP) ? "x" : "-");
        printf( (thestat.st_mode & S_IROTH) ? "r" : "-");
        printf( (thestat.st_mode & S_IWOTH) ? "w" : "-");
        printf( (thestat.st_mode & S_IXOTH) ? "x" : "-");

         printf(" ");   
        
        //And the easy-cheesy part
        //[size in bytes] [time of last modification] [filename]
        printf("%zu",thestat.st_size);
        printf(" %s\n", thefile->d_name);
    }
    closedir(thedirectory);

    return 1;
}