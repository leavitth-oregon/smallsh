/*
*   Author: Hayley Leavitt
*   Assignment: Archive
*   Due Date: 2022/05/09
*   Description: Create a zip folding with directories and text files
*   To compile use: gcc -std=c99 -Wall -Wextra -Wpedantic -Werror -o archive archive.c
*   Last Edited: 2022/05/03
*/

// ----------------------------------------------------------- CITATIONS ------------------------------------------------------------ //

/*
 * CITATION: getline() for getting user input, sourced from man pages
 * SOURCE: https://man7.org/linux/man-pages/man3/getline.3.html
 * DATE: 2022/05/03
*/

/*
 * CITATION: parse() - for loop turning line input into separate words, inspired by w3 tutorials string parsing exercise
 * SOURCE: https://www.w3resource.com/c-programming-exercises/string/c-string-exercise-31.php
 * DATE: 2022/05/03
*/


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


// ----------------------------------------------------------- FUNCTIONS ------------------------------------------------------------ //

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
    int i;
    int j = 0; 
    int counter = 0;
    
    char * input = stdin;
    char * output = stdout;

    /* We're going to loop through the entire input line and break apart the arguments. */
    for (i = 0; i <= (strlen(line)); i++)
    {
        /* If the character is a space or the end of the line, then we've reached the end of the word. */
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

    /* So, each index in the first brackets will indicate an argument. The first argument will be a command, if there is one. */


    printf("Parsed arguments: \n");  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    for(i = 0; i < counter; i++)  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    {
        printf(" %s\n",args[i]);  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    }  

    /* 
     * If the tstp flag is not on, then background processes are enabled and a '&' at the end of input indicates 
     * that an input should be processesed in the background.
    */
    if ((!tstp) && (line[-1] == '&'))
    {
        run_background = true;
    }

    printf("Parsing input!\n"); // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    printf("%s\n", line);       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |

    /* Parse the input into separate words. The first word will be the command, and any that follow will be */

    printf("Finished.\n");  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |

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

int main(int argc, char *argv[])
{
    // local variables 
    int this_pid = getpid();

    command_loop();

    exit(exit_status);
}
