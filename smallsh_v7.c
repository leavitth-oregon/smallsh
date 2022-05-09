/*
*   Author: Hayley Leavitt
*   Assignment: Archive
*   Due Date: 2022/05/09
*   Description: Create a zip folding with directories and text files
*   To compile use: gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -o smallsh smallsh.c
*   Last Edited: 2022/05/03
*/


// ----------------------------------------------------------- CITATIONS ------------------------------------------------------------ //
/*
 * CITATION: use of sprintf() for integer-to-string parsing, since itoa is not standard
 * SOURCE: User Matt J on StackOverflow https://stackoverflow.com/questions/190229/where-is-the-itoa-function-in-linux
 * DATE: 2022/05/04
*/


/*
 * CITATION: fork() code to produce children, spawn_pid, in parse()
 * SOURCE: Module 4 Exploration: Environment, subsection "Environment Variables Across Parent and Child Process"
 * DATE: 2022/05/06
*/

/* 
 * CITATION: general knowledge of fork() and exec functions
 * SOURCE: https://github.com/angrave/SystemProgramming/wiki/Forking%2C-Part-1%3A-Introduction
 * SOURCE: https://github.com/angrave/SystemProgramming/wiki/Forking,-Part-2:-Fork,-Exec,-Wait
 *     -> It's a 2-part Wiki on Github
 * DATE: 2022/05/06
*/

/* 
 * CITATION: custom signal handlers for SIGINT and SIGTSTP
 * SOURCE: Module 5 Exploration: Signal Handling API, subsection "Example: Catching & Ignoring Signals"
 * DATE: 2022/05/07
*/

/*
 * CITATION: Use of strtok to split line into words
 * SOURCE: User rlib on Stackoverflow, https://stackoverflow.com/questions/15472299/split-string-into-tokens-and-save-them-in-an-array 
 * DATE: 2022/05/08
*/ 

/*
 * CITATION: General knowledge and use of execvp() 
 * SOURCE: https://www.journaldev.com/40793/execvp-function-c-plus-plus
 * DATE: 2022/05/09
*/

/*
 * CITATION: waitpid() code
 * SOURCE: https://man7.org/linux/man-pages/man2/wait.2.html
 * 2022/05/08
*/ 


// ----------------------------------------------------------- LIBRARIES ------------------------------------------------------------ //
#define _POSIX_C_SOURCE 200809L // signals, getline()

#include <fcntl.h>       // fcntl - allows the changing of properties of a file currently in use
#include <signal.h>      // kill()
#include <stdbool.h>     // boolean data type, for convenience and familiarity
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>   // pid_t
#include <sys/wait.h>    // wait
#include <unistd.h>      // fork


// ------------------------------------------------------------ GLOBALS ------------------------------------------------------------- //
int exit_status = 0;
bool tstp = false;            // TSTP controls whether or not background processes are currently allowed
int max_args = 512;           // maximum number of arguments to handle
int max_line_length = 2048;   // maximum number of chars per input line
int * bchildren[5000];        // array to hold all children running in the background
int status;                  // the child exit status to return


// ----------------------------------------------------------- FUNCTIONS ------------------------------------------------------------ //
/**
 * @brief exit_process() handles the SIGINT and exit commands, killing all child processes and then terminating the parent process
 * 
 * @param shellpid 
 */
void exit_process(int shellpid)
{
    /* Loop through all currently running child processes and kill them. */
    while(waitpid(-1, &exit_status, WNOHANG) != -1)
    {
        kill(shellpid, SIGINT);
    }

    /* Exit out of the program */
    exit(0);
}


/**
 * @brief pid_expansion() takes in a pointer to the location of where "$$" begins and replaces $$ with the current process id, 
 *        then returns the same line, but with the shellpid expanded.
 * 
 * @param dollardollar
 * @param line 
 * @return char*
 */
void pid_expansion(char * line, int shellpid)
{
    /* local variables */
    char str1[2050];       // Initialize the holder strings to the size of line length, rounded up for comfort. 
    str1[0] = '\0';        // set string to start as empty
    char str2[2050];       // str1 will hold the part of the string before $$, str2 will hold the part that comes after.
    str2[0] = '\0';
    unsigned long i;
    unsigned long k;
    unsigned long j = 0;
    unsigned long slen = strlen(line);

    /* Write the first part of the string, up to where the $$ is, and null terminate the string. */
    for ( i = 0; i < slen - 1; i++ )
    {
        if ((line[i] == '$') && (line[i+1] == '$'))
        {
            break;
        }

        str1[i] = line[i];
    }
    str1[i+1] = '\0';

    /* Write the latter part of the string, after where $$ ends, and null terminate the string. */
    i = i+2;   // Add 2 to skip the $$.

    for ( k = i; k < slen; k++ )
    {
        str2[j] = line[k];
        j++;
    }
    str2[j] = '\0';

    /* Print out the first part of the string, the shellpid, and the second part of the string if it is not empty. */
    sprintf(line, "%s%d%s", str1, shellpid, str2);
    return;
}


/**
 * @brief foreground_process() runs the forking for a foreground child process 
 * 
 * @param command 
 * @param arguments 
 * @param argc 
 * @param shellpid 
 */
void foreground_process(char ** arguments, int argc, int shellpid) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
{
    /* local variables */
    int wstatus;  // child exit status
    int ces_num;            // child exit status number
    int child_signal;       // signal that may kill the child
    bool redirecting = false;         // For if we need to do I/O redirection.
    int i = 0;
    int w = 0;

    char * command = arguments[0];

    /* Call fork and create a child process. */
    int spawn_pid;
    spawn_pid = fork();

    switch(spawn_pid)
    {
        case -1: // Fork Failed. 
            printf("Fork() failed.\n");
            fflush(stdout);
            exit(1);
            break;

        case 0: // Child process.

            printf("Command = %s\n", command);
            /* SIGINT has effect now */ // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||

            /* Check for input and output redirection, and handle it with calls. */
            // for (i = 0; i < argc; i++)
            // {
                /* Do input redirection. */
                // else if (strcmp(arguments[k], "<") == 0)
                // {
                //     redirect_input(arguments[k + 1]);  // The next argument is where we want to redirect from.
                //     redirecting = true;
                // }

                /* Do output redirection. */
                // else if (strcmp(arguments[k], ">") == 0)
                // {
                //     redirect_output(arguments[k + 1]); // The next argument is where we want to redirect to.
                //     redirecting = true;
                // }
            // }

            /* Call Exec */
            execvp(command, arguments);
            /* Print an error */
            printf("Exec failed!\n");
            fflush(stdout);
            exit(2);
            break;

        default:
            /* Block Parent until child or children are done */
            do 
            {
                w = waitpid(spawn_pid, &wstatus, 0);

                if (w == -1) 
                {
                    printf("waitpid()\n");
                    fflush(stdout);
                    exit(EXIT_FAILURE);
                } 
                /* If the process exited, we want to get the child's exit status and cast it to our status variable. */
                if (WIFEXITED(wstatus))
                {
                    status = WEXITSTATUS(wstatus);
                    printf("exited, status=%d\n", WEXITSTATUS(wstatus));
                    fflush(stdout);
                }

                /* Else, if the process was terminated, write the signal that killed the child to our status buffer */
                else if (WIFSIGNALED(wstatus))
                {
                    child_signal = WTERMSIG(wstatus);
                    printf("killed by signal %d\n", WTERMSIG(wstatus));
                    fflush(stdout);
                }
                
                /* Else, if the process was stopped, write the stop signal */
                else if (WIFSTOPPED(wstatus)) 
                {
                    child_signal = WTERMSIG(wstatus);
                    printf("stopped by signal %d\n", WSTOPSIG(wstatus));
                    fflush(stdout);
                } 
                
                /* Else, if continued, write that it was continued */
                else if (WIFCONTINUED(wstatus)) 
                {
                    printf("continued\n");
                    fflush(stdout);
                }
            }
            while (!WIFEXITED(wstatus) && !WIFSIGNALED(wstatus));

            break;

    }

    return;
}


// void redirect_output(char * output) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
// {

// }



// void redirect_input(char * input) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
// {

// }

/**
 * @brief prep() handles input and output redirection, and determines if a program should be run in the foreground or background, and calls the respective function to run the command.
 * 
 * @param line 
 * @param argv 
 * @param argc 
 * @param shellpid 
 * @param try_bg 
 */
void prep(char * line, char ** argv, int argc, int shellpid, bool try_bg)
{
    /* If the last argument is an ampersand, and tstp is off, remove the ampersand and run the process in the background. */
    if ((try_bg) && (!tstp))
    {
        printf("Running as background.\n");
        // background_process(argv, argc, shellpid); - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
    }

    /* Otherwise, run it in the foreground. */
    else 
    {
        foreground_process(argv, argc, shellpid);
    }
    return;
}


/**
 * @brief change_directory() changes the current working directory to that specified. If no directory is specified the cwd is 
 * changed to the user's HOME.
 * 
 * @param dir 
 */
void change_directory(char * dir)
{
    int cd_status = 0;
    /* If no argument is specified, change the directory to the HOME directory, otherwise change the  */

    cd_status = chdir(dir);

    if(cd_status == -1)
    {
        printf("Failed to change directory.\n");
        fflush(stdout);
    }

    return;
}


/**
 * @brief parse() takes the user input in from command_loop() and splits the string into separate words, storing these words as
 *        commands and arguments. It also looks among the arguments for '<' which handles input redirection, '>' which handles
 *        output redirection, and '&' which indicates that a process should be run in the background if tstp is false. 
 * 
 * @param line 
 */
void parse(char * line, int shellpid)
{
    /* local variables */
    /* local variables */
    int argc = 0; 
    int i = 0;
    int j = 1;
    int q = 0;
    bool try_bg = false;

    char * argv[max_args];
    char * command;
    
    char * token = strtok(line, " "); 

    /* If "exit", then run exit_process to kill all processes */
    if (strcmp(line, "exit") == 0)
    {
        exit_process(shellpid);
    }

    /* Search the input for "$$" and expand pid if needed. */
    else if (strstr(line, "$$") != 0)
    {
        pid_expansion(line, shellpid);
    }

    /* Break the line into words. The first word will be the command. */
    while ((token != NULL) && (token != "\0") && (token != "\n"))
    {
        argv[i++] = token;
        token = strtok(NULL, " ");
        argc++; 
    }
    argv[argc] = NULL;

    /* If the last argument is &, remove it, decrement argc, and set try_bg to true */
    if (strcmp(argv[argc-1],"&") == 0)
    {
        printf("Try Background!\n");
        try_bg = true;
        argv[argc-1] = NULL;
        argc--;
    } 

    /* Identify cd for changing directory. If cd failed, the result will be -1. On success the result will be 0. */
    if (strcmp(argv[0], "cd") == 0)
    {
        if (argc == 1)
        {
            char * home = getenv("HOME");
            chdir(home);
        }
        else
        {
            change_directory(argv[1]);
        }
        return;
    }

    /* Return exit_status or status of last run process. */
    else if (strcmp(argv[0], "status") == 0) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
    {
        printf("%d\n", exit_status);
        fflush(stdout);
    }

    /* The command will be run by exec functions. */
    else
    {
        prep(line, argv, argc, shellpid, try_bg);    
    }

    return;
}


/**
 * @brief command_loop() is a function that runs the command-taking section of the program. 
 * The function also handles the exit command, and handles the skipping of blank lines and comments.
 * 
 * command_loop() takes no parameters and returns void. 
 */
void command_loop()
{
    /* local variables: */
    size_t input_size   = 0;
    int nread;
    char *line       = NULL;
    int shellpid = getpid();
    
    /* command loop */
    while(1)
    {
        /* Print the prompt character, ";" and empty stdout so command prompts don't overfill. */
        printf(": ");
        fflush(stdout);

        /* Read input from the prompt. */
        nread = getline(&line, &input_size, stdin);

        /* If there was an error, clear the error */
        if (nread == -1)
        {
            clearerr(stdin);
            fflush(stdin);
        }

        /* Ignore blank lines and ignore the input if it is a comment, which starts with '#' */
        if ((line[0] == '\n') || (line[0] == '#'))
        {
            fflush(stdin);
            continue;
        }

        /* if the input was too long, print a warning and continue. */
        else if (nread > max_line_length)
        {
            printf("Line is too long.\n");
            fflush(stdin);
            fflush(stdout);
            continue;
        }

        /* Go ahead and handle just "$$". It's more concise this way and saves us a function call. */
        else if ((strcmp(line, "$$") == 0) || (strcmp(line, "$$\n") == 0))
        {
            printf("%d\n", shellpid);
            fflush(stdout);
            fflush(stdin);
            free(line);
            line = NULL;
            return;
        }
        
        /* Then, if there is input, remove the newline character and null terminate the string, then call parse() to parse the input */
        else if((line != NULL))
        {
            line[strlen(line) - 1] = '\0';
            parse(line, shellpid);
        }

        /* flush stdin so an overfull buffer does not negatively impact future line reading */
        fflush(stdin);
        free(line);
        line = NULL;
    }

    return;
}


// ----------------------------------------------------------- MAIN CODE ------------------------------------------------------------- //
int main()
{
    command_loop();
    exit(exit_status);
}