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

char clients[5][256], rooms_names[10][256], clients_in_rooms[10][5][256];
// assumes that clients_in_rooms are the rooms as in rooms_names with corresponding indexes
// it doesnt assume possibility of removing room / didnt mentions in requirements

void join_client(int to_client, msbuf getMsg)
{
   for (int index = 0; index < 5; index++)
   {
      if (strcmp(getMsg.client, clients[index]) == 0)
      {
         //send error - such name already exists
         printf("such user already exists\n");
         msbuf sendMsg;
         strcpy(sendMsg.client, getMsg.client);
         sendMsg.type = USER_EXISTS;

         msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         break;
      }
      else if (index == 4)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(clients[j], "") == 0)
            {
               //add client and send ok
               strcpy(clients[j], getMsg.client);
               printf("add client\n");

               msbuf sendMsg;
               strcpy(sendMsg.client, getMsg.client);
               sendMsg.type = OK;
               msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               break;
            }
      }
   }
}

void join_room(int to_client, msbuf getMsg)
{
   int user_added = 0;

   for (int index = 0; index < 10; index++)
   {
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(clients_in_rooms[index][j], getMsg.client) == 0)
               user_added = 1;

         if (user_added)
         {
            //user already in room
            printf("user already in room\n");
            msbuf sendMsg;
            strcpy(sendMsg.client, getMsg.client);
            sendMsg.type = OK;
            msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
            break;
         }
         else
         {
            //add user to room
            for (int j = 0; j < 5; j++)
               if (strcmp(clients_in_rooms[index][j], "") == 0)
               {
                  strcpy(clients_in_rooms[index][j], getMsg.client);
                  printf("add client ot room\n");

                  msbuf sendMsg;
                  strcpy(sendMsg.client, getMsg.client);
                  sendMsg.type = OK;
                  msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
                  user_added = 1;
                  break;
               }
         }
      }
      else if (index == 9 && !user_added)
      {
         //no such room
         printf("no such room\n");
         msbuf sendMsg;
         strcpy(sendMsg.client, getMsg.client);
         sendMsg.type = NO_SUCH_ROOM;
         msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         break;
      }
   }
}

void create_room(int to_client, msbuf getMsg)
{
   for (int i = 0; i < 10; i++)
   {
      //check if the room exists
      if (strcmp(rooms_names[i], getMsg.option) == 0)
      {
         printf("room exists\n");
         msbuf sendMsg;
         sendMsg.type = ROOM_EXISTS;
         strcpy(sendMsg.client, getMsg.client);
         msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         break;
      }
      else if (i == 9)
         for (int j = 0; j < 10; j++)
         {
            //create new room
            if (strcmp(rooms_names[j], "") == 0)
            {
               strcpy(rooms_names[j], getMsg.option);
               printf("create room\n");

               msbuf sendMsg;
               sendMsg.type = OK;
               strcpy(sendMsg.client, getMsg.client);
               msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               break;
            }
         }
   }
}

void dispatch_private_message(int to_client, msbuf getMsg)
{
   int send_flag = 0;

   for (int i = 0; i < 5; i++)
      if (strcmp(getMsg.option, clients[i]) == 0)
      {
         //need to check if they are in the same room
         printf("send message to client\n");
         msbuf sendMsg;
         sendMsg.type = MESSAGE_FROM_CLIENT;
         strcpy(sendMsg.message, getMsg.message);
         strcpy(sendMsg.client, getMsg.client);
         msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         send_flag = 1;
         break;
      }

   //send feedback
   if (send_flag)
   {
      msbuf sendMsg;
      sendMsg.type = OK;
      msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
   else
   {
      msbuf sendMsg;
      sendMsg.type = WRONG_RECIPIENT;
      msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
}

void dispatch_room_message(int to_client, msbuf getMsg)
{
   int send_flag = 0;

   for (int index = 0; index < 10; index++)
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(clients_in_rooms[index][j], ""))
            {
               printf("send message to room\n");
               msbuf sendMsg;
               sendMsg.type = MESSAGE_FROM_ROOM;
               strcpy(sendMsg.message, getMsg.message);
               strcpy(sendMsg.client, getMsg.client);
               strcpy(sendMsg.option, rooms_names[index]);
               msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               send_flag = 1;
            }

         break;
      }

   //send feedback
   if (send_flag)
   {
      msbuf sendMsg;
      sendMsg.type = OK;
      msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
   else
   {
      msbuf sendMsg;
      sendMsg.type = WRONG_RECIPIENT; //or room is empty
      msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
}

int main(int argc, char *argv[])
{
   key_t key_from_client = 2137;
   key_t key_to_client = 2138;

   int from_client = msgget(key_from_client, 0777 | IPC_CREAT);
   int to_client = msgget(key_to_client, 0777 | IPC_CREAT);

   while (1)
   {
      msbuf getMsg;

      while (msgrcv(from_client, &getMsg, sizeof(msbuf) - sizeof(long), -9, 0) != -1)
      {
         printf("Received: %ld , %s , %s , %s\n\n", getMsg.type, getMsg.message, getMsg.option, getMsg.client);

         if (getMsg.type == JOIN)
         {
            join_client(to_client, getMsg);

            printf("Users:\n");
            for (int i = 0; i < 10; i++)
               if (strcmp(clients[i], ""))
                  printf("  %s\n", clients[i]);
         }
         else if (getMsg.type == JOIN_ROOM)
         {
            join_room(to_client, getMsg);

            printf("Clients in rooms:\n");
            for (int i = 0; i < 10; i++)
               if (strcmp(rooms_names[i], ""))
               {
                  printf("  %s\n", rooms_names[i]);

                  for (int j = 0; j < 5; j++)
                     if (strcmp(clients_in_rooms[i][j], ""))
                        printf("    %s\n", clients_in_rooms[i][j]);
               }
         }
         else if (getMsg.type == CREATE_ROOM)
         {
            create_room(to_client, getMsg);

            printf("Rooms:\n");
            for (int i = 0; i < 10; i++)
               if (strcmp(rooms_names[i], ""))
                  printf("  %s\n", rooms_names[i]);
         }
         else if (getMsg.type == MESSAGE_TO_ROOM)
            dispatch_room_message(to_client, getMsg);
         else if (getMsg.type == MESSAGE_TO_CLIENT)
            dispatch_private_message(to_client, getMsg);

         printf("\n\n--------------------------------------------\n\n");
      }
   }
}