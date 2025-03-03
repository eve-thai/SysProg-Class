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
 int exec_local_cmd_loop() {
    char cmd_buffer[SH_CMD_MAX];
    command_list_t clist;
    int rc;

    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buffer, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }


        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';

        if (*cmd_buffer == '\0') {
            printf(CMD_WARN_NO_CMD);
            continue;
        }

        if (strcmp(cmd_buffer, EXIT_CMD) == 0) {
            break;
        }

        // Parse the command line into a command list
        rc = build_cmd_list(cmd_buffer, &clist);
        switch (rc) {
            case WARN_NO_CMDS:
                printf(CMD_WARN_NO_CMD);
                continue;
            case ERR_TOO_MANY_COMMANDS:
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                continue;
            case ERR_MEMORY:
                fprintf(stderr, "Memory allocation error.\n");
                continue;
            default:
                break;
        }

        // Execute the parsed command pipeline
        if (execute_pipeline(&clist) == ERR_EXEC_CMD) {
            printf(CMD_ERR_EXECUTE);
        }

        // Free allocated memory for the command list
        free_cmd_list(&clist);
    }

    return OK;
}

int execute_pipeline(command_list_t *clist) {
    if (clist->num == 0) return ERR_EXEC_CMD;

    int pipes[CMD_MAX - 1][2];
    pid_t pids[CMD_MAX];

    for (int i = 0; i < clist->num; i++) {
        // Create pipe (except for the last command)
        if (i < clist->num - 1) {
            if (pipe(pipes[i]) == -1) {
                perror("pipe");
                return ERR_EXEC_CMD;
            }
        }

        // Fork a child process
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            return ERR_EXEC_CMD;
        }

        if (pids[i] == 0) {  // Child process
            // Handle input redirection
            if (clist->commands[i].input_file) {
                int in_fd = open(clist->commands[i].input_file, O_RDONLY);
                if (in_fd == -1) {
                    perror("open input file");
                    exit(EXIT_FAILURE);
                }
                dup2(in_fd, STDIN_FILENO);
                close(in_fd);
            } else if (i > 0) {
                dup2(pipes[i - 1][0], STDIN_FILENO);
            }

            // Handle output redirection
            if (clist->commands[i].output_file) {
                int out_fd;
                if (is_append_redirect(clist->commands[i].output_file)) {
                    // Handle `>>` (append mode)
                    out_fd = open(clist->commands[i].output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                } else {
                    // Handle `>` (overwrite mode)
                    out_fd = open(clist->commands[i].output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }

                if (out_fd == -1) {
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                dup2(out_fd, STDOUT_FILENO);
                close(out_fd);
            } else if (i < clist->num - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // Close all pipes in child process
            for (int j = 0; j < clist->num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Execute command
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    }

    // Close all pipes in the parent process
    for (int i = 0; i < clist->num - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all child processes
    for (int i = 0; i < clist->num; i++) {
        waitpid(pids[i], NULL, 0);
    }

    return OK;
}



_Bool is_append_redirect(const char *output_file) {
    return (output_file && output_file[0] == '>' && output_file[1] == '>');
}



int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
    return OK;
}


int free_cmd_list(command_list_t *cmd_lst) {
    for (int i = 0; i < cmd_lst->num; i++) {
        if (free_cmd_buff(&cmd_lst->commands[i]) != OK) {
            return ERR_MEMORY;
        }
    }
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    trim_whitespace(cmd_line);

    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;

    cmd_buff->argc = 0;
    cmd_buff->input_file = NULL;
    cmd_buff->output_file = NULL;
    
    char *token = cmd_buff->_cmd_buffer;
    bool in_quotes = false;

    while (*token) {
        while (isspace((unsigned char)*token) && !in_quotes) *token++ = '\0';


        if (*token == '<') {
            token++;
            trim_whitespace(token);
            cmd_buff->input_file = token;
            while (*token && !isspace((unsigned char)*token)) token++;
            *token = '\0';
            continue;
        }


        if (*token == '>') {
            token++;
            trim_whitespace(token);
            cmd_buff->output_file = token;
            while (*token && !isspace((unsigned char)*token)) token++;
            *token = '\0';
            continue;
        }

        if (*token == '"') {
            in_quotes = !in_quotes;
            token++;
            cmd_buff->argv[cmd_buff->argc++] = token;
            while (*token && (in_quotes || !isspace((unsigned char)*token))) {
                if (*token == '"') {
                    in_quotes = !in_quotes;
                    *token = '\0';  // Null terminate quoted string
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


int build_cmd_list(char *cmd_line, command_list_t *clist) {
    if (!cmd_line || *cmd_line == '\0') return WARN_NO_CMDS;

    memset(clist, 0, sizeof(command_list_t));

    trim_whitespace(cmd_line);

    char *token, *saveptr;
    int cmd_count = 0;

    token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while (token) {
        trim_whitespace(token);

        if (*token == '\0') {
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }

        if (cmd_count >= CMD_MAX) return ERR_TOO_MANY_COMMANDS;

        if (build_cmd_buff(token, &clist->commands[cmd_count]) != OK)
            return ERR_MEMORY;

        cmd_count++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    clist->num = cmd_count;
    return (cmd_count > 0) ? OK : WARN_NO_CMDS;
}

void trim_whitespace(char *str) {
    if (!str || *str == '\0') return;

    char *start = str;
    while (isspace((unsigned char)*start)) start++;

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) *end-- = '\0';

    memmove(str, start, end - start + 2);
}
