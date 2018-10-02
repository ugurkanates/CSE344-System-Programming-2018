#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#define DELIM " \t\r\n\a"

/*
  Function Declarations for builtin shell commands:
 */

char historyArray[1024][1024];

int help();
int exitcommand();
char **line_split(char *objLine);

/*
 *
 * Signal handler will handle signal of SIGTERM SIGINT = random termination & ctrl+c
 * will close any mem leaks and quit with proper message.
 *
 */
void sinyal_handler(int signo)
{
    if (signo == SIGINT) {
        printf("received Ctrl+C\n");
        printf("closing opened files...\n");

        exit(0); //any memory leaks if exist will be killed ->

    }
    if (signo == SIGTERM) {
        printf("received Ctrl+C\n");
        printf("closing opened files...\n");

        exit(0); //any memory leaks if exist will be killed ->

    }

}


/*
 *  Doing directional change like in Shell Terminal
 *
 *  cd ..  = go upward direction
 *  cd somewhere = going somewhere
 *  cd alone wont work and give error !!
 *
 */
int cd_command(char **arge)
{
    if (arge[1] == NULL) {
        fprintf(stderr, "CD:Wrong Argument ->\n");
    } else {
        if (chdir(arge[1]) != 0) {
            perror("ERROR !!!");
        }
    }
    return 1;
}
/*
 *
 * This command with one integer parameter works like this below
 * >$ history 0 = will bring first command to screen
 *
 * example running
 *
 * >$ pwd
 * ....
 * >$ history 0
 * Your 0'th command was pwd
 * if you want to run call historyrun 0
 * >$
 *
 * NOTE : PLEASE DONT CALL THIS CALL WITHOUT INTEGEGER
 * history alone wont give you history !
 */
int history(char **arge)
{
    if(arge[1] == NULL) {
        perror("History: dont call history without argument\n");
    }
    else {
        printf("Your %d'th command was : %s\n", atoi(arge[1]), historyArray[atoi(arge[1])]);
        printf("if you want to run this command use like this :'historyrun %d'\n", atoi(arge[1]));
    }
    return 1;
}
int historyrun(char **args)
{
    pid_t pid, waitpidler;

    int status;
    char launch[1500];
    if(args[1] == NULL) {
        pid = fork();
        if (pid == 0) {
            /*  sprintf(launch,"./%s",historyArray[atoi(args[1])]);
              printf("---%s\n",launch);
              execvp(launch,args);
              */
            //sadece argumani yolla
            args = line_split(historyArray[atoi(args[1])]);
            //burda yeni args lazım mı bak bi
            execvp(args[0], args);

            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            // Error forking
            perror("ERROR forking on historyrun");
        } else {
            // Parent process
            do {
                waitpidler = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    else{
        perror("Historyrun : Dont call without argument integer !");
    }

    return 1;
}
/*
 * PWD Will give you process current working directory
 * Where is your workspace right now
 * error check encouraged
 */
int pwd_command()
{
    char current[2048];
    if (getcwd(current, sizeof(current)) != NULL)
        fprintf(stdout, "%s\n", current);
    else
        perror("getcwd -> error");
}
/*
 *
 * Help function will called
 * >$ help
 * will list all commands available to user with their respective calling styles
 * this is pretty helpful function .
 *
 */
int help()
{
    printf("Shell Script for System Programming\n");
    printf("to use just write and argument. Example :>$cat a.txt -> will print entire a.txt.\n");
    printf("These are implemented :\n");
    printf("CD : Change directory \n");
    printf("Will be used cd .. = previous directory(up), cd somewhere = change to somewhere\n");

    printf("PWD : Display current working directory \n");
    printf("called like : pwd\n");

    printf("CAT : display given argument on default output\n");
    printf("example usage : cat rapor2.txt = will display rapor2.txt to screen if not changed.\n");

    printf("LS: show all files\n");
    printf("example usage : ls = will show all files according to parameters\n");

    printf("> : directional right = into file \n");
    printf("used like : ls > a.txt = will write contents of stdout to a.txt \n");

    printf("< : directional left = will get from file \n");
    printf("used like : wc < a.txt = will get wc of a.txt to stdout.\n");

    printf("| : Pipe operation\n");
    printf("will pipe 2 process right left  cat filename | less  example\n");

    printf("help: will help all these \n");
    printf("just type help\n");

    printf("Exit \n");
    printf("just call exit to exit properly\n");


    return 1;

}

/*
 * exit will return 0 so loop will break
 *
 */
int exitcommand()
{
    return 0;
}

/*
 *This function will fork call following accordingly
 *  ls
 *  cat argument
 *  wc
 *   these 3 implemented and compiled and executable file appeared as ./ runnable
 *   if program find these in argument , it will call
 *   mine written LS  / CAT / WC instead of normal linux shell call
 *
 *   outside of these
 *   anything will work
 *
 *   for all processes done it will fork and execvp with arguments
 *   execvp will never return so i error checked if they return from function it makes program error
 *
 *   ls functions like ls -al but little less info according to pdf
 *   cat will work with argument example cat rapor2.txt
 *   wc will work like wc rapor2.txt
 *
 *   these also work with > = for example ls > abc.txt = will write file to abc.txt
 *   these also work with < = for example wc < abc.txt = will write wc on screen from file.
 *
 *
 *
 */

int launcher(char **arge)
{
    pid_t fork_pid, waitpidler;
    int status;
    char launch[1500];
    int i = 0;

    //This will provide ARGC without parameter itself checking null param
    //

    while(arge[i]!=NULL){
       // printf("%s\n",args[i]);
        i++;
    }

    fork_pid = fork();
    if (fork_pid == 0) {

        if (strcmp(arge[0],"ls") == 0 && arge[1] == NULL){
            execvp("./ls",arge);
        }
        if (strcmp(arge[1],">") == 0){
            // execvp("./cat",args);
            // fdo is file-descriptor
            int fide1 ;
            if ((fide1 = creat(arge[2] , 0644)) < 0) {
                perror("Launcher: > ERROR ON OUTPUT");
                exit(4);
            }


            dup2(fide1, STDOUT_FILENO);
            close(fide1);
            //ls cagirma burda
            execvp("./ls", arge); /* Does NOT return */
        }

        if (strcmp(arge[1],"<") == 0){

            int fide0;
            if ((fide0 = open(arge[2], O_RDONLY, 0)) < 0) {
                perror("Launcher: < ERROR ON OUTPUT");
                exit(0);
            }
            dup2(fide0, 0);
            close(fide0);
        }

        if (strcmp(arge[0],"cat") == 0){
            execvp("./cat",arge);
        }
        if (strcmp(arge[0],"wc") == 0){
            execvp("./wc",arge);
        }
        if (strcmp(arge[1],"|") == 0){
            int fd_in[2], fd_out[2];

            if (fd_in != NULL)
            {
                close(fd_in[1]);
                dup2(fd_in[0], 0);
            }
            if (fd_out != NULL)
            {
                close(fd_out[0]);
                dup2(fd_out[1],1);
            }
        }


        if (execvp(arge[0], arge) == -1) {
            perror("error on commands");
        }
        // ls > a.txt  = ls/>/command

        //should never reach there? ->
        exit(EXIT_FAILURE);
    } else if (fork_pid < 0) {
        perror("Launcher: ERROR ON LAUNCHER");
    } else {
        do {
            waitpidler = waitpid(fork_pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/*
 * This function will be called when line splitted and call
 * according functions without any problem
 * NOTE :
 * while CAT,LS,WC implemented as other file
 *
 * CD PWD HISTORY HISTORY RUN EXIT will be implemented in to core of shell
 * this is what project wanted.
 *  according shell calls will be explained how to use in help call
 *
 */
int execution(char **arge)
{
    if (arge[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    if (strcmp(arge[0],"cd") == 0)
        return cd_command(arge);
    else if (strcmp(arge[0],"help") == 0)
        return help();
    else if (strcmp(arge[0],"pwd") == 0)
        return pwd_command();
    else if (strcmp(arge[0],"history") == 0)
        return history(arge);
    else if (strcmp(arge[0],"historyrun") == 0)
        return historyrun(arge);
    else if (strcmp(arge[0],"exit") == 0)
        return exitcommand();


    return launcher(arge);
}

/*
 * This will read line to arguments
 * without any problem error check enabled on buffer
 * return line read.
 *
 */
char *line_read(void)
{
    int bufsize = 1024;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int chare;

    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(5);
    }

    while (1) {
        // Read a character
        chare = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (chare == '\n' || chare == EOF ) {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = chare;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += 1024;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "Lineread : ERROR ON this function call\n");
                exit(5);
            }
        }
    }
}

/*
 *
 * LINE SPLITTER NEEDEDD FOR REMOVAL OF UNNECESSARY THINGS LIKE SPACE
 * NEW SPACE
 * TAB
 * OR ANY INPUT ERROR WILL BE DELETED
 * ls > a txt will become
 * ls=argv 0
 * > = argv 1
 * a.txt =argv 2
 */
char **line_split(char *objLine)
{
    int bufsize = 64, position = 0;
    char **arrays = malloc(bufsize * sizeof(char*));
    char *satir;

    if (!arrays) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(0);
    }

    satir = strtok(objLine, DELIM);
    while (satir != NULL) {
        arrays[position] = satir;
        position++;

        if (position >= bufsize) {
            bufsize += 64;
            arrays = realloc(arrays, bufsize * sizeof(char*));
            if (!arrays) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(0);
            }
        }

        satir = strtok(NULL, DELIM);
    }
    arrays[position] = NULL;
    return arrays;
}

/*
 * Main function will work on loop unless big error accures
 * or exit called.
 *
 */
void main_test(void)
{
    signal(SIGINT,&sinyal_handler);
    signal(SIGTERM,&sinyal_handler);
    char *line;
    char **args;
    int status;
    int i=0;
    do {
        printf(">$ ");
        line = line_read();
        strcpy(historyArray[i],line);
        args = line_split(line);
        // printf("argumanlar siralanamali %s %s %s \n",args[0],args[1],args[2]);
        status = execution(args);
        i++;
        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv)
{
    printf("This is basic shell for system programming hw3\n");
    printf("Please first call help if you didnt use it  before to use\n");
    //signal(SIGINT,&sinyal_handler);
    main_test();
    return 1;
}

