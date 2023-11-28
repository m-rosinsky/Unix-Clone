/*!
 * @file src/bash.c
 *
 * @brief This file contains the bash program source.
 * 
 *          The purpose of this program is to provide a command-line
 *          interface for the user to run the repo's binaries in a
 *          bash-style interface.
 * 
 *          Features:
 *              - Up and Down arrows to scroll through command history.
 *              - TAB autocompletion for files.
 *              - Display of the current working directory.
 * 
 *          Options:
 *              - 
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

// Default number of commands stored in console history.
#define DEFAULT_HISTORY_MAXLEN 30

// Default command maximum length.
#define DEFAULT_CMD_MAXLEN 1024

/*!
 * @brief This datatype defines a console context.
 *
 * @param old_console Old termios config.
 * @param new_console New termios config.
 * @param pp_history The command history queue.
 * @param history_max The maximum size of the history queue.
 */
typedef struct _console
{
    struct termios old_console;
    struct termios new_console;
    char ** pp_history;
    size_t history_max;
} console_t;

/*!
 * @brief This function intializes the console termios config.
 *
 * @param[in/out] p_console Pointer to the console context.
 * 
 * @return 0 on success, -1 on error.
 */
static int
console_initialize (console_t * p_console)
{
    int status = -1;
    if (NULL == p_console)
    {
        goto EXIT;
    }

    // Save the current console configuration.
    if ((-1 == tcgetattr(STDIN_FILENO, &(p_console->old_console))) ||
        (-1 == tcgetattr(STDIN_FILENO, &(p_console->new_console))))
    {
        goto EXIT;
    }

    // Configure flags.
    p_console->new_console.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    p_console->new_console.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    p_console->new_console.c_oflag &= ~(OPOST);
    p_console->new_console.c_cflag |= (CS8);

    // Apply new settings.
    if (-1 == tcsetattr(STDIN_FILENO, TCSAFLUSH, &(p_console->new_console)))
    {
        goto EXIT;
    }

    // Set success status.
    status = 0;

    EXIT:
        return status;
}

/*!
 * @brief This function restores the console termios config.
 *
 * @param[in/out] p_console Pointer to the console context.
 */
static void
console_finalize (console_t * p_console)
{
    if (NULL == p_console)
    {
        goto EXIT;
    }

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &(p_console->old_console));

    EXIT:
        return;
}

/*!
 * @brief This function is the main command loop for the console.
 * 
 * @param[in/out] p_console Pointer to the console context.
 */
static void
console_run (console_t * p_console)
{
    // Initialize the console.
    if ((NULL == p_console) ||
        (-1 == console_initialize(p_console)))
    {
        goto EXIT;
    }

    EXIT:
        // Finalize the console settings before exiting.
        console_finalize(p_console);
        return;
}

/*!
 * @brief This is the entry-point of the program.
 */
int
main (int argc, char * argv[])
{
    console_t console = {
        .history_max = DEFAULT_HISTORY_MAXLEN,
    };

    console_run(&console);

    return 0;
}

/***   end of file   ***/
