#include "common.h"

void create_dir(char *path)
{
    DIR *dir = opendir(path);
    if (dir)
    {
        closedir(dir);
    }
    else if (ENOENT == errno)
    {
        char cmd[1024] = "mkdir -p ";
        strcat(cmd, path);
        system(cmd);
    }
}

void ftok_init()
{
    create_dir(TEMP_DIR);
}

key_t get_id_from_string(char string[])
{
    char path[1024] = TEMP_DIR;
    char cmd[1024];

    strcat(path, string);
    strcpy(cmd, "touch ");
    strcat(cmd, path);
    system(cmd);
    return ftok(path, PROJ_ID);
}