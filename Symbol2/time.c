#include <sys/time.h>


double return_seconds (struct timeval t1, struct timeval t2)
{
	return (t2.tv_sec - t1.tv_sec) - (double) (t2.tv_usec - t1.tv_usec)/ 1000000;
}
