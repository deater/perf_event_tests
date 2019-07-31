#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "random.h"
#include "sanitise.h"

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


#define UNICODE_PAGESIZE	16384

static char random_unicode[UNICODE_PAGESIZE];

static int random_string(char *dest, int size) {

	int r,offset,length=0;

	/* refresh unicode page occasionally */
	r=rand()%128;
	if (r==0) gen_unicode_page(random_unicode,UNICODE_PAGESIZE);

	if (size>UNICODE_PAGESIZE) {
		fprintf(stderr,"Error! request too big of a string!\n");
		exit(1);
	}

	offset=rand()%(UNICODE_PAGESIZE-size);

	/* randomly pick size */
	r=rand()%4;
	switch(r) {
		case 0:
		case 1:
			length=rand()%size;
			break;
		case 2:
		case 3:
			length=size;
			break;
	}

	memcpy(dest,random_unicode+offset,length);

	/* randomly NUL terminate */
	r=rand()%16;
	switch(r) {
		case 0:
			/* do nothing */
			break;
		default:
			if (length>0) dest[length-1]=0;
			break;
	}




	return 0;
}


static int create_perf_header(int fd) {

	int i;

	uint64_t temp64;

	/* Fuzz the header magic */

	switch(rand()%16) {
		case 0 ... 11:
			/* valid case */
			memcpy(ph.magic,"PERFILE2",8);
			break;
		case 12:/* change the version number up */
			memcpy(ph.magic,"PERFILE2",8);
			ph.magic[7]=(rand()%10)+'0';
			break;
		case 13:/* change the version number up */
			memcpy(ph.magic,"PERFILE2",8);
			ph.magic[7]=rand();
			break;
		case 14:/* random string */
			random_string(ph.magic,8);
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

	int len,i;
	char data[4096];

	len=rand()%4096;

	for(i=0;i<len;i++) {
		data[i]=rand();
	}

	write(fd,data,len);

	return len;

}

/*
struct build_id_event {
        struct perf_event_header header;
        pid_t                    pid;
        uint8_t                  build_id[24];
        char                     filename[header.size - offsetof(struct build_id_event, filename)];
};
*/

static int create_flag_section_build_id(int fd) {

	int len;
	char data[4096];
	struct build_id_event *bev;
	int i;

	bev=(struct build_id_event *)data;

	bev->header.type=0;
	bev->header.misc=1;
	bev->header.size=100;

	bev->pid=rand();

	for(i=0;i<24;i++) bev->build_id[i]=rand();

	random_string(bev->filename,bev->header.size-64);

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
			break;
		case 2:
			string_len=rand()%8192;
			random_string(string->string,string_len);
			break;
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


static int create_flag_section_string_list(int fd) {

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
			break;
		case 2:
			string_len=rand()%8192;
			random_string(string->string,string_len);
			break;
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



static int create_flag_section_u64(int fd) {

	int i;
	uint64_t value;

	i=rand()%16;
	switch(i) {
		case 0:
			value=rand64();
			break;
		case 1:
			value=0;
			break;
		default:
			value=rand64()%256000000;
			break;

	}

	write(fd,&value,8);

	return 8;

}

/*
struct nr_cpus {
       uint32_t nr_cpus_available; // CPUs not yet onlined 
       uint32_t nr_cpus_online;
};
*/

static uint32_t nr_cpus_available,nr_cpus_online;

static int create_flag_section_nrcpus(int fd) {

	int len,i;

	i=rand()%16;
	switch(i) {
		/* create random value */
		case 0:
		case 1:
			nr_cpus_available=rand();
			break;
		/* create realistic value */
		default:
			nr_cpus_available=rand()%128;
			break;

	}

	i=rand()%16;
	switch(i) {
		/* create random value */
		case 0:
		case 1:
			nr_cpus_online=rand();
			break;
		/* create random realistic value */
		case 2:
		case 3:
			nr_cpus_online=rand()%128;
			break;
		/* create realistic value */
		case 4:
			nr_cpus_online=nr_cpus_available-rand()%4;
			break;
		case 5:
			nr_cpus_online=nr_cpus_available+rand()%4;
			break;

		/* create realistic value */
		default:
			nr_cpus_online=nr_cpus_available;
			break;

	}

	/* FIXME: truncate? */

	len=8;
	write(fd,&nr_cpus_available,sizeof(uint32_t));
	write(fd,&nr_cpus_online,sizeof(uint32_t));

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
		case HEADER_VERSION:
			*len=create_flag_section_string(fd);
			break;
		case HEADER_ARCH:
			*len=create_flag_section_string(fd);
			break;
		case HEADER_NRCPUS:
			*len=create_flag_section_nrcpus(fd);
			break;
		case HEADER_CPUDESC:
			*len=create_flag_section_string(fd);
			break;
		case HEADER_CPUID:
			*len=create_flag_section_string(fd);
			break;
		case HEADER_TOTAL_MEM:
			*len=create_flag_section_u64(fd);
			break;
		case HEADER_CMDLINE:
			*len=create_flag_section_string_list(fd);
			break;
		default:
			*len=create_flag_section_random(fd);
			break;
	}

	return 0;
}

static int create_perf_attributes(int fd) {

	int i;
	unsigned char fake_attr[128];

	ph.attrs.offset=168;
	ph.attrs.size=128;

	for(i=0;i<128;i++) fake_attr[i]=rand();
	write(fd,fake_attr,128);

//	lseek(fd,128,SEEK_CUR);

	return 0;
}

static int create_perf_data(int fd) {

	int i;
	unsigned char data[4096];

	ph.data.offset=296;
	ph.data.size=1576;

	for(i=0;i<1576;i++) data[i]=rand();
	write(fd,data,1576);

//	lseek(fd,1576,SEEK_CUR);

	return 0;
}


int create_perf_data_file(void) {

	int fd,flag_count=0,i,len=0;
	off_t flag_header_offset,flag_section_offset;
	struct perf_file_section temp_section;

	/* setup random strings */
	gen_unicode_page(random_unicode,4096);

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
	create_perf_attributes(fd);

	/* Next the data */
	create_perf_data(fd);

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
