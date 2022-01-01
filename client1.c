#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

//messages from client
#define JOIN 1
#define JOIN_ROOM 2
#define CREATE_ROOM 3
#define MESSAGE_TO_ROOM 4
#define MESSAGE_TO_CLIENT 5
//messages from server
#define OK 10
#define USER_EXISTS 11
#define NO_SUCH_ROOM 12
#define ROOM_EXISTS 13
#define WRONG_RECIPIENT 14
//types of messages
#define MESSAGE_FROM_CLIENT 20
#define MESSAGE_FROM_ROOM 21

typedef struct
{
   long type;
   char message[1024];
   char option[256];
   char client[256];
} msbuf;

char server_name[256], client_name[256];

int join_server(int to_server, int from_server)
{
   printf("enter server's name: ");
   scanf("%s", server_name); //for now it does nothing xd
   printf("\nenter client name: ");
   scanf("%s", client_name);

   msbuf sendMsg;
   sendMsg.type = JOIN;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.option, "");
   strcpy(sendMsg.client, client_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), -19, 0);
   printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      printf("\nOK\n");
      return 1;
   }
   else if (getMsg.type == USER_EXISTS)
   {
      printf("\nsuch name already exists\n");
      return 0;
   }
   else
   {
      printf("\nSth has fucked up...  ╯︿╰\n");
      return 0;
   }
}

int join_room(int to_server, int from_server)
{
   char room_name[256];
   printf("\nenter room name: ");
   scanf("%s", room_name);

   msbuf sendMsg;
   sendMsg.type = JOIN_ROOM;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.client, client_name);
   strcpy(sendMsg.option, room_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), -19, 0);
   printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      printf("\nOK\n");
      return 1;
   }
   else if (getMsg.type == NO_SUCH_ROOM)
   {
      printf("\nno such room\n");
      return 0;
   }
   else
   {
      printf("\nSth has fucked up...  ╯︿╰\n");
      return 0;
   }
}

int create_room(int to_server, int from_server)
{
   char room_name[256];
   printf("\nenter room name to create: ");
   scanf("%s", room_name);

   msbuf sendMsg;
   sendMsg.type = CREATE_ROOM;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.client, client_name);
   strcpy(sendMsg.option, room_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), -19, 0);
   printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      printf("\nOK\n");
      return 1;
   }
   else if (getMsg.type == ROOM_EXISTS)
   {
      printf("\nsuch room already exists\n");
      return 0;
   }
   else
   {
      printf("\nSth has fucked up...  ╯︿╰\n");
      return 0;
   }
}

void send_message(int to_server, int from_server)
{
   int recipient_type;
   char recipient_name[256], message[1024];

   printf("enter message: ");
   getchar();
   scanf("%1023[^\n]", message);

   printf("\nchoose recipient 1-room  2-private \n");
   scanf("%d", &recipient_type);

   printf("\nenter recipient name \n");
   scanf("%s", recipient_name);

   msbuf sendMsg;
   if (recipient_type == 1)
      sendMsg.type = MESSAGE_TO_ROOM;
   else if (recipient_type == 2)
      sendMsg.type = MESSAGE_TO_CLIENT;

   strcpy(sendMsg.option, recipient_name);
   strcpy(sendMsg.message, message);
   strcpy(sendMsg.client, client_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), -19, 0);

   if (getMsg.type == OK)
      printf("\nOK\n");
   else if (getMsg.type == WRONG_RECIPIENT)
      printf("\nu 've chosen wrong recipient bro  (╯°□°)╯ ┻━┻\n");
   else
      printf("\nSth has fucked up...  ╯︿╰\n");
}

void display_private_messages(int from_server)
{
   msbuf getMsg;

   while (msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), MESSAGE_FROM_CLIENT, IPC_NOWAIT) != -1)
   {
      printf("\ngot something for ya  ( ͡° ͜ʖ ͡°)\n");
      printf("  \"%s\"  from  %s\n", getMsg.message, getMsg.client);
      getMsg.type = 0;
   }
}

void display_room_chat(int from_server)
{
   //need additional info about room
   msbuf getMsg;

   while (msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), MESSAGE_FROM_ROOM, IPC_NOWAIT) != -1)
   {
      printf("\ngot something for ya  ( ͡° ͜ʖ ͡°)\n");
      printf("  \"%s\"  from  %s  in room: %s\n", getMsg.message, getMsg.client, getMsg.option);
      getMsg.type = 0;
   }
}

int main(int argc, char *argv[])
{
   key_t key_to_server = 2137;
   key_t key_from_server = 2138;

   int to_server = msgget(key_to_server, 0777 | IPC_CREAT);
   int from_server = msgget(key_from_server, 0777 | IPC_CREAT);

   //----------------- join server -------------------------

   int joined = 0;
   while (joined == 0)
      joined = join_server(to_server, from_server);

   //wait for ENTER
   getchar();
   char ch = fgetc(stdin);
   while (ch != 0x0A)
      ch = fgetc(stdin);

   system("clear");
   printf("------  %s  ------\n\n", client_name);

   //------------ join to the room ---------------------

   joined = 0;
   int choice;

   while (joined == 0)
   {
      printf("\nDisplay available rooms   1");
      printf("\nJoin existing room        2");
      printf("\nCreate the room           3\n");
      scanf("%d", &choice);

      if (choice == 1)
         printf("Available rooms:...\n");
      else if (choice == 2)
         joined = join_room(to_server, from_server);
      else if (choice == 3)
         create_room(to_server, from_server);

      getchar();
      char ch = fgetc(stdin);
      while (ch != 0x0A)
         ch = fgetc(stdin);

      system("clear");
      printf("------  %s  ------\n\n", client_name);
   }

   //------------------ main loop -----------------------
   while (1)
   {
      printf("\nDisplay available rooms      0");
      printf("\nJoin existing room           1");
      printf("\nCreate the room              2");
      printf("\nWithdraw from room           3");
      printf("\nSend message                 4");
      printf("\nDisplay private messages     5");
      printf("\nDisplay room chat            6");
      printf("\nDisplay clients in server    7");
      printf("\nDisplay clients in rooms     8");
      printf("\nExit                         9\n");
      scanf("%d", &choice);

      if (choice == 0)
         printf("Available rooms:...\n");
      else if (choice == 1)
         join_room(to_server, from_server);
      else if (choice == 2)
         create_room(to_server, from_server);
      else if (choice == 3)
         printf("Witdraw from room\n");
      else if (choice == 4)
         send_message(to_server, from_server);
      else if (choice == 5)
         display_private_messages(from_server);
      else if (choice == 6)
         display_room_chat(from_server);
      else if (choice == 7)
         printf("Displat clients in server\n");
      else if (choice == 8)
         printf("Display clients in rooms\n");
      else if (choice == 9)
      {
         msgctl(from_server, IPC_RMID, NULL);
         msgctl(to_server, IPC_RMID, NULL);
         exit(0);
      }

      getchar();
      char ch = fgetc(stdin);
      while (ch != 0x0A)
         ch = fgetc(stdin);

      system("clear");
      printf("------  %s  ------\n\n", client_name);
   }
}