#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "random.h"

#include "../include/perf_event.h"

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

	/* Fuzz the header magic */

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

	/* Fuzz the header size */

	switch(rand()%3) {
		case 0: ph.size=sizeof(struct perf_header);
			break;
		case 1: ph.size=rand()%256;
			break;
		case 2: ph.size=rand64();
			break;
	}

	/* Fuzz the header attr_size */

	switch(rand()%3) {
		case 0: ph.size=sizeof(struct perf_event_attr);
			break;
		case 1: ph.attr_size=rand()%256;
			break;
		case 2: ph.attr_size=rand64();
			break;
	}

	/* attrs, data, event_types */
	/* we'll come back and fill those in later */


	/* Fuzz the header flags */

	switch(rand()%3) {
		case 0: ph.flags=0;
			break;
		case 1: ph.flags=rand()%256;
			break;
		case 2: ph.flags=rand64();
			break;
	}

	/* Fuzz the reserved fields */
	/* Do this slightly more rarely */
	switch(rand()%16) {
		case 0 ... 13: /* do nothing */
			break;
		case 14: ph.flags1[rand()%4]=rand64();
			break;
		case 15: for(i=0;i<3;i++) {
				ph.flags1[i]=rand64();
			}
			break;
	}


	/* write random size output (truncate sometimes) */

	switch(rand()%16) {
		case 0 ...14:
			/* proper case */
			write(fd,&ph,sizeof(struct perf_header));
			break;
		case 15:
			i=rand()%16384;
			write(fd,&ph,i);
			break;
	}
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
