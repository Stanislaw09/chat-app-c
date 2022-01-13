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
#include <dirent.h>
#include <errno.h>

#include "const.h"

typedef struct
{
   long type;
   char message[1024];
   char option[256];
   char client[256];
} msbuf;

void create_dir(char *path);

void ftok_init();

key_t get_id_from_string(char string[]);