#include <stdio.h>
#include <stddef.h>
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
static int header_written_size=0;

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
		case 14:
			ph.flags1[rand()%3]=rand64();
			break;
		case 15:
			for(i=0;i<3;i++) {
				ph.flags1[i]=rand64();
			}
			break;
	}


	/* write random size output (truncate sometimes) */

	switch(rand()%16) {
		case 0 ...14:
			/* proper case */
			header_written_size=sizeof(struct perf_header);
			break;
		case 15:
			header_written_size=rand()%16384;
			break;
	}

	write(fd,&ph,header_written_size);

	return 0;
}

static int create_flag_section_random(int fd) {

	/* FIXME: random string? */

	int len,i;
	char data[4096];

	len=rand()%4096;

	for(i=0;i<len;i++) {
		data[i]=rand();
	}

	write(fd,data,len);

	return len;

}



static int create_flag_section_build_id(int fd) {

	/* FIXME: random string? */

	int len;
	char data[4096];
	struct build_id_event *bev;

	bev=(struct build_id_event *)data;

	bev->header.type=0;
	bev->header.misc=1;
	bev->header.size=100;

	bev->pid=rand();

	strncpy(bev->filename,"VMW",4);

	len=rand()%512;

	write(fd,data,len);

	return len;

}


static int create_flag_section_string(int fd) {

	int len,i;
	char data[16384];
	struct perf_header_string *string;
	int string_len;

	string=(struct perf_header_string *)data;

	i=rand()%16;
	switch(i) {
		/* create realistic string */
		case 0:
			strcpy(string->string,"VMW");
			string_len=strlen(string->string);
			string->len=string_len;
			break;
		case 1:
			string_len=rand()%8192;
			for(i=0;i<string_len;i++) string->string[i]=rand();
			string->len=rand();
		default:
			string_len=rand()%8192;
			for(i=0;i<string_len;i++) string->string[i]=rand();
			string->len=string_len;
			break;

	}

	/* FIXME: truncate? */

	len=string_len+4;
	write(fd,data,len);

	return len;

}



static int create_flag_section(int fd,int which, int *len) {

	*len=0;

	switch(which) {

		case HEADER_RESERVED:
			*len=create_flag_section_random(fd);
			break;
		case HEADER_TRACING_DATA:
			*len=create_flag_section_random(fd);
			break;
		case HEADER_BUILD_ID:
			*len=create_flag_section_build_id(fd);
			break;
		case HEADER_HOSTNAME:
			*len=create_flag_section_string(fd);
			break;
		case HEADER_OSRELEASE:
			*len=create_flag_section_string(fd);
			break;
		default:
			*len=create_flag_section_random(fd);
			break;
	}

	return 0;
}


int create_perf_data_file(void) {

	int fd,flag_count=0,i,len=0;
	off_t flag_header_offset,flag_section_offset;
	struct perf_file_section temp_section;

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

	/* Padding????? Not sure what that's about yet */
	/* FIXME: seek to 168 */
	lseek(fd,168,SEEK_SET);

	/* Next some perf attributes? */
	/* FIXME: seek 128 bytes further */
	ph.attrs.offset=168;
	ph.attrs.size=128;
	lseek(fd,128,SEEK_CUR);


	/* Next the data */
	/* FIXME: seek 1576 bytes further */
	ph.data.offset=296;
	ph.attrs.size=1576;
	lseek(fd,1576,SEEK_CUR);

	/**************************/
	/* Next the flag sections */
	/**************************/
	for(i=0;i<64;i++) {
		if (ph.flags&1UL<<i) flag_count++;
	}

	flag_section_offset=lseek(fd,0,SEEK_CUR);
	flag_section_offset+=flag_count*sizeof(struct perf_file_section);

//	printf("Flag count: %d\n",flag_count);

	for(i=0;i<flag_count;i++) {

		/* save current offset */
		flag_header_offset=lseek(fd,0,SEEK_CUR);

		create_flag_section(fd,i,&len);

		/* restore current offset */
		lseek(fd,flag_header_offset,SEEK_SET);

		temp_section.offset=flag_section_offset;
		temp_section.size=len;

		/* FIXME: fuzz this? */
		write(fd,&temp_section,sizeof(struct perf_file_section));

		flag_section_offset+=len;

	}


	/* re-write the header */
	lseek(fd,0,SEEK_SET);
	write(fd,&ph,header_written_size);

	/* FIXME: randomly truncate */
	/* FIXME: randomly sprinkle zeros */

	close(fd);

	return 0;

}
