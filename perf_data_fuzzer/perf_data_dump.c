#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "../include/perf_event.h"

#include "create_perf_data.h"
#include "parse_record.h"
#include "data_read.h"
#include "perf_attr_print.h"


#include "perf_data.h"

#define BUFFER_SIZE 4096

#define LITTLE_ENDIAN	0
#define BIG_ENDIAN	1

static struct perf_header header;
static int file_endian=LITTLE_ENDIAN;

static uint32_t nr_cpus_avail=0;

static int sample_type,read_format;


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

/*
struct build_id_event {
        struct perf_event_header header;
        pid_t                    pid;
        uint8_t                  build_id[24];
        char                     filename[header.size - offsetof(struct build_id_event, filename)];
};
*/

static int read_section_build_id(int fd, int len) {

	int result,i;
	struct build_id_event *bev;
	char data[BUFFER_SIZE];

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"build_id too big...\n");
		return -1;
	}

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

/*
struct nr_cpus {
       uint32_t nr_cpus_available; // CPUs not yet onlined
       uint32_t nr_cpus_online;
};
*/

static int read_section_nrcpus(int fd, int len) {

	int result;
	unsigned char data[BUFFER_SIZE];
	int offset=0;
	uint32_t nr_cpus_available,nr_cpus_online;


	if (len>BUFFER_SIZE) {
		fprintf(stderr,"ncrpus too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read cpus header!: %s\n",
			strerror(errno));
		return -1;
	}

	nr_cpus_available=get_uint32(data,offset);
	offset+=4;

	nr_cpus_online=get_uint32(data,offset);
	offset+=4;

	printf("\t\tCPUs available: %d, CPUs online: %d\n",
		nr_cpus_available,nr_cpus_online);

	nr_cpus_avail=nr_cpus_available;

	return 0;
}


/*
struct perf_header_string {
       uint32_t len;
       char string[len]; // zero terminated
};
*/
static int read_section_string(int fd, int len,char *name) {

	int result;
	unsigned char data[BUFFER_SIZE];
	char temp_string[BUFFER_SIZE];
	int offset=0,length;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"String length too big!\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read %s header!: %s\n",
			name,strerror(errno));
		return -1;
	}

	length=get_string(data,offset,temp_string);
	offset+=length;

	printf("\t\tLength: %d\n",length-4);
	printf("\t\t%s: %s\n",name,temp_string);

	return 0;
}

/*
struct perf_header_string_list {
     uint32_t nr;
     struct perf_header_string strings[nr]; // variable length records
};
*/

static int read_section_string_list(int fd, int len,char *name) {

	int result,i,offset=0;
	unsigned char data[BUFFER_SIZE];
	char temp_string[BUFFER_SIZE];
	uint32_t number,length;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"String list %d size too big!\n",len);
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read %s header!: %s\n",
			name,strerror(errno));
		return -1;
	}

	number=get_uint32(data,offset);
	offset+=4;

	printf("\t\tContains %d strings\n",number);

	for(i=0;i<number;i++) {

		length=get_string(data,offset,temp_string);
		offset+=length;

		printf("\t\t\t%d: %s\n",
			i,
			temp_string);
	}

	return 0;
}



static int read_section_u64(int fd, int len,char *name) {

	int result,offset=0;
	unsigned char data[BUFFER_SIZE];
	uint64_t value;

	if (len!=sizeof(uint64_t)) {
		fprintf(stderr,"U64 size wrong!\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read %s header!: %s\n",
			name,strerror(errno));
		return -1;
	}

	value=get_uint64(data,offset);

	printf("\t\t%s: %ld\n",name,value);

	return 0;
}

/*
struct {
       uint32_t nr; // number of events
       uint32_t attr_size; // size of each perf_event_attr
       struct {
              struct perf_event_attr attr;  // size of attr_size
              uint32_t nr_ids;
              struct perf_header_string event_string;
              uint64_t ids[nr_ids];
       } events[nr]; // Variable length records
};
*/

static int read_section_event_desc(int fd, int len) {

	int result,i,j,offset=0;
	unsigned char data[BUFFER_SIZE];
	char event_name[BUFFER_SIZE];
	uint32_t nr,attr_size,nr_ids,length;
	uint64_t id;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"event desc size too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read event_desc header!: %s\n",
			strerror(errno));
		return -1;
	}

	nr=get_uint32(data,offset);
	offset+=4;
	attr_size=get_uint32(data,offset);
	offset+=4;


	printf("\t\tNumber of events: %d, each size %d bytes\n",
		nr,attr_size);

	for(i=0;i<nr;i++) {

		printf("\t\t\tEvent data: TODO\n");
		offset+=attr_size;

		nr_ids=get_uint32(data,offset);
		offset+=4;

		printf("\t\t\tEvent %d, Ids %d\n",i,nr_ids);

		length=get_string(data,offset,event_name);
		offset+=length;

		printf("\t\t\tName: %s\n",event_name);
		printf("\t\t\tIds: ");
		for(j=0;j<nr_ids;j++) {
			id=get_uint64(data,offset);
			offset+=8;
			printf("%lx ",id);
		}
		printf("\n");

	}

	return 0;
}


/*
	struct perf_header_string_list cores; // Variable length
	struct perf_header_string_list threads; // Variable length

	Second revision of HEADER_CPU_TOPOLOGY also contains:

	struct {
		uint32_t core_id;
		uint32_t socket_id;
	} cpus[nr];	// 'nr' comes from HEADER_NRCPUS nr_cpu_avail

	Third revision of HEADER_CPU_TOPOLOGY also contains:

	struct perf_header_string_list dies;
	uint32_t die_id[nr_cpus_avail]; // HEADER_NR_CPUS
*/

static int read_section_cpu_topology(int fd, int len) {

	int result,i;
	unsigned char data[BUFFER_SIZE];
	char temp_string[BUFFER_SIZE];
	int string_list_size;
	int offset=0,length;
	uint32_t core_id,socket_id,die_id;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"CPU topo Size too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read cpu_topo header!: %s\n",
			strerror(errno));
		return -1;
	}

	string_list_size=get_uint32(data,offset);
	offset+=4;

	printf("\t\tCores: %d\n",string_list_size);
	for(i=0;i<string_list_size;i++) {
		length=get_string(data,offset,temp_string);
		offset+=length;

		printf("\t\t\t%d: %s\n",
			i,
			temp_string);
	}

	string_list_size=get_uint32(data,offset);
	offset+=4;

	printf("\t\tThreads: %d\n",string_list_size);

	for(i=0;i<string_list_size;i++) {
		length=get_string(data,offset,temp_string);
		offset+=length;
		printf("\t\t\t%d: %s\n",
			i,
			temp_string);
	}

	/* only do the following if len from disk was large enough */
	if (offset>=len) {
		return 0;
	}

	for(i=0;i<nr_cpus_avail;i++) {
		core_id=get_uint32(data,offset);
		offset+=4;
		socket_id=get_uint32(data,offset);
		offset+=4;

		printf("\t\t%d: core_id=%d, socket_id=%d\n",
			i,core_id,socket_id);
	}

	/* only do the following if len from disk was large enough */
	if (offset>=len) {
		//printf("Stopping early %d %d\n",total_size,len);
		return 0;
	}

	string_list_size=get_uint32(data,offset);
	offset+=4;

	printf("\t\tDies: %d\n",string_list_size);

	for(i=0;i<string_list_size;i++) {
		length=get_string(data,offset,temp_string);
		offset+=length;

		printf("\t\t\t%d: %s\n",
			i,
			temp_string);
	}

	for(i=0;i<nr_cpus_avail;i++) {

		die_id=get_uint32(data,offset);
		offset+=4;

		printf("\t\t%d: die_id=%d\n",
			i,die_id);
	}

	return 0;
}


/*
struct {
       uint32_t nr;
       struct {
              uint32_t nodenr;
              uint64_t mem_total;
              uint64_t mem_free;
              struct perf_header_string cpus;
       } nodes[nr]; // Variable length records
};
*/

static int read_section_numa_topology(int fd, int len) {

	int result,i,offset=0,length;
	unsigned char data[BUFFER_SIZE];
	char cpus[BUFFER_SIZE];
	uint32_t nodes,nodenr;
	uint64_t mem_total,mem_free;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"NUMA topo Size too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read cpu_numa header!: %s\n",
			strerror(errno));
		return -1;
	}

	nodes=get_uint32(data,offset);
	offset+=4;

	printf("\t\tNodes: %d\n",nodes);

	for(i=0;i<nodes;i++) {
		nodenr=get_uint32(data,offset);
		offset+=4;

		mem_total=get_uint64(data,offset);
		offset+=8;

		mem_free=get_uint64(data,offset);
		offset+=8;

		length=get_string(data,offset,cpus);
		offset+=length;

		printf("\t\t\tNode %d: \"%s\"\n",
			i,cpus);

		printf("\t\t\t#%d, RAM total %ld, Ram Free %ld\n",
			nodenr,mem_total,mem_free);

	}

	return 0;

}






static int read_section_pmu_mappings(int fd, int len) {

	int result,i,offset=0;
	unsigned char data[BUFFER_SIZE];
	char string[BUFFER_SIZE];
	uint32_t nr,type,length;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"PMU Size too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read pmu_mapping header!: %s\n",
			strerror(errno));
		return -1;
	}

	nr=get_uint32(data,offset);
	offset+=4;

	printf("\t\tPMUs: %d\n",nr);

	for(i=0;i<nr;i++) {
		type=get_uint32(data,offset);
		offset+=4;

		length=get_string(data,offset,string);
		offset+=length;

		printf("\t\t\tPMU %d: Type=%d, \"%s\"\n",i,type,string);

	}

	return 0;

}

/*
u32 version     Currently always 1
        u32 number_of_cache_levels

struct {
        u32     level;
        u32     line_size;
        u32     sets;
        u32     ways;
        struct perf_header_string type;
        struct perf_header_string size;
        struct perf_header_string map;
}[number_of_cache_levels];
*/

static int read_section_cache(int fd, int len) {

	int result,i,offset=0;
	unsigned char data[BUFFER_SIZE];
	char string_type[BUFFER_SIZE];
	char string_size[BUFFER_SIZE];
	char string_map[BUFFER_SIZE];
	uint32_t version,levels,length;
	uint32_t level, line_size, sets, ways;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"Cache Size too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read cache header!: %s\n",
			strerror(errno));
		return -1;
	}

	version=get_uint32(data,offset);
	offset+=4;

	levels=get_uint32(data,offset);
	offset+=4;

	printf("\t\tCache Header Version: %d, Levels: %d\n",version,levels);

	for(i=0;i<levels;i++) {

		level=get_uint32(data,offset);
		offset+=4;

		line_size=get_uint32(data,offset);
		offset+=4;

		sets=get_uint32(data,offset);
		offset+=4;

		ways=get_uint32(data,offset);
		offset+=4;

		length=get_string(data,offset,string_type);
		offset+=length;

		length=get_string(data,offset,string_size);
		offset+=length;

		length=get_string(data,offset,string_map);
		offset+=length;

		printf("\t\t\tCache %d: Type=\"%s\", Size=\"%s\", Map=\"%s\"\n",
			i,string_type,string_size,string_map);
		printf("\t\t\t\tLevel %d, Line_Size: %d, Sets: %d, Ways: %d\n",
			level,line_size,sets,ways);

	}

	return 0;

}

/* two uint64_t */
static int read_section_sample_time(int fd, int len) {

	int result,offset=0;
	unsigned char data[BUFFER_SIZE];
	uint64_t begin,end;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"Sample Time too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read sample time header!: %s\n",
			strerror(errno));
		return -1;
	}

	begin=get_uint64(data,offset);
	offset+=8;

	end=get_uint64(data,offset);
	offset+=8;

	printf("\t\tSample Time:\n");
	printf("\t\t\tBegin: %lu (%lx)\n",begin,begin);
	printf("\t\t\tEnd  : %lu (%lx)\n",end,end);

	return 0;

}

/*
Physical memory map and its node assignments.

The format of data in MEM_TOPOLOGY is as follows:



u64 version;     		Currently 1
u64 block_size_bytes;		/sys/devices/system/memory/block_size_bytes
u64 count;			number of nodes

struct memory_node {
	u64 node_id;			 node index
	u64 size;			size of bitmap
	struct bitmap {
		u64 bitmapsize;
		u64 entries[(bitmapsize/64)+1];
	}
		          bitmap of memory indexes that belongs to node
                        /sys/devices/system/node/node<NODE>/memory<INDEX>
}[count];

*/

static int read_section_sample_topology(int fd, int len) {

	int result,offset=0,i,j,k;
	unsigned char data[BUFFER_SIZE];
	uint64_t version,block_size_bytes,count,node_id,size,bitmap_size;
	uint64_t bitmap;

	if (len>BUFFER_SIZE) {
		fprintf(stderr,"Sample topology too big...\n");
		return -1;
	}

	result=read(fd,data,len);
	if (result!=len) {
		fprintf(stderr,"Couldn't read sample topology header!: %s\n",
			strerror(errno));
		return -1;
	}

	version=get_uint64(data,offset);
	offset+=8;
	block_size_bytes=get_uint64(data,offset);
	offset+=8;
	count=get_uint64(data,offset);
	offset+=8;

	printf("\t\tSample Topology:\n");
	printf("\t\t\tVersion: %lu\n",version);
	printf("\t\t\tBlock size: %lu (0x%lx)\n",
		block_size_bytes,block_size_bytes);
	printf("\t\t\t%ld Nodes:\n",
		count);
	for(i=0;i<count;i++) {
		node_id=get_uint64(data,offset);
		offset+=8;
		size=get_uint64(data,offset);
		offset+=8;
		bitmap_size=get_uint64(data,offset);
		offset+=8;
		printf("\t\t\t\t%d: Node=%ld Size=%ld\n",
			i,node_id,size);
		printf("\t\t\t\t\tBitmap size: %ld\n",bitmap_size);
		printf("\t\t\t\t\t");
		for(j=0;j<(bitmap_size/64)+1;j++) {
			bitmap=get_uint64(data,offset);
			offset+=8;
			printf("%lx: ",bitmap);
			for(k=0;k<64;k++) {
				if (bitmap&1ULL<<k) printf("%d ",k);

			}
		}
		printf("\n");
	}
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
		case HEADER_EVENT_DESC:
			result=read_section_event_desc(fd,length);
			break;
		case HEADER_CPU_TOPOLOGY:
			result=read_section_cpu_topology(fd,length);
			break;
		case HEADER_NUMA_TOPOLOGY:
			result=read_section_numa_topology(fd,length);
			break;
		case HEADER_PMU_MAPPINGS:
			result=read_section_pmu_mappings(fd,length);
			break;
		case HEADER_CACHE:
			result=read_section_cache(fd,length);
			break;
		case HEADER_SAMPLE_TIME:
			result=read_section_sample_time(fd,length);
			break;
		case HEADER_SAMPLE_TOPOLOGY:
			result=read_section_sample_topology(fd,length);
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


static int dump_attributes(int fd) {

//	int i,result=0,length=0;
//	off_t offset;

#define MAX_ATTR_SIZE	4096

	unsigned char raw_attr[MAX_ATTR_SIZE];

	int result;
	struct perf_event_attr *attr;

	printf("\nAttribute section:\n");

	/* seek to attr section */
	result=lseek(fd,header.attrs.offset,SEEK_SET);
	if (result<0) {
		fprintf(stderr,"Trouble seeking!\n");
		return -1;
	}

	if (header.attrs.size>MAX_ATTR_SIZE) {
		fprintf(stderr,"Error! attr too big %ld > %d\n",
			header.attrs.size,MAX_ATTR_SIZE);
		return -1;
	}

	result=read(fd,raw_attr,header.attrs.size);

	attr=(struct perf_event_attr *)raw_attr;

	perf_pretty_print_attr(stdout, attr, 0);

	sample_type=attr->sample_type;
	read_format=attr->read_format;


	return 0;
}

#define MAX_RECORD_SIZE	16384



static int dump_data(int fd) {

	int result=0,length=0;
	off_t offset=0;
	unsigned char data[MAX_RECORD_SIZE];

	printf("\nDATA section:\n");

	/* seek to after data section */
	result=lseek(fd,header.data.offset,SEEK_SET);
	if (result<0) {
		fprintf(stderr,"Trouble seeking!\n");
		return -1;
	}

	while(offset<header.data.size) {

//		printf("VMW: offset=%ld, size=%ld\n",offset,header.data.size);

		result=lseek(fd,header.data.offset+offset,SEEK_SET);

		result=read(fd,data,MAX_RECORD_SIZE);
		if (result<0) {
			fprintf(stderr,"Couldn't read data header!: %s\n",
			strerror(errno));
			return -1;
		}

		length=parse_perf_record(data,sample_type,read_format);
		if (length<0) {
			fprintf(stderr,"Error parsing data!\n");
			return -1;
		}
		offset+=length;
	}

	return 0;
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

	/* Go back and dump attributes section */
	result=dump_attributes(fd);
	if (result<0) {
		return -1;
	}

	/* Go back and dump data section */
	result=dump_data(fd);
	if (result<0) {
		return -1;
	}


	close(fd);

	return 0;

}
