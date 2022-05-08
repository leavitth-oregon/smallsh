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
 * CITATION: parse() - for loop turning line input into separate words, inspired by w3 tutorials string parsing exercise
 * SOURCE: https://www.w3resource.com/c-programming-exercises/string/c-string-exercise-31.php
 * DATE: 2022/05/03
*/

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



void foreground_process(char ** exec_argv, int exec_argc, int shellpid) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
{
    return;
}



void redirect_output(char * output) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
{

}



void redirect_input(char * input) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
{

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
void parse(char * line)
{
    /* local variables */
    char argv [max_args][max_line_length];           // There will be up to 512 arguments; setting the size of the arguments to the max line length.
    size_t argc = 0;                                 // argc holds the size of argv.
    unsigned long i;
    unsigned long j = 0; 
    unsigned long counter = 0;
    int shellpid = getpid();

    /* Go ahead and handle just  "$$". It's more concise this way and saves us a function call. */
        if (strcmp(line, "$$") == 0)
        {
            printf("%d\n", shellpid);
            return;
        }

    /* 
     * We're going to loop through the entire input line and break apart the arguments. We break at spaces and \0.
     * Each index in the first brackets will indicate the argument number, and the first argument will be a command, if there is one.
    */
    for (i = 0; i <= strlen(line); i++)
    {
        if((line[i] == ' ') || (line[i] == '\0'))
        {
            argc++;
            argv[counter][j] = '\0';                // Set the end of the word in the argv string.
            counter++;                              // Increment the counter to hold the start of the next argument.
            j = 0;                                  // Reset j to hold where the end of the next argument will be.
        }
        /* Otherwise, we're going to log the character into the argv array */
        else
        {
            argv[counter][j] = line[i];             // Insert the character. 
            j++;                                    // Increment j to keep track of where the last character will be. 
        }
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


    /* If "exit", then run exit_process to kill all processes */
    else if (strcmp(argv[0], "exit") == 0)
    {
        exit_process(shellpid);
    }


    /* Return exit_status or status of last run process. */
    else if (strcmp(argv[0], "status") == 0) // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
    {

    }


    /* The command will be run by exec. */
    else
    {
        /* local variables */
        int redirecting = 0;         // For if we need to do I/O redirection.
        char * exec_argv[argc + 1];  // Array to pass to exec with the correct values.
        int exec_argc = 0;           // Count of arguments in exec_argv.


        /* Set default SIGINT action for child processes. */ // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||



        /* Prepare the input for exec. We want to handle input and output, and remove ampersands. */
        int k;
        for (k = 0; k < argc; k++)
        {
            /* Skip ampersands. */
            if (strcmp(argv[k], "&") == 0)
            {
                continue;
            }

            /* Do input redirection. */
            else if (strcmp(argv[k], "<") == 0)
            {
                redirect_input(argv[k + 1]);  // The next argument is where we want to redirect from.
                redirecting = 1;
            }

            /* Do output redirection. */
            else if (strcmp(argv[k], ">") == 0)
            {
                redirect_output(argv[k + 1]); // The next argument is where we want to redirect to.
                redirecting = 1;
            }

            else if (!redirecting)
            {
                /* Search the input for "$$" and expand if needed. */
                if (strstr(line, "$$") != 0)
                {
                    pid_expansion(line, shellpid);
                }

                /* Add arguments to an array that we'll pass to exec with the now-correct values. */
                exec_argv[exec_argc] = argv[k];
                exec_argc++;
            }
            
        }

        /* If the last argument is an ampersand, and tstp is off, remove the ampersand and run the process in the background. */
        if ((strcmp(exec_argv[-1], "&") == 0) && (!tstp))
        {
            exec_argv[-1] = NULL;
            // background_process(exec_argv, exec_argc, shellpid); - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - TO DO - - - |||
        }

        /* Otherwise, run it in the foreground. */
        else 
        {
            if (strcmp(exec_argv[-1], "&") == 0) // Remove any unecessary ampersands. 
            {
                exec_argv[-1] = NULL;
            }

            foreground_process(exec_argv, exec_argc, shellpid);
        }
    }

    return;
}



/**
 * @brief command_loop() is a function that runs the command-taking section of the program. 
 * The function also handles the exit command, and handles the skipping of blank lines and comments.
 * 
 * command_loop() takes no parameters and returns void. 
 */
void command_loop(void)
{
    /* local variables: */
    size_t input_size   = 0;
    int nread;
    char *line       = NULL;
    int pid = getpid();


    /* Handle SIGTSTP, Ctrl+Z, sigaction struct. */
    // struct sigaction SIGTSTP_action = {0};
    // SIGTSTP_action.sa_handler = catchSIGTSTP; 
    // sigfillset(&SIGTSTP_action.sa_mask);
    // SIGTSTP_action.sa_flags = 0;
    // sigaction(SIGTSTP, &SIGTSTP_action, NULL);

    /* 
       Initialize ignore SIGINT sigaction struct. 
       The parent will ignore SIGINT, but all children will be terminated.
    */
    // struct sigaction ignore_sigint = {0};
    // ignore_sigint.sa_handler = SIG_IGN; 
    // sigaction(SIGINT, &ignore_sigint, NULL);

    
    /* command loop */
    while(1)
    {
        /* Print the prompt character, ";". */
        printf(": ");

        /* Empty stdout so command prompts don't overfill. */
        fflush(stdout);

        /* Read input from the prompt. */
        nread = getline(&line, &input_size, stdin);

        /* If there was an error, clear the error */
        if (nread == -1)
        {
            clearerr(stdin);
        }

        /* Ignore blank lines */
        if (line[0] == '\n')
        {
            continue;
        }

        /* ignore the input if it is a comment, which starts with '#' */
        else if(line[0] == '#')
        {
            continue;
        }

        /* if the input was too long, print a warning and continue. */
        else if (nread > max_line_length)
        {
            printf("Line is too long.\n");
            continue;
        }

        /* Then, if there is input, remove the newline character and null terminate the string, then call parse() to parse the input */
        else if((line != NULL))
        {
            line[strlen(line) - 1] = '\0';
            parse(line);
        }

        /* flush stdin so an overfull buffer does not negatively impact future line reading */
        fflush(stdin);

    }

    return;
}




// ----------------------------------------------------------- MAIN CODE ------------------------------------------------------------- //

int main()
{
    int childStatus;

    command_loop();
    exit(exit_status);
}



