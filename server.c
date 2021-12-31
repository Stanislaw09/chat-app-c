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
#define MESSAGE 4

#define OK 10
#define USER_EXISTS 11
#define NO_SUCH_ROOM 12
#define ROOM_EXISTS 13

typedef struct
{
   long type;
   char message[1024];
   char option[256];
   char client[256];
} msbuf;

char clients[5][256], rooms_names[10][256], clients_in_rooms[10][5][256];

void join_client(int to_client, msbuf getMsg)
{
   for (int index = 0; index < 5; index++)
   {
      if (strcmp(getMsg.client, clients[index]) == 0)
      {
         //send error - such name already exists
         msbuf sendMsg;
         strcpy(sendMsg.client, getMsg.client);
         sendMsg.type = USER_EXISTS;

         msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         break;
      }
      else if (index == 4)
      {
         for (int j = 0; j < 5; j++)
         {
            if (strcmp(clients[j], "") == 0)
            {
               //add client and send ok
               strcpy(clients[j], getMsg.client);

               msbuf sendMsg;
               strcpy(sendMsg.client, getMsg.client);
               sendMsg.type = OK;
               msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               break;
            }
         }
      }
   }
}

void join_room(int to_client, msbuf getMsg)
{
   for (int index = 0; index < 10; index++)
   {
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         int user_found = 0;

         for (int j = 0; j < 5; j++)
            if (strcmp(clients_in_rooms[index][j], getMsg.client) == 0)
               user_found = 1;

         if (user_found)
         {
            //user already in room
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

                  msbuf sendMsg;
                  strcpy(sendMsg.client, getMsg.client);
                  sendMsg.type = OK;
                  msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
                  break;
               }
         }
      }
      else if (index == 9)
      {
         //no such server
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
         msbuf sendMsg;
         sendMsg.type = ROOM_EXISTS;
         strcpy(sendMsg.client, getMsg.client);
         msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         break;
      }
      else if (i == 9)
      {
         for (int j = 0; j < 10; j++)
         {
            //create new room
            if (strcmp(rooms_names[j], "") == 0)
            {
               strcpy(rooms_names[j], getMsg.option);

               msbuf sendMsg;
               sendMsg.type = OK;
               strcpy(sendMsg.client, getMsg.client);
               msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               break;
            }
         }
      }
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

         if (getMsg.type == JOIN_ROOM)
            join_room(to_client, getMsg);

         if (getMsg.type == CREATE_ROOM)
         {
            create_room(to_client, getMsg);

            printf("Rooms:\n");
            for (int i = 0; i < 10; i++)
               if (strcmp(rooms_names[i], ""))
                  printf("  %s\n", rooms_names[i]);
         }

         printf("\n\n--------------------------------------------\n\n");
      }
   }

   // if (fork() == 0)
   // {
   //    while (1)
   //       if (read(0, msg.str, 1024) > 0)
   //       {
   //          msg.msgtype = M_SRV;
   //          printf("SND | %s\n", msg.str);
   //          msgsnd(to_client, &msg, sizeof(msbuf) - sizeof(long)- sizeof(long), - sizeof(long) 0);
   //       }
   // }
   // else
   // {
   //    close(0);
   //    while (1)
   //       while (msgrcv(id, &msg, sizeof(msbuf) - sizeof(long)- sizeof(long), - sizeof(long) -3, 0) != -1)
   //          printf("RCV | %s\n", msg.str);
   // }
}