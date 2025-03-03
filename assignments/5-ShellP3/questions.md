1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

_answer here_
waitpid() waits for all child processes to finish. Without it, the shell could continue accepting input while child processes run, causing errors.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

_answer here_
Closing unused pipes prevents file descriptor leaks and ensures proper communication. Leaving them open could cause resource leaks or deadlocks.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

_answer here_
cd affects the shell’s working directory, requiring it to be built-in. An external cd would only change the child process’s directory, not the shell’s.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

_answer here_
Use dynamic memory allocation for pipes and commands. Trade-offs include added complexity in memory management and potential performance overhead.
