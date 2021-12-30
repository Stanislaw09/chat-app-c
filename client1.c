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

msbuf message1, message2;
char server_name[256], room_name[256], client_name[256], servers[10][256];

int main(int argc, char *argv[])
{
   printf("enter server's name: ");
   scanf("%s", server_name);
   printf("\nenter client name: ");
   scanf("%s", client_name);

   key_t key_to_server = 2137;
   key_t key_from_server = 2138;

   int to_server = msgget(key_to_server, 0777 | IPC_CREAT);
   int from_server = msgget(key_from_server, 0777 | IPC_CREAT);

   msbuf sendMsg;
   sendMsg.type = JOIN;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.option, "");
   strcpy(sendMsg.client, client_name);

   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg;
   msgrcv(from_server, &getMsg, sizeof(msbuf) - sizeof(long), (-20), 0);
   printf("\nRCV: %ld\n", getMsg.type);
   sleep(2);

   //------------ join to the room ------------------

   printf("\nenter initial room name: ");
   scanf("%s", room_name);
   sendMsg.type = JOIN_ROOM;
   strcpy(sendMsg.message, "");
   strcpy(sendMsg.option, room_name);
   msgsnd(to_server, &sendMsg, sizeof(msbuf) - sizeof(long), 0);

   msbuf getMsg1;
   msgrcv(from_server, &getMsg1, sizeof(msbuf) - sizeof(long), (-20), 0);
   printf("\nRCV: %ld\n", getMsg1.type);

   msgctl(from_server, IPC_RMID, NULL);
   msgctl(to_server, IPC_RMID, NULL);

   // if (fork() == 0)
   // {
   //    while (1)
   //       if (read(0, msg.str, 1024) > 0)
   //       {
   //          msg.msgtype = M_SRV;
   //          printf("SND | %s\n", msg.str);
   //          msgsnd(from_server, &msg, sizeof(msbuf) - sizeof(long), IPC_NOWAIT);
   //       }
   // }
   // else
   // {
   //    close(0);
   //    while (1)
   //       while (msgrcv(from_server, &msg, sizeof(msbuf) - sizeof(long), M_CLI, IPC_NOWAIT) != -1)
   //          printf("RCV | %s\n", msg.str);
   // }
}