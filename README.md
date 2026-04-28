*This project has been created as part of the 42 curriculum by eel-abed and mafourni*

# ft_irc - Internet Relay Chat Server

## Description
The **ft_irc** project aims to recreate a fully functional Internet Relay Chat (IRC) server in C++98. It is designed to handle multiple simultaneous client connections using non-blocking I/O operations and the `poll()` system call.

The server allows users to authenticate, join chat channels, and communicate with each other in real-time. It acts as a lightweight, compliant implementation of the IRC protocol, serving as a solid introduction to network programming, socket management, and client-server architectures in C++.

### Key Features
- **Non-blocking I/O**: Handles multiple clients efficiently on a single thread using `poll()`.
- **Client Authentication**: Processing of `PASS`, `NICK`, and `USER` commands.
- **Channel System**: Users can join (`JOIN`), leave (`PART`), and interact within chat rooms.
- **Messaging**: Peer-to-peer and channel broadcasting using the `PRIVMSG` command.
- **Resource Management**: Safe memory handling and safe exits on UNIX signals (e.g., `SIGINT`).

## Instructions

### Compilation
A `Makefile` is provided to compile the project. To build the server, simply run:
```bash
make
```
This will compile the source code using the `-std=c++98` flag and create the `ircserv` executable.

### Execution
The server requires two arguments to run: a port number and a connection password.
```bash
./ircserv <port> <password>
```
*Example:*
```bash
./ircserv 6667 1234
```

### Connection
You can test the server using standard tools like `netcat` or specialized IRC clients like `irssi` or `weechat`.

**Using Netcat:**
```bash
nc -C 127.0.0.1 6667
```
*(Note: the `-C` flag is very important as it appends `\r\n` (CRLF) to the end of your inputs, complying with the IRC RFC).*

Once connected via netcat, authenticate by typing:
```text
PASS 1234
NICK my_nickname
USER my_user 0 * :My Real Name
```

## Resources
Here are the references and tools used to build this project:
- [RFC 2812 - Internet Relay Chat: Client Protocol](https://datatracker.ietf.org/doc/html/rfc2812): The absolute reference for IRC rules and syntaxes.
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/): A must-read to understand sockets, `bind()`, `listen()`, and `poll()`.
- [FT_IRC MEDIUM ARTICLE](https://medium.com/@mohcin.ghalmi/irc-server-internet-relay-chat-bd08e4f469f8): A detailed write-up of the implementation process, challenges faced, and solutions found.

### AI Usage
Artificial Intelligence (GitHub Copilot via VS Code) was used as an interactive tutor and debugging assistant during the development of this project. Specifically, AI assisted in:
- **Protocol Understanding:** Explaining arcane RFC 2812 requirements, such as why the `USER` command requires a colon (`:`) before the real name, and the necessity of `\r\n` (CRLF) terminations.
- **Architecture Insights:** Clarifying the difference between blocking and non-blocking I/O, and the inner workings of the `poll()` function.
- **Debugging & Memory Management:** Analyzing Valgrind reports to track down "still reachable" memory upon exit, and suggesting the implementation of a clean `SIGINT` (Ctrl+C) signal handler to properly destroy the `Server` object and prevent memory leaks.
