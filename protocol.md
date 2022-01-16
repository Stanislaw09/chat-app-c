## Description of used structures

All of the following structures used to keep data about client, rooms, queues are stored in server:

- `char* client_names[]` stores names of all clients connected to server

- `char* rooms_names[]` stores names of chat rooms available on server

- `char* rooms_client_names[][]` stores names of clients connected to a specific chat room

- `int client_queues[]` stores keys to message queues of all clients

- `int rooms_client_queues[][]` stores keys to message queues of clients connected to a specific chat room

- `msbuf recent_room_messages[][]` stores the last 10 messages for each room, used as a circular buffer

- `int recent_room_messages_index[]` indicates the next message to be overwritten for a specific room in `recent_room_messages`; 


General structure of message:

```c
msbuf {
   long type;
   char message[1024];
   char option[256];
   char client[256];
};
```

* `type` is one of the available types of message
* `message` keeps the actual message content
* `option` is a general field to send other data like room name, recipient name, etc.
* `client` represents the sender's name

Not all fields are used in all types of messages.

## Description of client-server communication logic

Once booted up, the server creates a message queue and listens for incoming messages. Clients can access this queue's ID based on server name (see `get_id_from_string` in common.c) and use it to send a join request. The client's name is validated by server to elliminate possible collisions and then used to obtain its message queue. This way, `client->server` communication is realised through the server's queue while `server->client` communication uses individual queues, each corresponding to a single client. All the message queues mentioned above are used as one-way only.

There are 3 categories of messages types (see `const.h`):

- 0-9 are used by client to send messages and requests to server
- 10-17 are used by server to send non-message data to clients
- 20-22 are sent from server to client and correspond to different types of messages (private, room/public, and cached)

Clients often use the `msgsnd/msgrcv` functions in blocking mode (__msgflag set to `0`) as they need to wait for a confirmation message before proceeding with the next operation, whereas the server in most cases utilizes the `IPC_NOWAIT` flag, operating in a fire-and-forget pattern