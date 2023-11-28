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
#include <string.h>
#include <termios.h>
#include <unistd.h>

// Versioning.
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

// Default number of commands stored in console history.
#define DEFAULT_HISTORY_MAXLEN 30

// Default command maximum length.
#define DEFAULT_CMD_MAXLEN 1024

// Key macros
#define KEY_CTRL(k) ((k) & 0x1F)
#define KEY_ENTER 13
#define KEY_ESCAPE 27
#define KEY_BACKSPACE 127

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
 * @brief This function gets a single command from stdin.
 *
 * @param[in/out] p_console Pointer to the console context.
 * @param[in/out] p_cmd The command buffer.
 * 
 * @return -1 on error that should exit the program.
 *          1 on command cancel, such as CTRL-C.
 *          0 otherwise.
 */
static int
get_cmd (console_t * p_console, char * p_cmd)
{
    int status = -1;
    if ((NULL == p_console) ||
        (NULL == p_cmd))
    {
        goto EXIT;
    }

    // Input characters. Escape sequences hold 2 additional characters.
    char c = '\0';
    char e1 = '\0'; // escape char 1.
    char e2 = '\0'; // escape char 2.

    // This is a buffer to hold a copy of the current command if the user
    // has typed something, but then begins scrolling through command
    // history.
    char p_cmd_copy[DEFAULT_CMD_MAXLEN];
    memset(p_cmd_copy, '\0', DEFAULT_CMD_MAXLEN);

    // The current length of the command in the buffer.
    size_t cmd_len = 0;

    // The current position within the buffered command.
    size_t cmd_idx = 0;

    // The current index within the console's history.
    // -1 is the current command (meaning not in history).
    size_t hist_idx = -1;

    for (;;)
    {
        c = '\0';

        // Read a byte off stdin.
        if (-1 == read(STDIN_FILENO, &c, 1))
        {
            goto EXIT;
        }

        switch (c)
        {
            case KEY_CTRL('c'):
                // CTRL-C
                printf("^C");
                status = 1;
                goto EXIT;
            break;
            case KEY_CTRL('d'):
                // CTRL-D
                goto EXIT;
            break;

            case KEY_ENTER:
                status = 0;
                goto EXIT;
            break;

            default:
                // Bounds check.
                if (cmd_len >= DEFAULT_CMD_MAXLEN) { break; }

                // Print the character.
                printf("%c", c);

                // Shift all characters in buffer back one.
                for (size_t i = cmd_len; i > cmd_idx; --i)
                {
                    p_cmd[i] = p_cmd[i - 1];
                }

                // Update command buffer and indices.
                p_cmd[cmd_idx] = c;
                cmd_len++;
                cmd_idx++;

                // Print the characters that follow.
                for (size_t i = cmd_idx; i < cmd_len; ++i)
                {
                    printf("%c", p_cmd[i]);
                }

                // Return cursor to position.
                for (size_t i = cmd_idx; i < cmd_len; ++i)
                {
                    printf("\b");
                }
                fflush(stdout);
            break;
        }
    }

    EXIT:
        return status;
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

    // Display the welcome banner.
    printf("Mike's bash clone v%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR);

    // The command buffer.
    char p_cmd[DEFAULT_CMD_MAXLEN];
    char prompt_c = '$';
    
    // Main command loop.
    for (;;)
    {
        // Clear the command buffer.
        memset(p_cmd, '\0', DEFAULT_CMD_MAXLEN);

        // Display the prompt.
        printf("%c ", prompt_c);
        fflush(stdout);

        // Get a command.
        int status = get_cmd(p_console, p_cmd);
        printf("\r\n");
        if (0 > status)
        {
            // Exit.
            fflush(stdout);
            break;
        }
        else if (1 == status)
        {
            // Cancel command.
            if (0 == strlen(p_cmd))
            {
                printf("Press CTRL-D to exit.\r\n");
            }
            continue;
        }

        // Add command to history.

        // Parse command.

        // Handle command.
        printf("'%s'\r\n", p_cmd);
    }

    printf("Exiting...\r\n");

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
