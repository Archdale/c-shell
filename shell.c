#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define USER_INPUT_LEN (2048)
#define ARG_NUM (256)

/*
 *  Program that acts as a shell for Linux
 *  Handles firing up processes and directory
 *  management.
 */

int main(int argc, char* argv[])
{
    /* Variable declarations */
    char userInput[USER_INPUT_LEN];
    char* commandArgs[ARG_NUM]; /* Max number */
    char* buffer;
    
    /*
     * We need a place to store the directory we're in, the 
     * previous directory, and a buffer for swapping them.
     */
    char currWorkDir[USER_INPUT_LEN];
    char prevWorkDir[USER_INPUT_LEN];
    char buffWorkDir[USER_INPUT_LEN];
    
    // Keep track of the number of arguments
    int numOfArgs = 0;
    
    // Keep track of the ID of a child when we fork
    pid_t childPid = 0;
    
    // Initialize the prev Working Dir with the current
    // So that it isn't null.
    getcwd(prevWorkDir, sizeof(prevWorkDir));
    
    while(1) /* Repeat forever */
    {
        // Reset after every loop
        numOfArgs = 0;
        childPid = 0;
                
        /* Working Environment Dir */
        getcwd(currWorkDir, sizeof(currWorkDir));
        
        /* Display a prompt */
        fprintf(stdout, "%s%% ", currWorkDir);
        fflush(stdout);
        
        /* Read a command from the user */
        fgets(userInput, sizeof(userInput), stdin);
        
        // This line gets rid of the \n that is hanging at
        // the end of our input
        userInput[strcspn(userInput, "\n")] = 0;
        
        

        /* Parse the string into the command portion and
         * an array of pointers to argument strings using the blank
         * character as the separator. Note: the first argument
         * in the array of argument strings must be the same as
         * the command and the array must have a NULL pointer
         * after the last argument.
         */
        
        buffer = strtok(userInput, " \n");       
        commandArgs[0] = buffer;
        
        // While we still have things to parse
        while(buffer)
        {
            if (buffer)
            {
                // Won't count the null, so (numOfArgs-1) is the loc
                // of the last arg, and numOfArgs is how many
                numOfArgs++;
            }
            buffer = strtok(NULL, " ");  
            // Even if strtoke returns null, we want to add it to the arguments
            // So that it is null terminated.
            commandArgs[numOfArgs] = buffer;
            
        }
        
                
        /*
         * As long as there is an input, we're going to handle it
         * If we didn't put anything in, we're going to skip and
         * reprint the prompt.
         */
        if(commandArgs[0])
        {
            
            /*
             * If the user entered 'exit' then call the exit() system call
             * to terminate the process
             */
            if(!strcmp(commandArgs[0],"exit"))
            { 
                exit(EXIT_SUCCESS);
            }

            /* 
             * If the user entered 'cd' then we have to handle changing the PWD
             */
            else if(!strcmp(commandArgs[0],"cd"))
            {
                // Switch between current and previous dir
                if(!commandArgs[1] || !strcmp(commandArgs[1], "-"))
                {
                    strcpy(buffWorkDir,prevWorkDir);
                    strcpy(prevWorkDir,currWorkDir);
                    strcpy(currWorkDir,buffWorkDir);
                    chdir(currWorkDir);
                }
                // HOME
                else if(!strcmp(commandArgs[1], "~"))
                {
                    getcwd(prevWorkDir, sizeof(prevWorkDir));
                    chdir(getenv("HOME"));
                }
                // Absolute Path and ".." command
                else if(commandArgs[1])
                {
                    getcwd(prevWorkDir, sizeof(prevWorkDir));
                    chdir(commandArgs[1]);
                    getcwd(currWorkDir, sizeof(currWorkDir));                   
                }
                
                setenv("PWD", currWorkDir, 1);
                setenv("OLDPWD", prevWorkDir, 1);
                
                // Cleanly handle a non-existant function
                // This will be thrown by chdir if its set
                if (errno)
                {
                    fprintf(stderr, "ERROR: %s\n", strerror(errno));
                    errno = 0;
                }
            }
            
             /* Fork a child process to execute the command and return 
             * the result of the fork() in the childPid variable so 
             * we know whether we're now executing as the parent 
             * or child and whether or not the fork() succeeded
             */
            else
            {
                // Fork and if we're the parent, it won't be zero
                childPid = fork();
                
                if (!childPid) /* We forked no child, we ARE the child */
                {
                    /* We're now executing in the context of the child process.
                     * Use execvp() or execlp() to replace this program in 
                     * the process' memory with the executable command the 
                     * user has asked for.  
                     */
                    
                    // If the last argument is an &, overwrite it
                    // With a NULL and run as detached.
                    if(!strcmp(commandArgs[numOfArgs-1],"&"))
                    {
                        commandArgs[numOfArgs-1] = NULL;                       
                        // This removes us from the same session as the parent
                        setsid();                       
                    } 
                    
                    execvp(commandArgs[0], commandArgs);
                    
                    // Cleanly handle a non-existant function
                    if (errno)
                    {
                        fprintf(stderr, "ERROR: %s\n", strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    exit(EXIT_SUCCESS);
                }
                
                // If the fork failed for some reason
                else if (childPid == -1) 
                {
                    /* An error occured during the fork - print it */
                    fprintf(stderr, "ERROR: %s\n", strerror(errno));
                }
                
                // If we're the parent
                else /* childPid is the PID of the child */
                {
                    /* We're still executing in the parent process.
                     * Wait for the child to finish before we prompt
                     * again.
                     */
                    if(strcmp(commandArgs[numOfArgs-1], "&"))
                    {
                        waitpid(childPid, NULL, 0);
                    }
                    // If its a background process, we don't wait
                    // But print the PID
                    else
                    {
                        fprintf(stdout, "Job %d\n", childPid);
                    }
                }
            }
        }  
    } /* while */
    return 1;
} /* my shell */
