
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <ctype.h>


//INCLUDES for extra credit
#include <signal.h>
#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"


/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;

    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0) {
        int err_code = svr_socket;  // Server socket will carry error code
        return err_code;
    }

    rc = process_cli_requests(svr_socket, is_threaded);

    stop_server(svr_socket);

    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
int boot_server(char *ifaces, int port){
    int svr_socket;
    struct sockaddr_in server_addr;
    int enable = 1;

    //Create socket
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    //Set socket option to reuse addy
    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
        perror("setsockopt");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    //Bind socket to specific interface and port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ifaces);
    server_addr.sin_port = htons(port);

    if (bind(svr_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    //Listen
    if (listen(svr_socket, 5) < 0) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */

 // Multiple threads helper
 void *client_handler(void *arg) {
    thread_args_t *targs = (thread_args_t *)arg;
    int client_socket = targs->client_socket;
    free(targs);

    exec_client_requests(client_socket);

    close(client_socket);

    return NULL;
}

int process_cli_requests(int svr_socket, int is_threaded) {
    int client_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int rc;

    while (1) {
        // Step 1a: Accept a client connection
        client_socket = accept(svr_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }

        if (is_threaded) {
            // Step 1b: Multi-threaded - Create a new thread for each client
            pthread_t thread_id;
            thread_args_t *targs = malloc(sizeof(thread_args_t));
            if (!targs) {
                perror("malloc");
                close(client_socket);
                continue;
            }

            targs->client_socket = client_socket;
            if (pthread_create(&thread_id, NULL, client_handler, targs) != 0) {
                perror("pthread_create");
                free(targs);
                close(client_socket);
                continue;
            }

            // Detach the thread to auto-cleanup when it exits
            pthread_detach(thread_id);
        } else {
            // Step 1c: Single-threaded - Process client requests sequentially
            rc = exec_client_requests(client_socket);
            close(client_socket);

            // Step 1d: Check return code for stopping server
            if (rc == OK_EXIT) {
                break;  // Stop the server if requested
            } else if (rc < 0) {
                return rc;  // Return error code
            }
        }
    }

    // Step 2: Cleanup and stop the server
    stop_server(svr_socket);
    return OK_EXIT;
}


/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */
int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (!io_buff) {
        perror("malloc");
        return ERR_RDSH_COMMUNICATION;
    }

    while (1) {
        // Receive the command from the client
        ssize_t bytes_received = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ - 1, 0);
        if (bytes_received < 0) {
            perror("recv");
            break;
        } else if (bytes_received == 0) {
            // Client disconnected
            break;
        }

        // Null-terminate the received command
        io_buff[bytes_received] = '\0';

        // Parse the command into a command list
        command_list_t cmd_list;
        if (build_cmd_list(io_buff, &cmd_list) < 0) {
            // Handle command parsing errors
            break;
        }

        // Execute the command pipeline
        int rc = rsh_execute_pipeline(cli_socket, &cmd_list);

        // Free allocated memory for the command list
        free_cmd_list(&cmd_list);

        // Send EOF to the client
        if (send_message_eof(cli_socket) < 0) {
            free(io_buff);
            return ERR_RDSH_COMMUNICATION;
        }

        // Check for special commands
        if (rc == OK_EXIT) {
            // Client sent `stop-server`
            break;
        } else if (rc == OK) {
            // Client sent `exit`
            break;
        }
    }

    // Cleanup
    free(io_buff);
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

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
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


/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){

    int send_len = (int)sizeof(RDSH_EOF_CHAR);

    int sent_len;

    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

 

    if (sent_len != send_len){

        return ERR_RDSH_COMMUNICATION;

    }

    return OK;

}


/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
int send_message_string(int cli_socket, char *buff){
    int bytes_sent = send(cli_socket, buff, strlen(buff), 0);
    if (bytes_sent < 0) {
        return ERR_RDSH_COMMUNICATION; // Error sending the message
    }

    // Step 2: Send the EOF character
    if (send_message_eof(cli_socket) < 0) {
        return ERR_RDSH_COMMUNICATION; // Error sending EOF
    }

    return OK;
}


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int socket_fd, command_list_t *clist) {
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
                dup2(socket_fd, STDERR_FILENO);
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

    // Send EOF message to indicate end of command execution
    int rc = send_message_eof(socket_fd);
    if (rc != OK) {
        printf(CMD_ERR_RDSH_COMM);
        close(socket_fd);
        return ERR_RDSH_COMMUNICATION;
    }

    return OK;
}



_Bool is_append_redirect(const char *output_file) {
    return (output_file && output_file[0] == '>' && output_file[1] == '>');
}

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 */
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0) {
        return BI_CMD_EXIT;
    } else if (strcmp(input, "dragon") == 0) {
        return BI_CMD_DRAGON;
    } else if (strcmp(input, "cd") == 0) {
        return BI_CMD_CD;
    } else if (strcmp(input, "rc") == 0) {
        return BI_CMD_RC;
    } else if (strcmp(input, "stop-server") == 0) {
        return BI_CMD_STOP_SVR;
    } else {
        return BI_NOT_BI; // Not a built-in command
    }
}

/*
 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 */
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds cmd_type = rsh_match_command(cmd->argv[0]);

    switch (cmd_type) {
        case BI_CMD_CD:
            if (cmd->argc < 2) {
                // No directory provided, default to home directory
                chdir(getenv("HOME"));
            } else {
                chdir(cmd->argv[1]);
            }
            return BI_EXECUTED;

        case BI_CMD_EXIT:
            return BI_CMD_EXIT;

        case BI_CMD_STOP_SVR:
            return BI_CMD_STOP_SVR;

        case BI_CMD_DRAGON:
        case BI_CMD_RC:
            // Handle other built-in commands here
            return BI_EXECUTED;

        default:
            return BI_NOT_BI; // Not a built-in command
    }
}
