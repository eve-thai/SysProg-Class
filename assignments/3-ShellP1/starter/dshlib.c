#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
/**
 * Trims leading and trailing spaces from a string in-place.
 */
void trim_whitespace(char *str)
{
    char *end;

    // Trim leading spaces
    while (*str == SPACE_CHAR)
        str++;

    // Trim trailing spaces
    if (*str == '\0') return;  // Empty string case

    end = str + strlen(str) - 1;
    while (end > str && *end == SPACE_CHAR)
        end--;

    *(end + 1) = '\0'; // Null-terminate the string
}

/**
 * Parses the command line and populates command_list_t.
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (cmd_line == NULL || strlen(cmd_line) == 0)
        return WARN_NO_CMDS;

    memset(clist, 0, sizeof(command_list_t)); // Clear clist structure

    char *token;
    char *saveptr;
    int cmd_count = 0;

    // Split input line into commands using '|'
    token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while (token != NULL)
    {
        if (cmd_count >= CMD_MAX)
            return ERR_TOO_MANY_COMMANDS;

        trim_whitespace(token); // Remove spaces around command

        if (strlen(token) == 0)
        {
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }

        // Parse command executable and arguments
        char *arg_ptr;
        arg_ptr = strchr(token, SPACE_CHAR); // Find first space

        if (arg_ptr != NULL)
        {
            *arg_ptr = '\0'; // Split executable and arguments
            arg_ptr++;       // Move to start of args
        }

        // Validate lengths
        if (strlen(token) >= EXE_MAX || (arg_ptr && strlen(arg_ptr) >= ARG_MAX))
            return ERR_CMD_OR_ARGS_TOO_BIG;

        // Store in clist
        strcpy(clist->commands[cmd_count].exe, token);
        if (arg_ptr)
            strcpy(clist->commands[cmd_count].args, arg_ptr);
        else
            clist->commands[cmd_count].args[0] = '\0'; // No args case

        cmd_count++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr); // Next command
    }

    clist->num = cmd_count;

    return (cmd_count > 0) ? OK : WARN_NO_CMDS;
}