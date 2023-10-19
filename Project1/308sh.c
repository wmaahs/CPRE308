#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>

pid_t getpid(void);

#define MAX_INPUT_SIZE 100


int main(int argc, char **argv) {

    /* string to hold prompt displayed to user */
    char prompt[256] = {'\0'};

    char input[MAX_INPUT_SIZE] = {'\0'};
    char command[MAX_INPUT_SIZE] = {'\0'};

    
    /* flag for if custom prompt inputed */
    int custPromptInd = -1;

    /* status variable for execvp */
    int status, status2;

    /* flag for background process */
    int bgf = 0;

    int i, k, rc, rc2;

    /* detect for custom prompt*/

    for (i = 0; i<argc; i++) {

        if (strcmp(argv[i], "-p") == 0) {
            custPromptInd = i + 1;
        }
    }

    /* set default prompt if custPrompt not set */
    if (custPromptInd != -1) {
            strcpy(prompt, strcat(argv[custPromptInd], ">>"));
        } else {
            strcpy(prompt, "308sh>>");
        }
    
    /* run loop */ 
    while(1) {

        printf(prompt);                         //output prompt
        fgets(input, MAX_INPUT_SIZE, stdin);    // get input from user and place in string input
        bgf = 0;

        /* Terminate command where actually ends */
        int lenCommand = strlen(input);
        
        input[lenCommand - 1] = '\0';

        /* copy input into command to split */
        for(i = 0; i < strlen(input) +1; i++){
            command[i] = input[i];
        }

        char *cmdName;
        char *argList;
        char *split = strchr(command, ' ');     //variable for location of space

        if (split != NULL) {
            int spaceLocation = (int)(split - command);

            command[spaceLocation] = '\0';

            cmdName = command;
            argList = split + 1;
        }
        else {
            cmdName = command;
            argList = NULL;
        }
        
        /* check for built-in commands */

        /* exit */
        if(strcmp(cmdName, "exit") == 0) {

            printf("Exit detecetd\n");
            printf("Exiting ... \n");
            break;
        }
        /* pid */
        else if(strcmp(cmdName, "pid") == 0) {
            __pid_t pid = getpid();
            printf("%d\n", pid);
        }
        /* ppid */
        else if(strcmp(cmdName, "ppid") == 0) {
            __pid_t ppid = getppid();
            printf("%d\n", ppid);

        }
        /* cd */
        else if(strcmp(cmdName, "cd") == 0) {
            if (argList != NULL) {

                for(i = 0; i < strlen(argList); i++) {
                    // add code to detect quotations for files with a space in the name
                    //maybe
                }
                char file[20];  //string for file name
                strcpy(file, argList);

                /* check if dir is child dir of cwd */
                if( chdir(argList) != -1) {
                    chdir(argList);
                }
                /* Print Error */
                else {
                    fprintf(stderr,"No such File or Directory: %s\n", file);
                }

            } else {
                chdir(getenv("HOME"));
            }

        }

        /* check if command is pwd */
        else if(strcmp(cmdName, "pwd") == 0) {
            char cwd[100];
            getcwd(cwd, 100);
            printf("%s\n", cwd);
        }

        


        /* Program Commands */
        else {

            int numArguments = 1;
            
            /* Scan through input, check for background proc and spaces*/
            for(i = 0; i < strlen(input); i++)
            {
                //  check for spaces in input
                if(input[i] == 32) {
                    numArguments++;
                }
            }

            /* Check for background proccess */
            if (input[strlen(input)-1] == 38)
            {
                bgf = 1;
                numArguments--;
            }
            

            char tempArg[numArguments][MAX_INPUT_SIZE];
            char *p;


            p =(char*)malloc(sizeof(tempArg));

            char *argsList[numArguments + 1];
            numArguments = 0;
            k = 0;

            for(i = 0; i <= strlen(input); i++) {

                /* after every space or null character, put the string tempArgs into argList */
                if((input[i] == 32 )||(input[i] == '\000')) {
                    tempArg[numArguments][k] = '\0';
                    argsList[numArguments] = tempArg[numArguments];
                    numArguments++;
                    k = 0;

                }
                /* If &, break and don't add to argslist */
                else if (input[i] == 38 ) {
                    tempArg[numArguments][k]='\0';
                    break;
                }

                /* Scan through each command and copy it into tempArg string*/
                else {
                    tempArg[numArguments][k] = input[i];
                    k++;
                }
                
            }

            argsList[numArguments] = '\0';

            free(p); 

            /* If background Process Flag = 1 */
            if(bgf == 1) {
                /* Create one child process */
                rc2 = fork();

                /* In child process, fork again */
                if(rc2 == 0) {
                    rc = fork();

                    /*child of child*/
                    if(rc == 0) {
                        /* print process ID */
                        printf("Background Process[%d], Command: %s\n", getpid(), argsList[0]);
                        /* execute in child of child */
                        execvp(argsList[0], argsList);
                    }
                    /* child waits for child child to finish */
                    else
                    {
                        usleep(1000);
                        /* initialize status and wait for child process to complete*/
                        status = -1;
                        if ((waitpid(rc, &status, 0)) == rc)
                        { // waiting

                            /*print the completed child process ID name and return status*/
                            printf("[%d] %s Background Exit Status: %d\n", rc, argsList[0], WEXITSTATUS(status));
                        }
                        /* Print Error if wait failed */
                        else
                        {
                            fprintf(stderr, "Exiting. Process Failed. Error Status: %d\n", WEXITSTATUS(status));
                        }
                    }
                }
                /* parent should wait, don't print anything when finished, only print on error*/
                else{
                    usleep(1000);
                    status2 = -1;
                    if((waitpid(rc2, &status, 0) == rc2)) {
                        break;
                    }
                    else {
                        fprintf(stderr, "Exiting. Process Failed. Error Status: %d\n", WEXITSTATUS(status));
                    }
                }
            }
            /* Background Process Flag == 0 
            Spawn Child process to execute command
            Parent process waits for child to complete */
            else{
            rc = fork();    // Create child process to execute the command

            
            /* Child Process */
            if(rc == 0)
            {
                //print name of command and pid
                printf("Command Name: %s\nPID: %d\n", argsList[0], getpid());
                /* execute command */
                execvp(argsList[0], argsList);

                /* If command could not be executed, print to error message to user */
                fprintf(stderr, "Could not exec command, No such file or directory \nExit Status: failed to execute\n");

            }
            /* Parent Process should wait for child to execute*/
            else
            {
                usleep(1000);
                /* initialize status and wait for child process to complete*/
                status = -1;
                if( (waitpid(rc, &status, 0)) == rc){ //waiting
                
                /*print the completed child process ID name and return status*/
                printf("[%d] %s Exit Status: %d\n", rc, argsList[0], WEXITSTATUS(status));
                }
                /* Print Error if wait failed */
                else {
                    fprintf(stderr, "Exiting. Process Failed. Error Status: %d\n", WEXITSTATUS(status));
                }

            }
            }

            
        }
    }
    

}

