#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <limits.h>
int saferead (int fd, void* a, size_t size);
int safewrite (int fd, void* a, size_t size);
int readstr(int fd, char* buf, size_t max_size);
int main (int argc, char *argv[])
{
    pid_t pids[2];
    int sock_fd;
    int port, err;
    char buf[256];
    struct sockaddr_in *sockaddr;
    struct addrinfo hints, *result;
    char msg[50];

    if (argc<3)
    {
        fprintf (stderr, "Not enough arguments\n");
        return 1;
    }
    
    port = atoi(argv[2]);
    if (port > USHRT_MAX || port < 0)
    {
        fprintf (stderr, "Invalid port number - %d\n", port);
        return 1;
    }

    memset(&msg, 0, 50*sizeof(char));

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_canonname = NULL;

    err = getaddrinfo (argv[1], NULL, &hints, &result);
    if (err != 0)
    {
        fprintf (stderr, "Can't get address for name %s: %s\n", argv[1], gai_strerror(err));
        return 1; 
    }

    sockaddr = (struct sockaddr_in *)result->ai_addr;
    sockaddr->sin_port = htons(port);

    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror ("Socket has not been created");
        return 1;
    }

    if (connect(sock_fd, result->ai_addr, result->ai_addrlen))
    {
        fprintf (stderr, "Can't connect to '%s:%d' %s\n", argv[1], port, strerror(errno));
        return 1;
    }

    freeaddrinfo (result);
   
    pids[0] = fork();
    if (pids[0] == 0)
    {
        close(1);
        while (1)
        {
            char ch;
            ch = getchar();
            if (safewrite (sock_fd, &ch, sizeof(char)))
            {
                fprintf (stderr, "Can't write char\n");
                shutdown(sock_fd, SHUT_WR);
                return 1;
            }
            if (ch == EOF)
            {
                shutdown(sock_fd, SHUT_WR);
                return 0;
            }
        }
    }
    
    pids[1] = fork();
    if (pids[1] == 0)
    {
        close(0);
        while(1)
        {
            if (readstr (sock_fd, buf, 256) == -1)
            {
                fprintf(stderr, "Disconnected from server\n");
                shutdown(sock_fd, SHUT_RD);
                return 0;
            }

            printf("%s", buf);
            if (buf[strlen(buf) - 1] != '\n') printf("\n");
        }
    }
    
    while (wait(NULL) != -1);

    close (sock_fd);
    return 0;
}
