#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


int main (int argc, char *argv[])
{
	int fd;
	ssize_t rd;
	uint32_t mtr_size;
	unsigned int i;
	double num, sum = 0;

	if (argc < 2)
	{
		fprintf(stderr, "No args.\n");
		return 1;
	}

	fd = open(argv[1], O_RDONLY, 0);
	if (fd == -1)
	{
		fprintf(stderr, "Can't open file %s because of %s\n.", argv[1], strerror(errno));
		return 1;
	}


	rd = read (fd, &mtr_size, sizeof(uint32_t));

	if (rd!= sizeof(uint32_t))

	{
		fprintf(stderr, "There is no int of 32 bytes.\n");
		return 1;
	}
	for (i = 0; i <mtr_size;i++)
	{
		if (i != 0)
        {
            if (i == 1)
            {
                if (lseek(fd,(mtr_size - 1 )*sizeof(double), SEEK_CUR)== -1L)
                {
                    fprintf(stderr, "Seek error\n");
                    return 1;
                }
            }
            else
            {
                if (lseek(fd,(mtr_size)*sizeof(double), SEEK_CUR)== -1L)
                {
                    fprintf(stderr, "Seek error\n");
                    return 1;
                }
            }
        }
		rd = read(fd, &num, sizeof(double));
		if (rd != sizeof(double))
		{
			fprintf(stderr, "There is no double because of %s.\n", strerror(errno));
				return 1;
		}
		sum += num;
		printf("%lf ",num); 

	}
	printf("\n%lf\n", sum);
	close(fd);
	return 0;


}
