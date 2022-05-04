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
bool tstp = false;          // TSTP controls whether or not background processes are currently allowed


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


    /* struct for user input to be parsed into by parse() */
    struct parsed_input
    {
        char * command;
        int arg_count;
        char * arg_variables[512];    // argv_variables has a max size of 512, as per the project requirements
    };

    /* 
     * if the tstp flag is not on, then background processes are enabled and a '&' at the end of input indicates 
     * that an input should be processesed in the background
    */
    if ((!tstp) && (line[-1] == '&'))
    {
        run_background = true;
    }

    printf("Parsing input!\n"); // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |
    printf("%s\n", line);       // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |

    int line_size = strlen(line);   
    printf("Size of line is: %d\n", line_size); // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - REMOVE - - - - - |

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
        if (nread > 2048)
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
