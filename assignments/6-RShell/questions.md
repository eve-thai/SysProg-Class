1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

_answer here_
- The server sends an EOF character (e.g., \0) after completing the command output. The client detects this character to determine the end of the transmission.
Technique:
- Use a loop to repeatedly call recv() until the EOF character is received.
- Use a fixed-size buffer and check the last byte for EOF after each recv() call.

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?

_answer here_
- Since TCP is stream-oriented, the shell protocol define the begining by prefixing message with their lenghth, and define the end of a command by using delimiter (EOF character) to mark the end of a message
- If this is not handled correctly, messages maybe fragmented or concatenated, leading to incomplete or corrupted data

3. Describe the general differences between stateful and stateless protocols.

_answer here_
- Stateful: maintains the session state between requests. Requires tracking client connections and context
- Stateless: treats each request independently. Easier to scale but lacks built-in session management.

4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?

_answer here_
UDP is faster and lower overhead than TCP, suitable for real-time app, or app where occasional packet loss is acceptable

5. What interface/abstraction is provided by the operating system to enable applications to use network communications?

_answer here_
the socket API