#include "./common.h"

char server_name[256], client_name[256];
int client_queue, server_queue;

void clean(int n)
{
   msgctl(client_queue, IPC_RMID, NULL);
   exit(1);
}

int join_server()
{
   msbuf sendMsg;
   sendMsg.type = JOIN;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.option, "");
   strcpy(sendMsg.client, client_name);
   msgsnd(server_queue, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), -19, 0);
   // printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      printf("\nOK\n");
      return 1;
   }
   else if (getMsg.type == USER_EXISTS)
   {
      printf("\nsuch name already exists\n");
      exit(69);
   }
   else
      OOF;
}

int room_command(int command, char *room_name)
{
   msbuf sendMsg;
   sendMsg.type = command;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.client, client_name);
   strcpy(sendMsg.option, room_name);
   msgsnd(server_queue, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), -19, 0);
   // printf("\nRCV: %ld\n", getMsg.type);

   if (getMsg.type == OK)
   {
      if (sendMsg.type == CREATE_ROOM)
         room_command(JOIN_ROOM, room_name);
      printf("\nOK\n");
      return 1;
   }
   else if (getMsg.type == NO_SUCH_ROOM)
   {
      printf("\nno such room\n");
      return 0;
   }
   else if (getMsg.type == ROOM_EXISTS)
   {
      printf("\nsuch room already exists\n");
      return 0;
   }
   else
      OOF;
}

int gui_room_command(int command)
{
   char room_name[256];
   printf("\nroom name: ");
   fflush(stdout);
   scanf("%s", room_name);

   return room_command(command, room_name);
}

void send_message(char *message)
{
   int recipient_type;
   char recipient_name[256];

   // printf("enter message: ");
   // getchar();
   // scanf("%1023[^\n]", message);
   system("stty raw");
   printf("\nROOM (1) PRIVATE (2) ");
   fflush(stdout);
   recipient_type = fgetc(stdin) - '0';
   if (recipient_type == 2)
      printf(" | recipient: ");
   else
      printf(" | room name: ");
   fflush(stdout);
   system("stty cooked");
   scanf("%s", recipient_name);

   msbuf sendMsg;
   if (recipient_type == 1)
      sendMsg.type = MESSAGE_TO_ROOM;
   else if (recipient_type == 2)
      sendMsg.type = MESSAGE_TO_CLIENT;

   strcpy(sendMsg.option, recipient_name);
   strcpy(sendMsg.message, message);
   strcpy(sendMsg.client, client_name);
   msgsnd(server_queue, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), -19, 0);

   if (getMsg.type == OK)
      printf("OK\n");
   else if (getMsg.type == USER_NOT_IN_ROOM)
      printf("User not in room!\n");
   else if (getMsg.type == WRONG_RECIPIENT)
      printf("ERR wrong recipient!\n");
   else
      OOF;
}

void display_private_messages()
{
   msbuf getMsg;

   while (msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), MESSAGE_FROM_CLIENT, IPC_NOWAIT) != -1)
      printf("\n%s: %s", getMsg.client, getMsg.message);
}

void display_room_chat()
{
   //need additional info about room
   msbuf getMsg;

   while (msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), MESSAGE_FROM_ROOM, IPC_NOWAIT) != -1)
      if (strcmp(getMsg.client, client_name) != 0)
         printf("\n[%s] %s: %s", getMsg.option, getMsg.client, getMsg.message);
}

void display_cached_room_chat()
{
   msbuf getMsg;
   while (msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), CACHED_MESSAGE_FROM_ROOM, IPC_NOWAIT) != -1)
      printf("\nCACHED [%s] %s: %s", getMsg.option, getMsg.client, getMsg.message);
}

void display_clients_in_rooms()
{
   msbuf sendMsg;
   sendMsg.type = DISPLAY_CLIENTS_IN_ROOMS;
   strcpy(sendMsg.client, client_name);
   msgsnd(server_queue, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), CLIENTS_IN_ROOMS, 0);

   if (getMsg.type != CLIENTS_IN_ROOMS)
      OOF;

   printf("\nDisplaying clients in rooms: \n");
   printf("%s\n", getMsg.message);
}

void display_available_rooms()
{
   msbuf sendMsg;
   sendMsg.type = DISPLAY_ROOMS;
   strcpy(sendMsg.client, client_name);
   msgsnd(server_queue, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(client_queue, &getMsg, sizeof(msbuf) - sizeof(long), ROOMS, 0);

   if (getMsg.type != ROOMS)
      OOF;

   printf("\nDisplaying available rooms: \n");
   printf("%s\n", getMsg.message);
}

void menu()
{
   int choice;

   printf("\n---------- MENU --------------");
   printf("\nDisplay available rooms      0");
   printf("\nJoin existing room           1");
   printf("\nCreate and join the room     2");
   printf("\nExit room                    3");
   // printf("\nSend message                 4");
   // printf("\nDisplay private messages     5");
   // printf("\nDisplay room chat            6");
   printf("\nDisplay rooms and clients    7");
   printf("\nDisplay recent room messages 8");
   printf("\nExit                         9");
   printf("\n------------------------------\n\n");
   system("stty raw");
   choice = fgetc(stdin) - '0';
   system("stty cooked");
   printf("\n");
   if (choice == 0)
      display_available_rooms();
   else if (choice == 1)
      gui_room_command(JOIN_ROOM);
   else if (choice == 2)
      gui_room_command(CREATE_ROOM);
   else if (choice == 3)
      gui_room_command(EXIT_ROOM);
   // else if (choice == 4)
   //    send_message();
   // else if (choice == 5)
   //    display_private_messages();
   // else if (choice == 6)
   //    display_room_chat();
   else if (choice == 7)
      display_clients_in_rooms();
   else if (choice == 8)
      gui_room_command(DISPLAY_RECENT_MESSAGES);
   else if (choice == 9)
      clean(0);

   // getchar();
   // char ch = fgetc(stdin);
   // while (ch != 0x0A)
   //    ch = fgetc(stdin);
}
int main(int argc, char *argv[])
{
   setbuf(stdout, NULL);
   printf("server name: ");
   fflush(stdout);
   scanf("%s", server_name);
   printf("\nclient name: ");
   fflush(stdout);
   scanf("%s", client_name);

   signal(SIGINT, clean);
   ftok_init(client_name);

   server_queue = msgget(get_id_from_string(server_name), 0777);
   client_queue = msgget(get_id_from_string(client_name), 0777 | IPC_CREAT);

   join_server();

   // wait for ENTER
   char ch = fgetc(stdin);
   while (ch != 0x0A)
      ch = fgetc(stdin);

   int flag = 0;
   int choice = 0;

   while (!flag)
   {
      system("clear");
      printf("------  %s  ------\n\n", client_name);
      printf("\nJoin existing room          1");
      printf("\nCreate and join a new room  2\n");
      system("stty raw");
      fflush(stdout);
      choice = fgetc(stdin) - '0';
      fflush(stdout);
      system("stty cooked");
      if (choice == 1)
      {
         display_available_rooms();
         flag = gui_room_command(JOIN_ROOM);
      }
      else if (choice == 2)
         flag = gui_room_command(CREATE_ROOM);

      if(!flag){
         printf("\n\n[press any key to continue]\n\n");
         getchar();
      }
      getchar();
   }

   // char ch = fgetc(stdin);
   // while (ch != 0x0A)
   //    ch = fgetc(stdin);

   system("clear");
   printf("------  %s  ------\n\n", client_name);
   printf("type your message or /menu to display menu\n");
   //------------------ main loop -----------------------
   if (fork() == 0)
      while (1)
      {
         display_room_chat();
         display_private_messages();
         display_cached_room_chat();
         sleep(1);
      }
   else
      while (1)
      {
         printf("\nmessage / menu > ");
         fflush(stdout);
         char *message;
         size_t size = 1024;
         message = (char *)malloc(size * sizeof(char));
         getline(&message, &size, stdin);
         if (strstr(message, "/menu") != NULL)
         {
            menu();
            getchar();
         }
         else
         {
            send_message(message);

            getchar();
         }
      }
}