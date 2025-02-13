1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  fgets() is a good choice for reading user input in our shell because it safely reads a full line from stdin, including spaces, and ensures we do not exceed the buffer size.

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  Using malloc() allows us to dynamically allocate memory based on the expected input size, preventing stack overflow if the buffer size is large. A fixed-size array declared on the stack might lead to inefficient memory use or limited input capacity.


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  Trimming spaces ensures that commands and arguments are stored correctly and prevents execution issues. If we don’t remove leading spaces, the shell might interpret the command incorrectly or fail to find the executable. Trailing spaces could lead to unnecessary empty arguments, potentially causing errors or unexpected behavior when passing parameters.

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:   Example 1: command > output.txt – Redirects STDOUT to a file.
                    Example 2: command 2> error.txt – Redirects STDERR to a file.
                    Example 3: command < input.txt – Redirects STDIN from a file.

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  Redirection modifies where STDIN, STDOUT, or STDERR are sent, such as writing output to a file instead of the terminal. Piping (|) connects the output of one command to the input of another, allowing for real-time data processing.

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:   Keeping STDERR separate from STDOUT allows error messages to be displayed independently of normal output, making debugging easier. If errors were mixed into STDOUT, filtering the correct output from logs or piped commands would be challenging. 

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  Our shell should capture the exit status of executed commands and provide meaningful error messages. If a command outputs both STDOUT and STDERR, we can offer the user the option to merge them using 2>&1, redirecting errors to standard output.