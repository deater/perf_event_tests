/* print_record_sample.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"

#include "perf_helpers.h"
#include "perf_barrier.h"

int perf_event_open(struct perf_event_attr *hw_event_uptr,
                    pid_t pid, int cpu, int group_fd, unsigned long flags) {

        return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
                        group_fd, flags);
}

long long perf_mmap_read( void *our_mmap, int mmap_size,
                    long long prev_head,
		    int sample_type, int read_format);



int sample_type=PERF_SAMPLE_TIME|PERF_SAMPLE_READ;

int read_format=
                PERF_FORMAT_GROUP |
                PERF_FORMAT_ID |
                PERF_FORMAT_TOTAL_TIME_ENABLED |
                PERF_FORMAT_TOTAL_TIME_RUNNING;




#define MMAP_DATA_SIZE 8
#define NUM_CPUS 1
#define NUM_EVENTS 2
//#define SAMPLE_FREQUENCY 100000000ULL /* 10Hz */
//#define SAMPLE_FREQUENCY 10000000ULL /* 100Hz */
#define SAMPLE_FREQUENCY 1000000ULL /* 1kHz */

int main(int argc, char** argv) {

	void *mmap_buffers[NUM_CPUS];
	long long prev_head[NUM_CPUS];
	static int fd[NUM_EVENTS][NUM_CPUS];

	int ret;
	int mmap_pages=1+MMAP_DATA_SIZE;

	int i,j;

	struct perf_event_attr pe;

	/* Initialize */
	for(i=0;i<NUM_CPUS;i++) {
		prev_head[i]=0;
	}

	/* Set up Leader Event */
	for(i=0;i<NUM_CPUS;i++) {

		memset(&pe,0,sizeof(struct perf_event_attr));

//		pe.type=PERF_TYPE_HARDWARE;
//		pe.config=PERF_COUNT_HW_REF_CPU_CYCLES;


		pe.type=PERF_TYPE_SOFTWARE;
		pe.config=PERF_COUNT_SW_CPU_CLOCK;

		pe.size=sizeof(struct perf_event_attr);
		pe.sample_period=SAMPLE_FREQUENCY;
		pe.sample_type=sample_type;
		pe.read_format=read_format;
		pe.disabled=1;
		pe.pinned=1;

		fd[0][i]=perf_event_open(&pe,-1,0,-1,0);
		if (fd[0][i]<0) {
			fprintf(stderr,"Problem opening leader %d %s\n",
			i,strerror(errno));
		}
		/* Open Cycles Event */

		memset(&pe,0,sizeof(struct perf_event_attr));

		pe.type=PERF_TYPE_HARDWARE;
		pe.size=sizeof(struct perf_event_attr);
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;
//		pe.sample_type=sample_type;
//		pe.read_format=read_format;
		pe.disabled=0;

		fd[1][i]=perf_event_open(&pe,-1,0,fd[0][i],0);
		if (fd[1][i]<0) {
			fprintf(stderr,"Error opening %llx\n",pe.config);
		}

		mmap_buffers[i]=mmap(NULL, mmap_pages*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, fd[0][i], 0);
	}

	for(i=0;i<NUM_CPUS;i++) {

		ioctl(fd[0][i], PERF_EVENT_IOC_RESET, 0);
		ioctl(fd[0][i], PERF_EVENT_IOC_ENABLE,0);

	}

	/* Work to do */
 	sleep(1);


	/* Disable */
	for(i=0;i<NUM_CPUS;i++) {
		ret=ioctl(fd[0][i], PERF_EVENT_IOC_DISABLE,0);
	}

	/* Read */
	for(i=0;i<NUM_CPUS;i++) {
		printf("(* CPU %d *)\n",i);
		prev_head[i]=perf_mmap_read(mmap_buffers[i],
				MMAP_DATA_SIZE,prev_head[i],
				sample_type,read_format);
	}

	/* Cleanup */
	for(i=0;i<NUM_CPUS;i++) {
		munmap(mmap_buffers[i],mmap_pages*getpagesize());
		for(j=0;j<NUM_EVENTS;j++) {
			close(fd[j][i]);
		}
	}

	(void) ret;

	return 0;
}

/* Urgh who designed this interface */
static int handle_struct_read_format(unsigned char *sample,
				     int read_format,
					long long timestamp) {

	int offset=0,i;
	static long long values[NUM_EVENTS];

	if (read_format & PERF_FORMAT_GROUP) {
		long long nr,time_enabled=0,time_running=0;

		memcpy(&nr,&sample[offset],sizeof(long long));
		offset+=8;

		if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) {
			memcpy(&time_enabled,&sample[offset],sizeof(long long));
			offset+=8;
		}
		if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) {
			memcpy(&time_running,&sample[offset],sizeof(long long));
			offset+=8;
		}

		if ((read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) &&
			(read_format & PERF_FORMAT_TOTAL_TIME_ENABLED)) {
			if (time_running!=time_enabled) {
				printf("ERROR! MULTIPLEXING\n");
			}
		}

		for(i=0;i<nr;i++) {
			long long value, id=0;

			memcpy(&value,&sample[offset],sizeof(long long));
			offset+=8;

			if (read_format & PERF_FORMAT_ID) {
				memcpy(&id,&sample[offset],sizeof(long long));
				offset+=8;
			}
			printf("%lld %lld (* %lld *)\n",timestamp,value-values[i],id);
			values[i]=value;

		}
	}
	else {

		long long value,time_enabled,time_running,id;

		memcpy(&value,&sample[offset],sizeof(long long));
		printf("\t\tValue: %lld ",value);
		offset+=8;

		if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) {
			memcpy(&time_enabled,&sample[offset],sizeof(long long));
			printf("enabled: %lld ",time_enabled);
			offset+=8;
		}
		if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) {
			memcpy(&time_running,&sample[offset],sizeof(long long));
			printf("running: %lld ",time_running);
			offset+=8;
		}
		if (read_format & PERF_FORMAT_ID) {
			memcpy(&id,&sample[offset],sizeof(long long));
			printf("id: %lld ",id);
			offset+=8;
		}
		printf("\n");
	}

	return offset;
}


long long perf_mmap_read( void *our_mmap, int mmap_size,
                    long long prev_head,
		    int sample_type, int read_format) {

	struct perf_event_mmap_page *control_page = our_mmap;
	long long head,offset;
	int size;
	long long bytesize,prev_head_wrap;
	static long long begin_time;

	unsigned char *data;

	void *data_mmap=our_mmap+getpagesize();

	begin_time=0;

	if (mmap_size==0) return 0;

	if (control_page==NULL) {
		fprintf(stderr,"ERROR mmap page NULL\n");
		return -1;
	}

	head=control_page->data_head;
	rmb(); /* Must always follow read of data_head */

	size=head-prev_head;

	//printf("Head: %lld Prev_head=%lld\n",head,prev_head);
	//printf("%d new bytes\n",size);

	bytesize=mmap_size*getpagesize();

	if (size>bytesize) {
		printf("error!  we overflowed the mmap buffer %d>%lld bytes\n",
			size,bytesize);
	}

	data=malloc(bytesize);
	if (data==NULL) {
		return -1;
	}

	prev_head_wrap=prev_head%bytesize;

	//   printf("Copying %d bytes from %d to %d\n",
	//	  bytesize-prev_head_wrap,prev_head_wrap,0);
	memcpy(data,(unsigned char*)data_mmap + prev_head_wrap,
		bytesize-prev_head_wrap);

	//printf("Copying %d bytes from %d to %d\n",
	//	  prev_head_wrap,0,bytesize-prev_head_wrap);

	memcpy(data+(bytesize-prev_head_wrap),(unsigned char *)data_mmap,
		prev_head_wrap);

	struct perf_event_header *event;


	offset=0;

	while(offset<size) {

		//printf("Offset %d Size %d\n",offset,size);
		event = ( struct perf_event_header * ) & data[offset];

		/********************/
		/* Print event Type */
		/********************/
		switch(event->type) {
				case PERF_RECORD_MMAP:
					printf("PERF_RECORD_MMAP"); break;
				case PERF_RECORD_LOST:
					printf("PERF_RECORD_LOST"); break;
				case PERF_RECORD_COMM:
					printf("PERF_RECORD_COMM"); break;
				case PERF_RECORD_EXIT:
					printf("PERF_RECORD_EXIT"); break;
				case PERF_RECORD_THROTTLE:
					printf("PERF_RECORD_THROTTLE"); break;
				case PERF_RECORD_UNTHROTTLE:
					printf("PERF_RECORD_UNTHROTTLE"); break;
				case PERF_RECORD_FORK:
					printf("PERF_RECORD_FORK"); break;
				case PERF_RECORD_READ:
					printf("PERF_RECORD_READ");
					break;
				case PERF_RECORD_SAMPLE:
					//printf("PERF_RECORD_SAMPLE [%x]",sample_type);
					break;
				case PERF_RECORD_MMAP2:
					printf("PERF_RECORD_MMAP2"); break;
				default: printf("UNKNOWN %d",event->type); break;
			}


			/* Both have the same value */
			if (event->misc & PERF_RECORD_MISC_MMAP_DATA) {
				printf(",PERF_RECORD_MISC_MMAP_DATA or PERF_RECORD_MISC_COMM_EXEC ");
			}

			if (event->misc & PERF_RECORD_MISC_EXACT_IP) {
				printf(",PERF_RECORD_MISC_EXACT_IP ");
			}

			if (event->misc & PERF_RECORD_MISC_EXT_RESERVED) {
				printf(",PERF_RECORD_MISC_EXT_RESERVED ");
			}

		offset+=8; /* skip header */

		/***********************/
		/* Print event Details */
		/***********************/

		switch(event->type) {

		/* Lost */
		case PERF_RECORD_LOST: {
			long long id,lost;
			memcpy(&id,&data[offset],sizeof(long long));
			printf("\tID: %lld\n",id);
			offset+=8;
			memcpy(&lost,&data[offset],sizeof(long long));
			printf("\tLOST: %lld\n",lost);
			offset+=8;
			}
			break;

		/* COMM */
		case PERF_RECORD_COMM: {
			int pid,tid,string_size;
			char *string;

			memcpy(&pid,&data[offset],sizeof(int));
			printf("\tPID: %d\n",pid);
			offset+=4;
			memcpy(&tid,&data[offset],sizeof(int));
			printf("\tTID: %d\n",tid);
			offset+=4;

			/* FIXME: sample_id handling? */

			/* two ints plus the 64-bit header */
			string_size=event->size-16;
			string=calloc(string_size,sizeof(char));
			memcpy(string,&data[offset],string_size);
			printf("\tcomm: %s\n",string);
			offset+=string_size;
			if (string) free(string);
			}
			break;

		/* Fork */
		case PERF_RECORD_FORK: {
			}
			break;

		/* mmap */
		case PERF_RECORD_MMAP: {
			}
			break;

		/* mmap2 */
		case PERF_RECORD_MMAP2: {
			}
			break;

		/* Exit */
		case PERF_RECORD_EXIT: {
			}
			break;

		/* Throttle/Unthrottle */
		case PERF_RECORD_THROTTLE:
		case PERF_RECORD_UNTHROTTLE: {
			long long throttle_time,id,stream_id;

			memcpy(&throttle_time,&data[offset],sizeof(long long));
			printf("\tTime: %lld\n",throttle_time);
			offset+=8;
			memcpy(&id,&data[offset],sizeof(long long));
			printf("\tID: %lld\n",id);
			offset+=8;
			memcpy(&stream_id,&data[offset],sizeof(long long));
			printf("\tStream ID: %lld\n",stream_id);
			offset+=8;

			}
			break;

		/* Sample */
		case PERF_RECORD_SAMPLE: {
			long long time=0;

			if (sample_type & PERF_SAMPLE_IP) {
				long long ip;
				memcpy(&ip,&data[offset],sizeof(long long));
				printf("\tPERF_SAMPLE_IP, IP: %llx\n",ip);
				offset+=8;
			}
			if (sample_type & PERF_SAMPLE_TID) {
				int pid, tid;
				memcpy(&pid,&data[offset],sizeof(int));
				memcpy(&tid,&data[offset+4],sizeof(int));

				printf("\tPERF_SAMPLE_TID, pid: %d  tid %d\n",pid,tid);

				offset+=8;
			}

			if (sample_type & PERF_SAMPLE_TIME) {
				memcpy(&time,&data[offset],sizeof(long long));
				//printf("\tPERF_SAMPLE_TIME, time: %lld\n",time);
				if (begin_time==0) begin_time=time;
				offset+=8;
			}
			if (sample_type & PERF_SAMPLE_ADDR) {
				long long addr;
				memcpy(&addr,&data[offset],sizeof(long long));
				printf("\tPERF_SAMPLE_ADDR, addr: %llx\n",addr);
				offset+=8;
			}
			if (sample_type & PERF_SAMPLE_ID) {
				long long sample_id;
				memcpy(&sample_id,&data[offset],sizeof(long long));
				printf("\tPERF_SAMPLE_ID, sample_id: %lld\n",sample_id);
				offset+=8;
			}
			if (sample_type & PERF_SAMPLE_STREAM_ID) {
				long long sample_stream_id;
				memcpy(&sample_stream_id,&data[offset],sizeof(long long));
				printf("\tPERF_SAMPLE_STREAM_ID, sample_stream_id: %lld\n",sample_stream_id);
				offset+=8;
			}
			if (sample_type & PERF_SAMPLE_CPU) {
				int cpu, res;
				memcpy(&cpu,&data[offset],sizeof(int));
				memcpy(&res,&data[offset+4],sizeof(int));
				printf("\tPERF_SAMPLE_CPU, cpu: %d  res %d\n",cpu,res);
				offset+=8;
			}
			if (sample_type & PERF_SAMPLE_PERIOD) {
				long long period;
				memcpy(&period,&data[offset],sizeof(long long));
				printf("\tPERF_SAMPLE_PERIOD, period: %lld\n",period);
				offset+=8;
			}
			if (sample_type & PERF_SAMPLE_READ) {
				int length;

				length=handle_struct_read_format(&data[offset],
						read_format,time-begin_time);
				if (length>=0) offset+=length;
			}
			if (sample_type & PERF_SAMPLE_CALLCHAIN) {
			}
			if (sample_type & PERF_SAMPLE_RAW) {
			}

			if (sample_type & PERF_SAMPLE_BRANCH_STACK) {
	   		}
			if (sample_type & PERF_SAMPLE_REGS_USER) {
			}

			if (sample_type & PERF_SAMPLE_REGS_INTR) {
			}

			if (sample_type & PERF_SAMPLE_STACK_USER) {
			}

			if (sample_type & PERF_SAMPLE_WEIGHT) {
			}

			if (sample_type & PERF_SAMPLE_DATA_SRC) {
			}

			if (sample_type & PERF_SAMPLE_IDENTIFIER) {
			}

			if (sample_type & PERF_SAMPLE_TRANSACTION) {
			}
			}
			break;

			default: printf("\tUnknown type %d\n",event->type);
		}
	}

	control_page->data_tail=head;

	free(data);

	return head;

}
