#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>

typedef struct 
{
    unsigned char m_Internal[3];
}int24_t;


int24_t int32to24 (int num)
{
    int24_t i;
    i.m_Internal[0] = ((unsigned char*) &num)[2];
    i.m_Internal[1] = ((unsigned char*) &num)[1];
    i.m_Internal[2] = ((unsigned char*) &num)[0];

    return i;

}

int main ()
{
    int24_t num;
    int i = 0;
    int fd;

    
    fd = open ("int24.bin", O_CREAT| O_WRONLY| O_TRUNC, 0660);
    
    for (i = -1;i >= -4; i--)
    {
        num = int32to24(i);
        write(fd, &num, sizeof(int24_t));
    }
    
    /*
    i = 0x112233;

    num = int32to24(i);

    write(fd, &num, sizeof(int24_t));

    */
    /*
   i = -1;

    num = int32to24(i);
    write(fd, &num, sizeof(int24_t));
    */
    close(fd);
    return 0;
}
