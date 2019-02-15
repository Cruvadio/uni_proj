#include <stdio.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "io_server_client.h"

struct sembuf wake_up_clients_op[1] = {CLIENTS_SEM, 1, 0};
struct sembuf cmd_sem[1] = {CMD_SERV,-1, 0 };
struct sembuf ready_for_commands_sem[1] = {CMD_SEM, 1, 0};
int sem_id, shm_id;

void handler (int signum);

int main()
{
    Mem *mem;
    key_t klutch;
    klutch=ftok("/bin/sh", 8);
    
    signal(SIGINT, handler);
    shm_id = shmget(klutch, sizeof(Mem), IPC_CREAT| IPC_EXCL| 0660);
    if (shm_id==-1)
    {
        perror("create shmem error");
        return IPC_ERROR;
    }
    sem_id = semget(klutch, 5, IPC_CREAT| IPC_EXCL| 0660);
    if (sem_id==-1)
    {
        perror("create sem error");
        return IPC_ERROR;
    }
    
    
    mem = shmat(shm_id, NULL, 0);
    if(mem == (Mem*)-1)
    {
        perror ("shmat error");
        return IPC_ERROR;
    }
    /* Initialization */
    mem->client_pid = 0;
    mem->cmd = 0;
    mem->data[0] = '\0';
    mem->result = 0;
    
    if (semop(sem_id, wake_up_clients_op, 1) == -1)
    {
        perror("semop wake up clients failed");
        return IPC_ERROR;
    }
    
    while(1)
    {
        if(semop(sem_id, cmd_sem, 1) == -1)
        {
            perror("semop wait client failed");
            return IPC_ERROR;
        }
        switch(mem->cmd)
        {
            case WELCOME_CMD:
                printf("Client's pid = %d\n",(int) mem->client_pid);
                break;
            case FINISH_CMD:
                printf("Client with pid %d has initialized finish\n\n",(int) mem->client_pid);
                semctl(sem_id, 5, IPC_RMID);
                shmctl(shm_id, IPC_RMID, 0);
                exit(NO_ERROR);
                break;
            case DATA_CMD:
            {
                int i, sum = 0;
                for (i = 0; i < strlen(mem->data); i++)
                {
                    sum += mem->data[i] - '0';
                }
                if (sum % 3 == 0)
                    mem->result = 1;
                else mem->result = 0;
                mem->cmd = RESULT_CMD;
                break;
            }
            case DISCONNECT_CMD:
                printf("Client with pid %d has disconnected.\n", (int) mem->client_pid);
                mem->client_pid = 0;
                if (semop(sem_id, wake_up_clients_op, 1) == -1)
                {
                    perror("semop wake up clients failed");
                    return IPC_ERROR;
                }
                break;
            default:
                fprintf(stderr, "Unknown command!\n");
                break;
        }
        
        if(semop(sem_id, ready_for_commands_sem, 1))
        {
            perror("wait for commands failed");
            return IPC_ERROR;
        }
    }/* END WHILE(1) */
    return UNKNOWN_ERROR;
}

void handler (int signum)
{
    signal(signum , handler);
    
    semctl(sem_id, 5, IPC_RMID);
    shmctl(shm_id, IPC_RMID, 0);
    
    exit(NO_ERROR);
}

