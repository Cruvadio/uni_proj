#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <inttypes.h>


#define FILE_NAME "file.bin"


int check_binary(uint32_t num, int n)
{
    uint32_t mask = 1;
    int begin = 0, end = 1;

    if (!num) return 0;
    while (1)
    {
        if (num & mask) break;
        mask = mask << 1;
        begin++;
    }
    
    mask = mask << 1;

    while (1)
    {
        if (num & mask) break;
        mask = mask << 1;
        if (!mask)
            return 0;
        end++;
    }
    if (end <= n)
        return 1;
    else
        return 0;
}


void init_file()
{
    int fd;
    uint32_t i = 0;
    int num = 5;

    fd = open(FILE_NAME,O_CREAT | O_WRONLY | O_TRUNC, 0660);

    for (i = 0; i < num; i++)
    {
        write(fd, &i, sizeof(uint32_t));
    }

    close(fd);

}


int main (int argc, char *argv[])
{
    int n, m;
    int k = 0, j = 0;
    int i = 0;
    int fd;
    uint32_t* nums = NULL;
    if (argc < 3)
    {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }
    
    n = atoi(argv[1]);
    m = atoi(argv[2]);
    
    init_file();
    nums = (uint32_t*) malloc(sizeof(uint32_t) * m);
    /*
     * TO DO:
     * CHECK NULL
     */
    
    fd = open(FILE_NAME, O_APPEND | O_RDWR);

    lseek(fd, 0, SEEK_SET);

    while (1)
    {
        int is_end = 0;
        for (i = 0; i < m; i++)
        {
            if (read(fd, nums + i, sizeof(uint32_t)) != sizeof(uint32_t))
            {
                is_end = 1;
                break;
            }
            k++;
        }
        lseek(fd, j * sizeof(uint32_t), SEEK_SET);
        for (i = 0; i < m; i++)
        {
            if (check_binary(nums[i], n))
            {
                j++;
                write(fd, nums + i, sizeof(uint32_t));
            }
        }
        if (is_end)
            break;
        lseek(fd,k * sizeof(uint32_t), SEEK_SET);
    }

    return 0;
}
