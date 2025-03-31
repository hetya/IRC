# IRC Server

## Overview
This project implements an Internet Relay Chat (IRC) server in C++. The goal is to understand the fundamentals of network programming, socket communication, and the IRC protocol.

## Features
- User authentication
- Channels and messaging
- Operator privileges
- Basic IRC commands (/join, /nick, /msg, /kick, etc.)

## Installation
### Prerequisites
- Make
- C++

### Compilation
``` sh
make
```
## Commands
- /nick <nickname>: Change nickname
- /user <username> <hostname> <servername> <realname>: Register user
- /join <#channel>: Join a channel
- /part <#channel>: Leave a channel
- /privmsg <target> :<message>: Send a private message
- /quit :<reason>: Disconnect from the server
- /kick <#channel> <user>: Kick a user from a channel (operator only)

### Running the Server
``` sh
./ircserv <port> <password>
```

Example:
``` sh
./ircserv 1234 mypassword
```

## Usage
1. Connect using Netcat:
``` sh
   nc 127.0.0.1 1234
```

2. Register with the server:
``` sh
   PASS password
   NICK mynickname
   USER myusername 0 * :Real Name
```

4. Join a channel:
``` sh
   JOIN #mychannel
```

5. Send a message:
``` sh
   PRIVMSG #mychannel :Hello, IRC!
```

6. Quit:
``` sh
   QUIT :Goodbye!
```
