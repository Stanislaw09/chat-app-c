#define IS_SERVER
#include "common.h"

char server_name[256], client_names[5][256], rooms_names[10][256], rooms_client_names[10][5][256];
int server_queue, client_queues[5], rooms_client_queues[10][5], recent_room_messages_index[10];
msbuf recent_room_messages[10][10];
// assumes that rooms_client_names are the rooms as in rooms_names with corresponding indexes
// it doesnt assume possibility of removing room / didnt mentions in requirements

void clean(int n)
{
    msgctl(server_queue, IPC_RMID, NULL);
   char cmd[1024] = "rm -rf ";
   strcat(cmd, TEMP_DIR);
   system(cmd);
    exit(1);
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
   for (int index = 0; index < 10; index++)
   {
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(rooms_client_names[index][j], getMsg.client) == 0){
               //user already in room
               printf("user already in room\n");
               msbuf sendMsg;
               strcpy(sendMsg.client, getMsg.client);
               sendMsg.type = OK;
               msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               return;
            }

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
               return;
            }
         
      }
      else if (index == 9)
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

void exit_room(msbuf getMsg)
{
   for (int index = 0; index < 10; index++)
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(rooms_client_names[index][j], getMsg.client) == 0){
               printf("removing %s from %s\n", getMsg.client, rooms_names[index]);
               strcpy(rooms_client_names[index][j], "");
               rooms_client_queues[index][j] = 0;
               msbuf sendMsg;
               strcpy(sendMsg.client, getMsg.client);
               sendMsg.type = OK;
               msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               return;
            }

         //user not in room
         printf("user not in room");

         msbuf sendMsg;
         strcpy(sendMsg.client, getMsg.client);
         sendMsg.type = USER_NOT_IN_ROOM;
         msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         return;
      }
      else if (index == 9)
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
         if(recent_room_messages_index[index] == 10)
            recent_room_messages_index[index] = 0;
         recent_room_messages[index][recent_room_messages_index[index]++] = getMsg;
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
   char out[1024] = "";
   for(int i = 0; i < 10; i++)
      if(strcmp(rooms_names[i], "") != 0){
         strcat(out, "* ");
         strcat(out, rooms_names[i]);
         strcat(out, "\n");
         for(int j = 0; j<5;j++)
            if(strcmp(rooms_client_names[i][j], "") != 0){
               strcat(out, "  - ");
               strcat(out, rooms_client_names[i][j]);
               strcat(out, "\n");
            }
      }
   msbuf sendMsg;
   sendMsg.type = CLIENTS_IN_ROOMS;
   strcpy(sendMsg.message, out);
   msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   
}

void get_available_rooms(msbuf getMsg)
{
   char rooms[256] = "";

   for (int i = 0; i < 10; i++)
      if (strcmp(rooms_names[i], ""))
      {
         strcat(rooms, "- ");
         strcat(rooms, rooms_names[i]);
         strcat(rooms, "\n");
      }

   msbuf sendMsg;
   sendMsg.type = ROOMS;
   strcpy(sendMsg.client, getMsg.client);
   strcpy(sendMsg.message, rooms);
   msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
}

void display_recent_messages(msbuf getMsg){
   for (int index = 0; index < 10; index++)
      if (strcmp(getMsg.option, rooms_names[index]) == 0)
      {
         for (int j = 0; j < 5; j++)
            if (strcmp(rooms_client_names[index][j], getMsg.client) == 0){
               printf("sending recent messages from [%s] to %s\n", rooms_names[index], getMsg.client);

               msbuf sendMsg;
               strcpy(sendMsg.client, getMsg.client);
               sendMsg.type = OK;
               msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);

               for(int i = recent_room_messages_index[index]; i < 10; i++)
                  if(recent_room_messages[index][i].type == MESSAGE_FROM_ROOM)
                     msgsnd(get_client_queue(getMsg.client), &recent_room_messages[index][i], sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               
               for(int i = 0; i < recent_room_messages_index[index]; i++)
                  if(recent_room_messages[index][i].type == MESSAGE_FROM_ROOM)
                     msgsnd(get_client_queue(getMsg.client), &recent_room_messages[index][i], sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
               
               return;
            }

         //user not in room
         printf("user not in room");

         msbuf sendMsg;
         strcpy(sendMsg.client, getMsg.client);
         sendMsg.type = USER_NOT_IN_ROOM;
         msgsnd(get_client_queue(getMsg.client), &sendMsg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
         return;
      }
      else if (index == 9)
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
   case EXIT_ROOM:
      exit_room(getMsg);
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
   case DISPLAY_RECENT_MESSAGES:
      display_recent_messages(getMsg);
      break;
   }
   printf("\n\n--------------------------------------------\n\n");
}

int main(int argc, char *argv[])
{
   for(int i = 0; i < 10; i++){
      recent_room_messages_index[i] = 0;
   }

   printf("enter server's name: ");
   scanf("%s", server_name);

   signal(SIGINT, clean);
   ftok_init(server_name);

   server_queue = msgget(get_id_from_string(server_name), 0777 | IPC_CREAT);

   msbuf getMsg;
   while (1)
      while (msgrcv(server_queue, &getMsg, sizeof(msbuf) - sizeof(long), -9, 0) != -1)
         handle_message(getMsg);
}