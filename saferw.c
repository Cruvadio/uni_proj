#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>


int saferead (int fd, void *a, size_t size)
{
    size_t already_read = 0;
    while (already_read < size)
    {
        ssize_t ret_val;
        ret_val = read (fd, a + already_read, size - already_read);
        if (ret_val == 0)
            return -1;
        if (ret_val == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            else
                return -1;
        }
        already_read += ret_val;
    }
    return 0;
}

int safewrite (int fd,const void *a, size_t size)
{
    size_t already_write = 0;
    while (already_write < size)
    {
        ssize_t ret_val;
        ret_val = write (fd, a + already_write, size - already_write);
        if (ret_val == 0)
            return -1;
        if (ret_val == -1)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            else
                return -1;
        }
        already_write += ret_val;
    }
    return 0;
}

int readstr (int fd, char* buf, size_t max_size)
{
    int i = 0;
    char ch;
    while (i < max_size - 1)
    {
        if (saferead(fd, &ch, sizeof(char)))
        {
            close(fd);
            return -1;
        }
        if (ch == EOF)
        {
            close(fd);
            return -1;
        }
        if (ch == '\n')
        {
            buf[i++] = '\n';
            buf[i] = '\0';
            return 0;
        }
        else
        {
            buf[i++] = ch;
        }
    }
    buf[i] = '\0';
    return 0;
}
