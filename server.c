#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>
#include <signal.h>

//messages from client
#define JOIN 1
#define JOIN_ROOM 2
#define CREATE_ROOM 3
#define MESSAGE_TO_ROOM 4
#define MESSAGE_TO_CLIENT 5

#define DISPLAY_CLIENTS_IN_ROOMS 7
#define DISPLAY_ROOMS 6
//messages from server
#define OK 10
#define USER_EXISTS 11
#define NO_SUCH_ROOM 12
#define ROOM_EXISTS 13
#define WRONG_RECIPIENT 14
#define CLIENTS_IN_ROOMS 16
//types of messages
#define MESSAGE_FROM_CLIENT 20
#define MESSAGE_FROM_ROOM 21

#define PROJ_ID 2137

typedef struct
{
   long type;
   char message[1024];
   char option[256];
   char client[256];
} msbuf;

char server_name[256], client_names[5][256], rooms_names[10][256], rooms_client_names[10][5][256];
int server_queue, client_queues[5], rooms_client_queues[10][5];
// assumes that rooms_client_names are the rooms as in rooms_names with corresponding indexes
// it doesnt assume possibility of removing room / didnt mentions in requirements

void clean_ftok_temp_shit(int n)
{
   system("rm -rf ./ftok_temp_shit");
   exit(1);
}

key_t get_id_from_string(char string[])
{
   // all the fuckery below is needed to get ftok working for this use case ¯\_(ツ)_/¯
   signal(SIGINT, clean_ftok_temp_shit);
   char cmd[1024] = "mkdir -p ";
   char path[1024] = "./ftok_temp_shit/";
   strcat(cmd, path);
   system(cmd);
   strcat(path, string);
   strcpy(cmd, "touch ");
   strcat(cmd, path);
   system(cmd);
   return ftok(path, PROJ_ID);
}

int check_room(char client1[], char client2[])
{
   for (int room = 0; room < 10; room++)
      for (int i = 0; i < 5; i++)
         if (strcmp(rooms_client_names[room][i], client1) == 0)
            for (int j = 0; j < 5; j++)
               if (strcmp(rooms_client_names[room][j], client2) == 0)
                  return 1;
   return 0;
}

int get_client_queue(char client[])
{
   for (int i = 0; i < 5; i++)
      if (strcmp(client_names[i], client) == 0)
         return client_queues[i];
}

void join_client(msbuf getMsg)
{
   for (int index = 0; index < 5; index++)
   {
      if (strcmp(getMsg.client, client_names[index]) == 0)
      {
         // name duplicate checking is disabled for now

         // //send error - such name already exists
         // printf("such user already exists\n");
         // msbuf sendMsg;
         // strcpy(sendMsg.client, getMsg.client);
         // sendMsg.type = USER_EXISTS;

         // msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         break;
      }
      else if (index == 4)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(client_names[j], "") == 0)
            {
               //add client and send ok
               strcpy(client_names[j], getMsg.client);
               client_queues[j] = msgget(get_id_from_string(getMsg.client), 0777);
               printf("add client\n");

               msbuf sendMsg;
               strcpy(sendMsg.client, getMsg.client);
               sendMsg.type = OK;
               msgsnd(client_queues[j], &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               break;
            }
      }
   }
}

void join_room(msbuf getMsg)
{
   int user_added = 0;

   for (int index = 0; index < 10; index++)
   {
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(rooms_client_names[index][j], getMsg.client) == 0)
               user_added = 1;

         if (user_added)
         {
            //user already in room
            printf("user already in room\n");
            msbuf sendMsg;
            strcpy(sendMsg.client, getMsg.client);
            sendMsg.type = OK;
            msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
            break;
         }
         else
         {
            //add user to room
            for (int j = 0; j < 5; j++)
               if (strcmp(rooms_client_names[index][j], "") == 0)
               {
                  strcpy(rooms_client_names[index][j], getMsg.client);
                  rooms_client_queues[index][j] = msgget(get_id_from_string(getMsg.client), 0777);
                  printf("add client >%s< to room >%s<\n", getMsg.client, rooms_names[index]);

                  msbuf sendMsg;
                  strcpy(sendMsg.client, getMsg.client);
                  sendMsg.type = OK;
                  msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
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
         msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         break;
      }
   }
}

void create_room(msbuf getMsg)
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
         msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
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
               msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               break;
            }
         }
   }
}

void dispatch_private_message(msbuf getMsg)
{
   int send_flag = 0;

   for (int i = 0; i < 5; i++)
      if (strcmp(getMsg.option, client_names[i]) == 0)
      {
         if (check_room(getMsg.client, getMsg.option) == 0)
         {
            // not in the same room
            printf("Error / sender >%s< and recipient >%s< are not in the same room!\n", getMsg.client, getMsg.option);
            send_flag = 0;
            break;
         }
         printf("send message to client\n");
         msbuf sendMsg;
         sendMsg.type = MESSAGE_FROM_CLIENT;
         strcpy(sendMsg.message, getMsg.message);
         strcpy(sendMsg.client, getMsg.client);
         msgsnd(client_queues[i], &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         send_flag = 1;
         break;
      }

   //send feedback
   if (send_flag)
   {
      msbuf sendMsg;
      sendMsg.type = OK;
      msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
   else
   {
      msbuf sendMsg;
      sendMsg.type = WRONG_RECIPIENT;
      msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
}

void dispatch_room_message(msbuf getMsg)
{
   int send_flag = 0;

   for (int index = 0; index < 10; index++)
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(rooms_client_names[index][j], ""))
            {
               printf("send message to room\n");
               msbuf sendMsg;
               sendMsg.type = MESSAGE_FROM_ROOM;
               strcpy(sendMsg.message, getMsg.message);
               strcpy(sendMsg.client, getMsg.client);
               strcpy(sendMsg.option, rooms_names[index]);
               msgsnd(rooms_client_queues[index][j], &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               send_flag = 1;
            }

         break;
      }

   //send feedback
   if (send_flag)
   {
      msbuf sendMsg;
      sendMsg.type = OK;
      msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
   else
   {
      msbuf sendMsg;
      sendMsg.type = WRONG_RECIPIENT;
      msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   }
}

void get_clients_in_rooms(msbuf getMsg){
   char out[1024];
   for(int i = 0; i < 10; i++){
      if(strcmp(rooms_names[i], "") != 0){
         strcat(out, "> ");
         strcat(out, rooms_names[i]);
         strcat(out, "\n");
         for(int j = 0; j<5;j++){
            if(strcmp(rooms_client_names[i][j], "") != 0)
               strcat(out, "  - ");
               strcat(out, rooms_client_names[i][j]);
               strcat(out, "\n");
         }
      }
   }
   msbuf sendMsg;
   sendMsg.type = CLIENTS_IN_ROOMS;
   strcpy(sendMsg.message, out);
   msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   
}
void get_available_rooms(msbuf getMsg)
{
   char rooms[256];

   for (int i = 0; i < 10; i++)
   {
      if (strcmp(rooms_names[i], ""))
      {
         strcat(rooms, "\n");
         strcat(rooms, rooms_names[i]);
      }
   }

   msbuf sendMsg;
   sendMsg.type = OK;
   strcpy(sendMsg.client, getMsg.client);
   strcpy(sendMsg.option, rooms);
   msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);

   printf("rooms: %s", rooms);
}

void handle_message(msbuf getMsg)
{
   printf("Received: %ld , %s , %s , %s\n\n", getMsg.type, getMsg.message, getMsg.option, getMsg.client);
   switch (getMsg.type)
   {
   case JOIN:
      join_client(getMsg);
      printf("Users:\n");
      for (int i = 0; i < 10; i++)
         if (strcmp(client_names[i], ""))
            printf("  %s\n", client_names[i]);
      break;
   case JOIN_ROOM:
      join_room(getMsg);
      printf("client_names in rooms:\n");
      for (int i = 0; i < 10; i++)
         if (strcmp(rooms_names[i], ""))
         {
            printf("  %s\n", rooms_names[i]);

               for (int j = 0; j < 5; j++)
                  if (strcmp(rooms_client_names[i][j], ""))
                     printf("    %s\n", rooms_client_names[i][j]);
            }
         break;
      case CREATE_ROOM:
         create_room(getMsg);
         printf("Rooms:\n");
         for (int i = 0; i < 10; i++)
            if (strcmp(rooms_names[i], ""))
               printf("  %s\n", rooms_names[i]);
         break;
      case MESSAGE_TO_ROOM:
         dispatch_room_message(getMsg);
         break;
      case MESSAGE_TO_CLIENT:
         dispatch_private_message(getMsg);
         break;
      case DISPLAY_CLIENTS_IN_ROOMS:
         get_clients_in_rooms(getMsg);
         break;
      case DISPLAY_ROOMS:
         get_available_rooms(getMsg);
         break;

   }
   printf("\n\n--------------------------------------------\n\n");
}

int main(int argc, char *argv[])
{
   printf("enter server's name: ");
   scanf("%s", server_name);
   server_queue = msgget(get_id_from_string(server_name), 0777 | IPC_CREAT);

   msbuf getMsg;
   while (1)
      while (msgrcv(server_queue, &getMsg, sizeof(msbuf) - sizeof(long), -9, 0) != -1)
         handle_message(getMsg);
}