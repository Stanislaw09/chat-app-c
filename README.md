## chat app in C  ¯\\\_(ツ)_/¯

App which allows to communicate between different users via server. The idea is that one server handles all requests coming from clients allowing them to send messages directly to another client or to the rooms. Creating and joining room is available from level of client. There is one server and each joining client has to provide correct name of it in order to join. Then each client has to send own name (unique one).

### instruction of usage 

to clean all old message queues and compile files run in terminal:

```makefile
make clean && make
```
run server:

```bash
./server
```

run client (each client in different terminal):

```bash
./client
```

### description of used files

#### server

main file which handles all request from clients and then executes proper functions to enable communication. It also stores and manages all the required structures. 

#### client

file can be run multiple time to create different clients. It gives the user ability to interact via options in menu with different clients connected to server. The main functionality is to send and receive messages but it also allows to create / join / leave the room, displaying last 10 messages in room or display all users in rooms stored in server. Taking input from user and receiving messages in asynchronous - client creates two processes. 

#### common

includes some functions which run during initialization and structure of messages

#### const

keeps constants, mainly types of messages which are send on line client-server