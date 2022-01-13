// messages from client
#define JOIN 1
#define JOIN_ROOM 2
#define CREATE_ROOM 3
#define MESSAGE_TO_ROOM 4
#define MESSAGE_TO_CLIENT 5
#define DISPLAY_ROOMS 6
#define DISPLAY_CLIENTS_IN_ROOMS 7
#define EXIT_ROOM 8
#define DISPLAY_RECENT_MESSAGES 9

// messages from server
#define OK 10
#define USER_EXISTS 11
#define NO_SUCH_ROOM 12
#define ROOM_EXISTS 13
#define USER_NOT_IN_ROOM 14
#define WRONG_RECIPIENT 15
#define CLIENTS_IN_ROOMS 16
#define ROOMS 17

// types of messages
#define MESSAGE_FROM_CLIENT 20
#define MESSAGE_FROM_ROOM 21

// display a nice oof and exit
#define OOF { printf("\n\n▒██████╗▒   ▒██████╗▒   ███████╗\n██╔═══██╗   ██╔═══██╗   ██╔════╝\n██║▒▒▒██║   ██║▒▒▒██║   █████╗▒▒\n██║▒▒▒██║   ██║▒▒▒██║   ██╔══╝▒▒\n╚██████╔╝   ╚██████╔╝   ██║▒▒▒▒▒\n▒╚═════╝▒   ▒╚═════╝▒   ╚═╝▒▒▒▒▒   ...we have a problem here\n\n"); \
              exit(420); }

#define PROJ_ID 2137
#define TEMP_DIR "./temp/"

#define QUEUE 0