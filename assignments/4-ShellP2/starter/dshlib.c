#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

 void trim_spaces(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) *end-- = '\0';

    memmove(str, start, end - start + 2);
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    trim_spaces(cmd_line); 

    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;

    cmd_buff->argc = 0;
    char *token = cmd_buff->_cmd_buffer;
    bool in_quotes = false;

    while (*token) {
        while (isspace((unsigned char)*token) && !in_quotes) *token++ = '\0';

        if (*token == '"') {
            in_quotes = !in_quotes;
            token++;
            cmd_buff->argv[cmd_buff->argc++] = token;
            while (*token && (in_quotes || !isspace((unsigned char)*token))) {
                if (*token == '"') {
                    in_quotes = !in_quotes;
                    *token = '\0'; // Null terminate quoted string
                }
                token++;
            }
            continue;
        }

        if (*token) {
            cmd_buff->argv[cmd_buff->argc++] = token;
            if (cmd_buff->argc >= CMD_ARGV_MAX - 1) return ERR_CMD_OR_ARGS_TOO_BIG;
        }

        while (*token && (!isspace((unsigned char)*token) || in_quotes)) {
            token++;
        }
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;
    return OK;
}



Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
    if (cmd->argc == 0) return BI_NOT_BI;

    if (strcmp(cmd->argv[0], EXIT_CMD) == 0) {
        free_cmd_buff(cmd);
        exit(0);
    } else if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argc > 1) {
            if (chdir(cmd->argv[1]) != 0) {
                perror("cd failed");
            }
        }
        return BI_EXECUTED;
    } else if (strcmp(cmd->argv[0], "dragon") == 0) {
        printf("You have summoned the dragon! \U0001F409\n");
        print_dragon();
        return BI_EXECUTED;
    }

    return BI_NOT_BI;
}

int exec_cmd(cmd_buff_t *cmd) {
    if (cmd->argc == 0) return WARN_NO_CMDS;

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return ERR_EXEC_CMD;
    }

    if (pid == 0) { // Child process
        execvp(cmd->argv[0], cmd->argv);
        perror("exec failed");
        exit(ERR_EXEC_CMD);
    }

    // Parent process waits for child to finish
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}


int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
    return OK;
}



int exec_local_cmd_loop() {
    char *cmd_buffer;
    int rc = 0;
    cmd_buff_t cmd;

    // Allocate command buffer
    cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buffer == NULL) {
        perror("Can't allocate command buffer");
        return ERR_MEMORY;
    }

    while (1) {
        printf("%s", SH_PROMPT);
        if (!fgets(cmd_buffer, SH_CMD_MAX, stdin)) {
            printf("\n");
            break;
        }

        // Remove newline character
        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

        // Build command buffer
        rc = build_cmd_buff(cmd_buffer, &cmd);
        if (rc != OK) {
            if (rc == ERR_MEMORY) {
                free(cmd_buffer);
                return ERR_MEMORY;
            } else if (rc == WARN_NO_CMDS) {
                printf(CMD_WARN_NO_CMD);
                continue;
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                continue;
            }
        }

        // Execute built-in commands
        if (exec_built_in_cmd(&cmd) == BI_EXECUTED) {
            free_cmd_buff(&cmd);
            continue;
        }

        // Execute external commands
        exec_cmd(&cmd);

        free_cmd_buff(&cmd);
    }

    free(cmd_buffer);
    return OK;
}

