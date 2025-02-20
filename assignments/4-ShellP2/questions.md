1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**: fork() creates a new process so the shell can continue running while the command executes. Without fork(), execvp() would replace the shell itself.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**: If fork() fails, it returns -1, meaning no child process was created. My code prints perror("fork failed") and returns an error code.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  It searches directories listed in the PATH environment variable for the executable.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  wait() prevents zombie processes by ensuring the parent collects the child's exit status. Without it, the shell might spawn multiple processes and lose control.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  It extracts the child's exit code, allowing the shell to determine if a command succeeded or failed.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  It treats quoted strings as a single argument, preserving spaces inside quotes while trimming extra spaces outside. If an argument is inside quotes (e.g., "hello world"), it is treated as one argument instead of two. Spaces between arguments are collapsed, but spaces inside quotes remain unchanged.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  Improved handling of spaces, proper termination of quoted arguments, and detection of mismatched quotes. Biggest challenge: handling escaped quotes.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals allow asynchronous process control (e.g., stopping or killing a process) without explicit communication.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**: SIGKILL (9): Forces process termination (cannot be caught).
SIGTERM (15): Requests termination (allows cleanup).
SIGINT (2): Interrupts process (e.g., Ctrl+C).


- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  It suspends a process and cannot be ignored or handled. The process can only resume with SIGCONT.