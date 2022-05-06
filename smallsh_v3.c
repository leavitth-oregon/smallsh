/*
*   Author: Hayley Leavitt
*   Assignment: Archive
*   Due Date: 2022/05/09
*   Description: Create a zip folding with directories and text files
*   To compile use: gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -o archive archive.c
*   Last Edited: 2022/05/03
*/

/* To Compile: gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -o smallsh smallsh.c */

// ----------------------------------------------------------- CITATIONS ------------------------------------------------------------ //


// ----------------------------------------------------------- LIBRARIES ------------------------------------------------------------ //

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>     // boolean data type, for convenience and familiarity
#include <sys/types.h>   // pid_t
#include <unistd.h>      // fork
#include <sys/wait.h>    // wait
#include <fcntl.h>       // fcntl - allows the changing of properties of a file currently in use


// ------------------------------------------------------------ GLOBALS ------------------------------------------------------------- //

int exit_status = 0;
bool tstp = false;            // TSTP controls whether or not background processes are currently allowed
int max_args = 512;           // maximum number of arguments to handle
int max_line_length = 2048;   // maximum number of chars per input line
int * bchildren[5000];        // array to hold all children running in the background


// ----------------------------------------------------------- FUNCTIONS ------------------------------------------------------------ //

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
    char str1[2050];        // Initialize the holder strings to the size of line length, rounded up for comfort. 
    char str2 [2050];       // str1 will hold the part of the string before $$, str2 will hold the part that comes after.
    unsigned long i;
    unsigned long k;
    unsigned long j = 0;

    /* Write the first part of the string, up to where the $$ is, and null terminate the string. */
    for ( i = 0; i < strlen(line); i++ )
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

    for ( k = i; k <= strlen(line); k++ )
    {
        str2[j] = line[k];
        j++;
    }
    str2[j] = '\0';

    /* Print out the first part of the string, the pid, and the second part of the string if it is not empty. */
    if (strlen(str2) > 0)    // strlen() does not count the null terminator.
    {
        sprintf("%s %d %s\n", str1, pid, str2);
    }
    else
    {
        sprintf("%s %d\n", str1, pid);
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
    bool run_background = false;
    char args [max_args][max_line_length];           // There will be up to 512 arguments; setting the size of the arguments to the max line length.
    unsigned long i;
    unsigned long j = 0; 
    unsigned long counter = 0;
    int pid = getpid();                              // the current process's pid (process id)

    /* Search the input for "$$" to see if we need to expand. */ 
    char * dollardollar = strstr(line, "$$");

    /* strstr() returns NULL if the substring is not found, so if dollardollar is NULL, there is no substring to expand. Otherwise, we expand. */
    if (dollardollar != NULL)
    {
        pid_expansion(line, pid);
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
            args[counter][j] = '\0';                // Set the end of the word in the args string.
            counter++;                              // Increment the counter to hold the start of the next argument.
            j = 0;                                  // Reset j to hold where the end of the next argument will be.
        }
        /* Otherwise, we're going to log the character into the args array */
        else
        {
            args[counter][j] = line[i];             // Insert the character. 
            j++;                                    // Increment j to keep track of where the last character will be. 
        }
    }

    /* Handle 'cd' command to change current working directory. */

    printf("Parsed arguments: \n");  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    for(i = 0; i < counter; i++)   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    {
        printf(" %s\n",args[i]);   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    }  

    /* 
     * If the tstp flag is not on, then background processes are enabled and a '&' at the end of input indicates 
     * that an input should be processesed in the background.
    */
    if ((!tstp) && (line[-1] == '&'))
    {
        run_background = true;
    }

    printf("Parsing input!\n"); // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    printf("%s\n", line);       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |

    /* Parse the input into separate words. The first word will be the command, and any that follow will be */

    printf("Finished.\n");  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |

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
    
    /* command loop */
    while(1)
    {
        /* Print the prompt character, ";". */
        printf(": ");

        /* Empty stdout so command prompts don't overfill. */
        fflush(stdout);

        /* Read input from the prompt. */
        nread = getline(&line, &input_size, stdin);

        /* if the input was too long, print a warning and continue. */
        if (nread > max_line_length)
        {
            printf("Too much input! Try again.\n");
        }

        /* If the command is 'exit', then return and exit. */
        if (strcmp(line, "exit\n") == 0)
        {
            return;
        }

        /* Ignore blank lines */
        else if (line[0] == '\n')
        {
            continue;
        }

        /* ignore the input if it is a comment, which starts with '#' */
        else if(line[0] == '#')
        {
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


// --------------------------------------------------------- MAIN CODE -------------------------------------------------------------- //

int main()
{
    command_loop();

    exit(exit_status);
}
