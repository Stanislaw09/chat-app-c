#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#define JOIN 1
#define JOIN_ROOM 2
#define CREATE_ROOM 3
#define MESSAGE_TO_SERVER 4

#define OK 10
#define USER_EXISTS 11
#define NO_SUCH_SERVER 12
#define ROOM_EXISTS 13
#define MESSAGE_TO_CLIENT 14
#define WRONG_RECIPIENT 15

typedef struct
{
   long type;
   char message[1024];
   char option[256];
   char client[256];
} msbuf;

char server_name[256], room_name[256], client_name[256];

int join_server(int to_server, int from_server)
{
   printf("enter server's name: ");
   scanf("%s", server_name);
   printf("\nenter client name: ");
   scanf("%s", client_name);

   msbuf sendMsg;
   sendMsg.type = JOIN;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.option, "");
   strcpy(sendMsg.client, client_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), (-20), 0);
   printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      printf("\nOK\n");
      return 1;
   }
   else
   {
      printf("\nsuch name already exists\n");
      return 0;
   }
}

int join_room(int to_server, int from_server)
{
   printf("\nenter room name: ");
   scanf("%s", room_name);

   msbuf sendMsg;
   sendMsg.type = JOIN_ROOM;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.client, client_name);
   strcpy(sendMsg.option, room_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), (-20), 0);
   printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      printf("\nOK\n");
      return 1;
   }
   else
   {
      printf("\nno such room\n");
      return 0;
   }
}

int create_room(int to_server, int from_server)
{
   printf("\nenter room name to create: ");
   scanf("%s", room_name);

   msbuf sendMsg;
   sendMsg.type = CREATE_ROOM;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.client, client_name);
   strcpy(sendMsg.option, room_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), (-20), 0);
   printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      printf("\nOK\n");
      return 1;
   }
   else
   {
      printf("\nsuch room already exists\n");
      return 0;
   }
}

void send_message(int to_server)
{
   char recipient[256], message[1024];
   printf("enter message:\n");
   getchar();
   scanf("%1023[^\n]", message);

   printf("enter recipient (room or client):\n");
   scanf("%s", recipient);

   msbuf sendMsg;
   sendMsg.type = MESSAGE_TO_SERVER;
   strcpy(sendMsg.option, recipient);
   strcpy(sendMsg.message, message);
   strcpy(sendMsg.client, client_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);
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

   getchar();
   char ch = fgetc(stdin);
   while (ch != 0x0A) //wait for enter
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
      {
         //display list of available rooms
         printf("Available rooms:...\n");
      }
      else if (choice == 2)
         joined = join_room(to_server, from_server);
      else if (choice == 3)
         create_room(to_server, from_server);

      getchar();
      char ch = fgetc(stdin);
      while (ch != 0x0A) //wait for enter
         ch = fgetc(stdin);

      system("clear");
      printf("------  %s  ------\n\n", client_name);
   }

   //------------------ main loop -----------------------

   while (1)
   {
      printf("\nDisplay available rooms   1");
      printf("\nJoin existing room        2");
      printf("\nCreate the room           3");
      printf("\nSend message              4");
      printf("\nClear queues              5\n");
      scanf("%d", &choice);

      if (choice == 1)
      {
         //display list of available rooms
         printf("Available rooms:...\n");
      }
      else if (choice == 2)
         join_room(to_server, from_server);
      else if (choice == 3)
         create_room(to_server, from_server);
      else if (choice == 4)
         send_message(to_server);
      else if (choice == 5)
      {
         msgctl(from_server, IPC_RMID, NULL);
         msgctl(to_server, IPC_RMID, NULL);
      }

      msbuf getMsg;
      sleep(0.1);
      msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), 14, IPC_NOWAIT);
      msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), 15, IPC_NOWAIT);
      if (getMsg.type != 0)
         printf("Got this: %ld , %s\n\n", getMsg.type, getMsg.client);

      if (strcmp(client_name, getMsg.client) == 0 && getMsg.type == MESSAGE_TO_CLIENT)
      {
         printf("\ngot something for ya  ( ͡° ͜ʖ ͡°)\n");
         printf("  %s", getMsg.message);
         getMsg.type = 0;
         strcpy(getMsg.message, "");
         sleep(0.1);
      }
      else if (strcmp(client_name, getMsg.client) == 0 && getMsg.type == WRONG_RECIPIENT)
      {
         printf("\nu have chosen wrong recipient  (╯°□°)╯ ┻━┻\n");
         getMsg.type = 0;
         strcpy(getMsg.message, "");
         sleep(0.1);
      }

      getchar();
      char ch = fgetc(stdin);
      while (ch != 0x0A) //wait for enter
         ch = fgetc(stdin);

      system("clear");
      printf("------  %s  ------\n\n", client_name);
   }

   // if (fork() == 0)
   // {
   //    while (1)
   //       if (read(0, msg.str, 1024) > 0)
   //       {
   //          msg.msgtype = M_SRV;
   //          printf("SND | %s\n", msg.str);
   //          msgsnd(from_server, &msg, sizeof(msbuf) - sizeof(long), 0);
   //       }
   // }
   // else
   // {
   //    close(0);
   //    while (1)
   //       while (msgrcv(from_server, &msg, sizeof(msbuf) - sizeof(long), M_CLI, 0) != -1)
   //          printf("RCV | %s\n", msg.str);
   // }
}