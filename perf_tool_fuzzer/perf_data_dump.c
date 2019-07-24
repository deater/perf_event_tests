#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "create_perf_data.h"

#include "perf_data.h"

#define LITTLE_ENDIAN	0
#define BIG_ENDIAN	1

static struct perf_header ph;
static int file_endian=LITTLE_ENDIAN;

static int dump_header(int fd) {

	int result,magic_version=0;
	char magic[9];

	result=read(fd,&ph,sizeof(struct perf_header));
	if (result<sizeof(struct perf_header)) {
		fprintf(stderr,"Could not read header: %s\n",strerror(errno));
		return -1;
	}

	/* check magic value */
	memcpy(magic,ph.magic,8);
	magic[8]=0;

	if (!strncmp(magic,"PERFILE",7)) {
		magic_version=magic[7]-'0';
		file_endian=LITTLE_ENDIAN;
	}
	else {
		if (!strncmp(magic+1,"ELIFREP",7)) {
			magic_version=magic[0]-'0';
			file_endian=BIG_ENDIAN;
			fprintf(stderr,"Unsupported endianess!\n");
			return -1;
		}
		fprintf(stderr,"Unknown magic %s\n",magic);
		return -1;
	}

	if (magic_version!=2) {
		fprintf(stderr,"Unsupported magic version %d\n",
			magic_version);
		return -1;
	}






	return 0;

}


int main(int argc, char **argv) {

	int fd,result;

	fd=open("perf.data",O_RDONLY);
	if (fd<0) {
		fprintf(stderr,"Trouble opening perf.data: %s\n",
			strerror(errno));
		return -1;
	}

	result=dump_header(fd);
	if (result<0) {
		return -1;
	}

	close(fd);

	return 0;

}
