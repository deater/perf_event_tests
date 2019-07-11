#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "create_perf_data.h"

int create_perf_data_file(void) {

	int fd;

	/* Open file */
	fd=open("perf.data",O_WRONLY|O_CREAT,0666);
	if (fd<0) {
		fprintf(stderr,"Error creating file: %s\n",
			strerror(errno));
		return -1;
	}

	/* FIXME: try messing with permissions */

	/* First, a header */

	close(fd);

	return 0;

}
