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
#define NO_SUCH_SERVER 12

typedef struct
{
   long type;
   char message[1024];
   char option[256];
   char client[256];
} msbuf;

char clients[5][256], servers_names[10][256], clients_in_servers[10][5][256];

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
         printf("RCV: %ld , %s , %s , %s\n", getMsg.type, getMsg.message, getMsg.option, getMsg.client);

         if (getMsg.type == JOIN)
         {
            printf("join new client\n");

            for (int index = 0; index < 5; index++)
            {
               if (strcmp(getMsg.client, clients[index]) == 0)
               {
                  //send error - such name already exists
                  printf("user exists\n");
                  msbuf sendMsg;
                  strcpy(sendMsg.client, getMsg.client);
                  sendMsg.type = USER_EXISTS;

                  msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
                  break;
               }
               if (strcmp(clients[index], "") == 0)
               { //CAN BREAK!!!!!!!!!!!!!!!!!!!!
                  //add client and send ok
                  printf("add user\n");
                  strcpy(clients[index], getMsg.client);

                  msbuf sendMsg;
                  strcpy(sendMsg.client, getMsg.client);
                  sendMsg.type = OK;
                  int x = msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
                  printf("status %d", x);
                  break;
               }
            }
         }

         if (getMsg.type == JOIN_ROOM)
         {
            for (int index = 0; index < 10; index++)
            {
               if (strcmp(getMsg.option, servers_names[index]) == 0)
               {
                  printf("server found\n");
                  int user_found = 0;

                  for (int j = 0; j < 5; j++)
                     if (strcmp(clients_in_servers[index][j], getMsg.client) == 0)
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
                        if (strcmp(clients_in_servers[index][j], "") == 0)
                        {
                           strcpy(clients_in_servers[index][j], getMsg.client);

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
                  sendMsg.type = NO_SUCH_SERVER;
                  msgsnd(to_client, &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
                  break;
               }
            }
         }

         printf("finish receiving\n");
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