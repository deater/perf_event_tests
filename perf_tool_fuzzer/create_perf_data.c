#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "random.h"

#include "create_perf_data.h"

struct perf_file_section {
	uint64_t offset;	/* offset from start of file */
	uint64_t size;		/* size of the section */
};

struct perf_header {
	char magic[8];		/* PERFILE2 */
	uint64_t size;		/* size of the header */
	uint64_t attr_size;	/* size of an attribute in attrs */
	struct perf_file_section attrs;
	struct perf_file_section data;
	struct perf_file_section event_types;
	uint64_t flags;
	uint64_t flags1[3];
};

static int create_perf_header(int fd) {

	int i;

	struct perf_header ph;

	switch(rand()%3) {
		case 0:	memcpy(ph.magic,"PERFILE2",8);
			break;
		case 1:	memcpy(ph.magic,"PERFILE2",8);
			ph.magic[7]=rand();
			break;
		case 2:
			for(i=0;i<8;i++) ph.magic[i]=rand();
			break;
	}

	switch(rand()%3) {
		case 0: ph.size=sizeof(struct perf_header);
			break;
		case 1: ph.size=rand()%128;
			break;
		case 2: ph.size=rand64();
			break;
	}

	/* FIXME: write random size */
	write(fd,&ph,sizeof(struct perf_header));

	return 0;
}

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
	create_perf_header(fd);


	/* FIXME: randomly truncate */
	/* FIXME: randomly sprinkle zeros */

	close(fd);

	return 0;

}
