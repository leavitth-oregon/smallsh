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
 * CITATION: fork() code to produce children in parse()
 * SOURCE: Module 4 Exploration: Environment, subsection "Environment Variables Across Parent and Child Process"
 * DATE: 2022/05/06
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
 * @param pid 
 */
void exit_process(int pid)
{
    /* Loop through all currently running child processes and kill them. */
    while(waitpid(-1, &exit_status, WNOHANG) != -1)
    {
        kill(pid, SIGINT);
    }
    return;
}


/**
 * @brief pid_expansion() takes in a pointer to the location of where "$$" begins and replaces $$ with the current process id, 
 *        then returns the same line, but with the pid expanded.
 * 
 * @param dollardollar
 * @param line 
 * @return char*
 */
void pid_expansion(char * line, int pid)
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

    /* Print out the first part of the string, the pid, and the second part of the string if it is not empty. */
    sprintf(line, "%s%d%s", str1, pid, str2);
    return;
}



void change_directory(char * argv)
{
    int cd_status = 0;
    /* If no argument is specified, change the directory to the HOME directory, otherwise change the  */

    printf("Changing directory to %s\n", argv); // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE 
    cd_status = chdir(argv);

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
    // FILE * input = stdin; 
    // FILE * output = stdout;
    // bool run_background = false;
    char argv [max_args][max_line_length];           // There will be up to 512 arguments; setting the size of the arguments to the max line length.
    size_t argc = 0;                                 // argc holds the size of argv.
    unsigned long i;
    unsigned long j = 0; 
    unsigned long counter = 0;
    int pid = getpid();


    /* Search the input for "$$" to see if we need to expand. */ 
    char * dollardollar = strstr(line, "$$");

    /* strstr() returns NULL if the substring is not found, so if dollardollar is NULL, there is no substring left to expand. Otherwise, we expand. */
    if (dollardollar != NULL)
    {
        /* Go ahead and handle "echo $$"" and  "$$". It's more concise this way and saves us a function call. */
        if (strcmp(line, "$$") == 0)
        {
            printf("%d\n", pid);
            return;
        }

        /* Otheriwse, expand line to include pid instead of $$ */
        pid_expansion(line, pid);

        /* Recursively call parse to process the expanded line. Return to avoid parsing the incorrect version of line. */
        parse(line);
        printf("%s\n", line);
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
        exit_process(pid);
    }


    /* Return exit_status or status of last run process. */
    else if (strcmp(argv[0], "status") == 0)
    {

    }


    /* Use exec family of functions to run other commands */
    else
    {
        pid_t childPid = fork();
        switch(childPid)
        {
            case -1: 
                perror("fork() failed.\n");
                exit(1);
                break;

            case 0: 
                // child process
                system_command(line, argc, pid);
                break;
            
            default:
                break;
        }


        alarm(600);   // Automatically kill any runaway children.
    }


    /* 
     * If the tstp flag is not on, then background processes are enabled and a '&' at the end of input indicates 
     * that an input should be processesed in the background.
    */
    if ((!tstp) && (line[-1] == '&'))
    {
        // run_background = true;
    }


    /* Parse the input into separate words. The first word will be the command, and any that follow will be */


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
    
    /* command loop */
    while(1)
    {
        /* Print the prompt character, ";". */
        printf(": ");

        /* Empty stdout so command prompts don't overfill. */
        fflush(stdout);

        /* Read input from the prompt. */
        nread = getline(&line, &input_size, stdin);

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



