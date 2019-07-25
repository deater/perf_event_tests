#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "create_perf_data.h"

#include "perf_data.h"

#define LITTLE_ENDIAN	0
#define BIG_ENDIAN	1

static struct perf_header header;
static int file_endian=LITTLE_ENDIAN;


static char *section_names[256]={
/*  0 */	"HEADER_RESERVED",
/*  1 */	"HEADER_TRACING_DATA",
/*  2 */	"HEADER_BUILD_ID",
/*  3 */	"HEADER_HOSTNAME",
/*  4 */	"HEADER_OSRELEASE",
/*  5 */	"HEADER_VERSION",
/*  6 */	"HEADER_ARCH",
/*  7 */	"HEADER_NRCPUS",
/*  8 */	"HEADER_CPUDESC",
/*  9 */	"HEADER_CPUID",
/* 10 */	"HEADER_TOTAL_MEM",
/* 11 */	"HEADER_CMDLINE",
/* 12 */	"HEADER_EVENT_DESC",
/* 13 */	"HEADER_CPU_TOPOLOGY",
/* 14 */	"HEADER_NUMA_TOPOLOGY",
/* 15 */	"HEADER_BRANCH_STACK",
/* 16 */	"HEADER_PMU_MAPPINGS",
/* 17 */	"HEADER_GROUP_DESC",
/* 18 */	"HEADER_AUXTRACE",
/* 19 */	"HEADER_STAT",
/* 20 */	"HEADER_CACHE",
/* 21 */	"HEADER_SAMPLE_TIME",
/* 22 */	"HEADER_SAMPLE_TOPOLOGY",
/* 23 */	"HEADER_CLOCKID",
/* 24 */	"HEADER_DIR_FORMAT",
/* 25 */	"HEADER_BPF_PROG_INFO",
/* 26 */	"HEADER_BPF_BTF",
/* 27 */	"HEADER_COMPRESSED",
};

static int dump_header(int fd) {

	int i;
	int result,magic_version=0;
	char magic[9];

	result=read(fd,&header,sizeof(struct perf_header));
	if (result<sizeof(struct perf_header)) {
		fprintf(stderr,"Could not read header: %s\n",strerror(errno));
		return -1;
	}

	/* check magic value */
	memcpy(magic,header.magic,8);
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
	printf("\nHEADER: valid magic %s version %d\n",
		magic,magic_version);

	/* check size */
	if (header.size!=sizeof(struct perf_header)) {
		fprintf(stderr,"Unexpected header size %ld!\n",header.size);
		return -1;
	}
	printf("\theader size: %ld\n",header.size);

	/* check attribute size */
	printf("\tattribute size: %ld\n",header.attr_size);

	/* check attribute section */
	printf("\tattribute_section:\n");
	printf("\t\t%ld bytes at offset %ld\n",
		header.attrs.size,header.attrs.offset);

	/* check data section */
	printf("\tdata_section:\n");
	printf("\t\t%ld bytes at offset %ld\n",
		header.data.size,header.data.offset);

	/* check event_types section */
	printf("\tevent_types_section:\n");
	printf("\t\t%ld bytes at offset %ld\n",
		header.event_types.size,header.event_types.offset);

	/* check flags */
	printf("\tflags: 0x%lx\n",header.flags);

	/* check reserved flags */
	//printf("\tflags1: ");
	for(i=0;i<3;i++) {
		if (header.flags1[i]!=0) {
			printf("Unknown flags1[%d]=0x%lx ",i,
			header.flags1[i]);
			return -1;
		}
	}
	printf("\n");
	return 0;
}

static int read_section_build_id(int fd, int len) {

	int result,i;
	struct build_id_event *bev;
	char data[4096];

	bev=(struct build_id_event *)data;

	result=read(fd,bev,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read build_id header!: %s\n",
			strerror(errno));
		return -1;
	}

	printf("\t\tType: %d, Misc %hd, Size: %hd\n",
		bev->header.type,bev->header.misc,bev->header.size);

	printf("\t\tPid: %d\n",bev->pid);

	printf("\t\tBuild Id: ");
	for(i=0;i<BUILD_ID_SIZE;i++) {
		printf("%02x",bev->build_id[i]);
	}
	printf("\n");
	printf("\t\tFilename: %s\n",bev->filename);



	return 0;
}



static int read_section_nrcpus(int fd, int len) {

	int result;
	struct nr_cpus *cpus;
	char data[4096];

	cpus=(struct nr_cpus *)data;

	result=read(fd,cpus,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read cpus header!: %s\n",
			strerror(errno));
		return -1;
	}

	printf("\t\tCPUs available: %d, CPUs online: %d\n",
		cpus->nr_cpus_available,cpus->nr_cpus_online);

	return 0;
}



static int read_section_string(int fd, int len,char *name) {

	int result;
	char data[4096];

	struct perf_header_string *string;

	string=(struct perf_header_string *)data;

	if (len>4096) {
		fprintf(stderr,"String length too big!\n");
		return -1;
	}

	result=read(fd,string,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read %s header!: %s\n",
			name,strerror(errno));
		return -1;
	}

	printf("\t\tLength: %d\n",string->len);
	printf("\t\t%s: %s\n",name,string->string);

	return 0;
}

static int read_section_string_list(int fd, int len,char *name) {

	int result,i;
	char data[4096],*hack;

	struct perf_header_string_list *string_list;
	struct perf_header_string *string;

	string_list=(struct perf_header_string_list *)data;

	if (len>4096) {
		fprintf(stderr,"String list %d size too big!\n",len);
		return -1;
	}

	result=read(fd,string_list,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read %s header!: %s\n",
			name,strerror(errno));
		return -1;
	}

	printf("\t\tContains %d strings\n",string_list->nr);
	string=string_list->strings;
	for(i=0;i<string_list->nr;i++) {
		printf("\t\t\t%d: (%d) %s\n",
			i,
			string->len,
			string->string);
		hack=(char *)string;
		hack+=(string->len+4);
		string=(struct perf_header_string *)hack;
	}
	return 0;
}



static int read_section_u64(int fd, int len,char *name) {

	int result;
	char data[4096];

	uint64_t *value;

	value=(uint64_t *)data;

	if (len!=sizeof(uint64_t)) {
		fprintf(stderr,"U64 size wrong!\n");
		return -1;
	}

	result=read(fd,value,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read %s header!: %s\n",
			name,strerror(errno));
		return -1;
	}

	printf("\t\t%s: %ld\n",name,*value);

	return 0;
}



static int dump_section(int fd, int which,off_t offset,int length) {

	int result;
	off_t old_offset;

	/* save old offset */
	old_offset=lseek(fd,0,SEEK_CUR);

	/* seek to right place */
	result=lseek(fd,offset,SEEK_SET);

	printf("\tFlag section %d (%s)\n",which,section_names[which]);

	switch(which) {
		case HEADER_RESERVED:
			fprintf(stderr,"Invalid section!\n");
			return -1;
		case HEADER_TRACING_DATA:
			fprintf(stderr,"Invalid section!\n");
			return -1;
		case HEADER_BUILD_ID:
			result=read_section_build_id(fd,length);
			break;
		case HEADER_HOSTNAME:
			result=read_section_string(fd,length,"hostname");
			break;
		case HEADER_OSRELEASE:
			result=read_section_string(fd,length,"OS release");
			break;
		case HEADER_VERSION:
			result=read_section_string(fd,length,"Version");
			break;
		case HEADER_ARCH:
			result=read_section_string(fd,length,"Arch");
			break;
		case HEADER_NRCPUS:
			result=read_section_nrcpus(fd,length);
			break;
		case HEADER_CPUDESC:
			result=read_section_string(fd,length,"CPU Description");
			break;
		case HEADER_CPUID:
			result=read_section_string(fd,length,"CPU ID");
			break;
		case HEADER_TOTAL_MEM:
			result=read_section_u64(fd,length,"Total memory (kB)");
			break;
		case HEADER_CMDLINE:
			result=read_section_string_list(fd,length,"Cmdline");
			break;
		default:
			fprintf(stderr,"Invalid section!\n");
			return -1;
	}

	if (result<0) return result;

	/* restore old offset */
	lseek(fd,old_offset,SEEK_SET);

	return 0;
}

static off_t dump_section_offset(int fd, int *bytes) {

	int result;
	struct perf_file_section section;

	/* read in the section value */
	result=read(fd,&section,sizeof(struct perf_file_section));
	if (result<0) {
		fprintf(stderr,"Trouble reading section\n");
		return -1;
	}

	printf("\t%ld bytes at offset 0x%lx\n",section.size,section.offset);

	*bytes=section.size;

	return section.offset;
}

static int dump_flag_sections(int fd) {

	int i,result=0,length=0;
	off_t offset;

	printf("\nFLAGS sections:\n");

	/* seek to after data section */
	result=lseek(fd,header.data.offset+header.data.size,SEEK_SET);
	if (result<0) {
		fprintf(stderr,"Trouble seeking!\n");
		return -1;
	}

	/* FIXME: 256??? */
	for(i=0;i<64;i++) {

		if (header.flags&(1UL<<i)) {
			offset=dump_section_offset(fd,&length);
			if (offset==-1) return offset;
			result=dump_section(fd,i,offset,length);
			if (result<0) return result;
		}


	}


	return result;
}


int main(int argc, char **argv) {

	int fd,result;

	/* FIXME: get from command line */
	char filename[]="perf.data";

	fd=open(filename,O_RDONLY);
	if (fd<0) {
		fprintf(stderr,"Trouble opening %s: %s\n",
			filename,strerror(errno));
		return -1;
	}

	/* Print title */
	printf("Attempting to dump \"%s\"...\n",filename);

	/* Dump header */
	result=dump_header(fd);
	if (result<0) {
		return -1;
	}

	/* Dump flag sections */
	result=dump_flag_sections(fd);
	if (result<0) {
		return -1;
	}


	close(fd);

	return 0;

}
