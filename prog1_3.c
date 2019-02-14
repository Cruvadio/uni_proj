#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>


#define LOG "log"

pid_t pids[3] = {-1, -1, -1};
int i;

void redirect_stream ()
{
    int fd;
    fd = open (LOG, O_CREAT| O_APPEND| O_WRONLY, 0660);
    dup2(fd, 1);
    close(fd);
}

void handler2 (int signum)
{
    kill(pids[i], SIGKILL);
    wait(NULL);
}

void handler1 (int signum)
{
    i = 0;
    signal(signum, handler2);
    for (i = 0; i < 3; i++)
    {
        if (pids[i] != -1)
        {
            kill(pids[i], SIGTERM);
            alarm(1);
            wait(NULL);
        }
    }
    exit(0);
}

int main (int argc, char* argv[])
{
    int status = 0;
    int num_sec;
    if (argc < 6)
    {
        fprintf(stderr, "NOT ENOUGH ARGUMENTS\n");
        return 1;
    }
    
    num_sec = atoi(argv[5]);

    signal(SIGALRM, handler1);
    alarm(num_sec);

    pids[0] = fork();
    if (!pids[0])
    {
        redirect_stream();
        execlp(argv[1], argv[1], argv[2], NULL);
        perror(argv[1]);
        return 1;
    }
    wait(NULL);
    pids[0] = -1;
    pids[1] = fork();
    if (!pids[1])
    {
        redirect_stream();
        execlp(argv[3], argv[3], NULL);
        perror(argv[3]);
        return 1;
    }
    wait(&status);
    pids[1] = -1;
    
    if (WIFEXITED(status))
    {
        if (!WEXITSTATUS(status))
        {
            pids[2] = fork();
            if (!pids[2])
            {
                execlp(argv[4], argv[4], NULL);
                perror(argv[4]);
                return 1;
            }

        }
    }
    wait(NULL);
    return 0;
}
