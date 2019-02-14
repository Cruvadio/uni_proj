#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


int main (int argc, char *argv[])
{
	int fd;
	int wr;
	uint32_t mtr_size;
	unsigned int i;

	double num;

	fd = open(argv[1], O_WRONLY|O_CREAT,0600);
	if (fd == -1)
	{
		fprintf(stderr, "Can't open file %s because of %s", argv[1], strerror(errno));
	}
	scanf("%d",&mtr_size);

	wr = write(fd, &mtr_size, sizeof(uint32_t));

	for (i = 0; i < mtr_size*mtr_size; i++)
	{
		num=i;
		wr = write (fd,&num, sizeof(double));
	}
	close(fd);
	return 0;
}
