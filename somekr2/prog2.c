#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

typedef struct 
{
    unsigned char m_Internal[3];
}int24_t;

int int24to32 (int24_t num)
{
    if (num.m_Internal[0] & 0x80)
    {
        return (0xff << 24) | (num.m_Internal[0] << 16) | (num.m_Internal[1] << 8) | (num.m_Internal[2] << 0);
    }
    return (num.m_Internal[2] << 0) | (num.m_Internal[1] << 8) | (num.m_Internal[0] << 16);
}

int main ()
{
    int fd;
    int sum = 0;
    int24_t num;
    fd = open ("int24.bin", O_RDONLY);


    while (read(fd, &num, sizeof(int24_t)) == sizeof(int24_t))
    {
        sum += int24to32(num);
    }

    printf("%d\n", sum);
    
    return 0;
}
