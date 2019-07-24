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

#include "perf_data.h"

#define ENDIAN_SWAP_U64(val) ((uint64_t) ( \
    (((uint64_t) (val) & (uint64_t) 0x00000000000000ffULL) << 56) | \
    (((uint64_t) (val) & (uint64_t) 0x000000000000ff00ULL) << 40) | \
    (((uint64_t) (val) & (uint64_t) 0x0000000000ff0000ULL) << 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00000000ff000000ULL) <<  8) | \
    (((uint64_t) (val) & (uint64_t) 0x000000ff00000000ULL) >>  8) | \
    (((uint64_t) (val) & (uint64_t) 0x0000ff0000000000ULL) >> 24) | \
    (((uint64_t) (val) & (uint64_t) 0x00ff000000000000ULL) >> 40) | \
    (((uint64_t) (val) & (uint64_t) 0xff00000000000000ULL) >> 56)))


static struct perf_header ph;

static int create_perf_header(int fd) {

	int i;

	uint64_t temp64;

	/* Fuzz the header magic */

	switch(rand()%16) {
		case 0 ... 12:
			/* valid case */
			memcpy(ph.magic,"PERFILE2",8);
			break;
		case 13:/* change the version number up */
			memcpy(ph.magic,"PERFILE2",8);
			ph.magic[7]=(rand()%10)+'0';
			break;
		case 14:/* change the version number up */
			memcpy(ph.magic,"PERFILE2",8);
			ph.magic[7]=rand();
			break;
		case 15:
			/* completely random */
			for(i=0;i<8;i++) ph.magic[i]=rand();
			break;
	}
	switch(rand()%4) {
		case 0 ... 2: /* do nothing */
			break;
		case 3: /* swap endian */
			memcpy(&temp64,ph.magic,8*sizeof(char));
			temp64=ENDIAN_SWAP_U64(temp64);
			memcpy(ph.magic,&temp64,8*sizeof(char));
			break;
	}
	/* FIXME: if we endian swap, we should remember and endian swap */
	/* all kinds of values */


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





	/* FIXME: If there were flags in the header, write out sections here??? */


	/* FIXME: randomly truncate */
	/* FIXME: randomly sprinkle zeros */

	close(fd);

	return 0;

}
