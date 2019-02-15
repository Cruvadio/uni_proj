#include <stdio.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include "io_server_client.h"

struct sembuf wait_for_server_op[1] = {CLIENTS_SEM, -1, 0};
struct sembuf cmd_sem[1] = {CMD_SEM,-1, 0 };
struct sembuf end_command_sem[1] = {CMD_SERV, 1, 0};
struct sembuf disconnect_from_server_sem[1] = {CLIENTS_SEM, 1, 0};

int main(int argc, char *argv[])
{
    int shm_id, sem_id;
    Mem *mem;
    FILE* file = NULL;
    key_t klutch;
    klutch=ftok("/bin/sh", 8);
    shm_id = shmget(klutch, sizeof(Mem), 0);
    if (argc < 2)
    {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }
    if (shm_id==-1)
    {
        perror("create shmem error");
        return IPC_ERROR;
    }
    sem_id = semget(klutch, 5, 0);
    if (sem_id==-1)
    {
        perror("create sem error");
        return IPC_ERROR;
    }
    
    file = fopen (argv[1], "r");
    if (file == NULL)
    {
        fprintf(stderr, "Can't open file\n");
        return 1;
    }
    mem = shmat(shm_id, NULL, 0);
    if(mem == (Mem*)-1)
    {
        perror ("shmat error");
        return IPC_ERROR;
    }
    
    if (!semctl(sem_id, CLIENTS_SEM, GETVAL))
        printf("Server is bysy\n");
    
    if (semop(sem_id, wait_for_server_op, 1) == -1)
    {
        perror("semop wake up clients failed");
        return IPC_ERROR;
    }
    
    if ((argc > 2) && !strcmp(argv[2], "--finish"))
    {
        mem->client_pid = getpid();
        mem->cmd = FINISH_CMD;
        if (semop(sem_id, end_command_sem, 1))
        {
            perror("command ending failed");
            return IPC_ERROR;
        }
    }
    else
    {
        mem->client_pid = getpid();
        mem->cmd = WELCOME_CMD;
        if (semop(sem_id, end_command_sem, 1))
        {
            perror("command ending failed");
            return IPC_ERROR;
        }
    }
    
    while(1)
    {
        int ch, i;
        char buf[513];
        if(semop(sem_id, cmd_sem, 1) == -1)
        {
            perror("semop wait client failed");
            return IPC_ERROR;
        }
        if (mem->cmd == RESULT_CMD)
        {
            if (mem->result)
                printf ("%s is divided by 3\n", mem->data);
            else
                printf ("%s is not divided by 3\n", mem->data);
        }
        i = 0;
        while(i < 512)
        {
            ch = fgetc(file);
            if (ch == EOF)
            {
                mem->cmd = DISCONNECT_CMD;
                if (semop(sem_id, end_command_sem, 1))
                {
                    perror("command ending failed");
                    return IPC_ERROR;    
                }
                return NO_ERROR;
            }
            if (ch == '\n')
            {
                buf[i] = '\0';
                break;
            }
            buf[i++] = ch;
        }
        
        strcpy(mem->data, buf);
        mem->cmd = DATA_CMD;
        
        if(semop(sem_id, end_command_sem, 1))
        {
            perror("wait for commands failed");
            return IPC_ERROR;
        }
    }/* END WHILE(1) */
    return UNKNOWN_ERROR;
}

