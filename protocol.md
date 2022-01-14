### description of used structures

All of the following structures used to keep data about client, rooms, queues are stored in server:

- `char client names` names of clients

- `char rooms names` names of rooms

- `char rooms_client_names` stores arrays of all users in rooms under the index corresponding to `rooms_names`

- `int client_queues[]` stores keys to queues for specific clients

- `int rooms_client_queues` stores arrays of queues' keys to clients in specific room with the index same as in `rooms_names`. Analogous to the `rooms_client_names`
- `int recent_room_messages_index` stores the index to replace proper cached message in room history

- `msbuf recent_room_messages` stores the last 10 messages for each room

General structure of message:

```c
msbuf {
   long type;
   char message[1024];
   char option[256];
   char client[256];
};
```

where `type` is one of the available types of messages, `message` is the content of message if needed, `option` is general field to send config data between server and client like name of the room, name of the recipient and so on, `client` is name of the message sender, alike in sending and receiving. Of course in some cases not all fields were needed.

### description of way server communicates with clients

After signing into the server (under condition that name of server and client are valid) the queues based on these names are created. Server stores them in array, so data for each of the clients are stored in different arrays under the same index what allows to connect information between them without additional structures.

As mentioned in README there are 3 levels of messages types:

- 0-9 is used by client to send messages to server, so it need only to listen type `-9`
- 10-17 is used by server to send messages to clients, in order to not interfere with another clients, each one has own queue stored in server. If server needs to sends message to many clients it iterates through the array with queues' keys to clients.
- types 20-22 are send from server to client and correspond to different types of messages (from client-private, from room the user belongs to, and cached messages from room)

In essence the user sends and receive messages with the flag `0` to make sure that current task was processed correctly before going next, whereas the server in most cases utilizes the flag `IPC_NOWAIT` because there is no need to wait for the client