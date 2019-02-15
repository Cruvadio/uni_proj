#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define CMD_SERV 0
#define CLIENTS_SEM 1
#define CMD_SEM 2

#define WELCOME_CMD 1
#define DATA_CMD 2
#define FINISH_CMD 3
#define DISCONNECT_CMD 4
#define RESULT_CMD 5

#define UNKNOWN_ERROR 4
#define IPC_ERROR 1
#define NO_ERROR 0
typedef struct
{
    pid_t client_pid;
    int cmd;
    char data[513];
    int result;
} Mem;
