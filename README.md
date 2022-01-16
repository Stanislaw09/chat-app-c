# chat app in C  ¯\\\_(ツ)_/¯

App which allows users to communicate through a central server. The server handles all requests coming from clients, allowing them to send messages directly to one another or to a public chat room. Clients can view, create and join rooms, view (limited) chat history and see other connected users.

## Installation

To download and compile both `server` and `client` apps, run the following commands:

```bash
git clone https://github.com/Stanislaw09/chat-app-c.git
cd ./chat-app-c
make
```

## Usage
First, run the server app:

```bash
./server
```

Then, run clients (each client in a different terminal):

```bash
./client
```
## Troubleshooting
If you experience an unexpected error after force-stopping `server` or `client`, please run:
```bash
make clean && make
```


## Files in this repository

### `server.c`

Handles all requests from clients and then executes proper functions to enable communication. It also stores and manages all the required structures. 

### `client.c`

Can be run multiple time to create different clients. It gives the user ability to interact via options in menu with different clients connected to server. The main functionality is to send and receive messages but it also allows to create / join / leave a chat room, display the last 10 messages in a room or display all users in all rooms stored in server. Taking input from user and receiving messages in asynchronous - client creates two processes. 

### `common.h, common.c`

Contains functions and structures used both by server and client

### `const.h`

Stores shared constants

### [protocol.md](./protocol.md)
A technical description