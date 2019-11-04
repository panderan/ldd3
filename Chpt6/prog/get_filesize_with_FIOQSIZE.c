#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int fd = 0;
	long long size = 0;

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("Open Failed\n");
		exit(1);
	}
	
	ioctl(fd, FIOQSIZE, &size);
	close(fd);
	printf("Size: %ld\n", size);
	return 0;
}
