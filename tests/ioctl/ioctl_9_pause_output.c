/* ioctl_9_pause_output.c  */
/* by Vince Weaver   vincent.weaver@maine.edu */

/* Test out ioctl(PERF_IOC_PAUSE_OUTPUT); */

/* TODO: check that lost samples are made */

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
#include "test_utils.h"

#include "instructions_testcode.h"


static char test_string[]="Testing ioctl(PERF_IOC_PAUSE_OUTPUT)...";
static int count=0;

static long long prev_head=0;
static void *our_mmap;
static int quiet;

static int ip_samples=0,lost_samples=0;
static int quiet;

#define MMAP_PAGES	2

long long perf_mmap_read( void *our_mmap, int mmap_size,
			long long prev_head) {

	struct perf_event_mmap_page *control_page = our_mmap;
	long long head,offset;
	int size;
	long long bytesize,prev_head_wrap;
	long long ip,id,lost;

	unsigned char *data;

	void *data_mmap=our_mmap+getpagesize();

	if (mmap_size==0) return 0;

        if (control_page==NULL) {
                fprintf(stderr,"ERROR mmap page NULL\n");
                return -1;
        }

        head=control_page->data_head;
        rmb(); /* Must always follow read of data_head */

        size=head-prev_head;

//	printf("Head: %lld Prev_head=%lld\n",head,prev_head);
//	printf("%d new bytes\n",size);

        bytesize=mmap_size*getpagesize();

        if (size>bytesize) {
                printf("error!  we overflowed the mmap buffer %d>%lld bytes\n",
                        size,bytesize);
        }

	data=malloc(bytesize);
	if (data==NULL) {
		return -1;
	}

//	printf("Allocated %lld bytes at %p\n",bytesize,data);

	prev_head_wrap=prev_head%bytesize;

//	printf("Copying %lld bytes from (%p)+%lld to (%p)+%d\n",
//		bytesize-prev_head_wrap,data_mmap,prev_head_wrap,data,0);

	memcpy(data,(unsigned char*)data_mmap + prev_head_wrap,
		bytesize-prev_head_wrap);

//	printf("Copying %lld bytes from %d to %lld\n",
//		prev_head_wrap,0,bytesize-prev_head_wrap);

	memcpy(data+(bytesize-prev_head_wrap),(unsigned char *)data_mmap,
		prev_head_wrap);

	struct perf_event_header *event;
	offset=0;

	while(offset<size) {

	//	printf("Offset %lld Size %d\n",offset,size);
		event = ( struct perf_event_header * ) & data[offset];
		offset+=8; /* skip header */

		switch(event->type) {
			case PERF_RECORD_SAMPLE:
				memcpy(&ip,&data[offset],sizeof(long long));
				ip_samples++;
//				if (!quiet) printf("\tPERF_SAMPLE_IP, IP: %llx\n",ip);
				offset+=8;
				break;
			case PERF_RECORD_LOST:
				memcpy(&id,&data[offset],sizeof(long long));
				memcpy(&lost,&data[offset+8],sizeof(long long));
//				if (!quiet) printf("\tPERF_SAMPLE_LOST: %lld %lld\n",id,lost);
				lost_samples+=lost;
				offset+=16;
				break;
			default:
				printf("Unknown type %d\n",event->type);
				exit(0);
				break;
		}
	}
//	mb();
	control_page->data_tail=head;

	free(data);

	return head;

}

static void our_handler(int signum,siginfo_t *oh, void *blah) {

	int ret;

	count++;

	prev_head=perf_mmap_read(our_mmap,MMAP_PAGES-1,prev_head);


//	printf("COUNT2=%d %d\n",count,oh->si_fd);
	ret=ioctl(oh->si_fd, PERF_EVENT_IOC_REFRESH,1);
	(void) ret;
}


int main(int argc, char** argv) {

	int i;
	int fd1;
	int ret;
	double result;
	struct perf_event_attr pe;
	int first_count,second_count,third_count;
	size_t pagesize = sysconf(_SC_PAGESIZE);

	struct sigaction sa;

	quiet=test_quiet();

	if (!quiet) {
		printf("\nThis check tests PERF_EVENT_IOC_PAUSE_OUTPUT\n");
	}

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.sample_period=1000000;
	pe.sample_type=PERF_SAMPLE_IP;
	pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
	pe.disabled=1;
	pe.pinned=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.wakeup_events=1;

	arch_adjust_domain(&pe,quiet);

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	our_mmap=mmap(NULL, MMAP_PAGES*pagesize,
		PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

	fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fd1, F_SETSIG, SIGIO);
	fcntl(fd1, F_SETOWN,getpid());

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH, 1);

	if (ret<0) {
		if (!quiet) {
			fprintf(stderr,"Error with PERF_EVENT_IOC_REFRESH "
				"of group leader: %d %s\n",
				errno,strerror(errno));
		}
		test_fail(test_string);
	}

	for(i=0;i<1000;i++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) {
		fprintf(stderr,"\tCode unimplemented\n");
		test_skip(test_string);
	}

	first_count=count;

	ret=ioctl(fd1, PERF_EVENT_IOC_PAUSE_OUTPUT,1);

	for(i=0;i<1000;i++) {
		result=instructions_million();
	}

	second_count=count;

	ret=ioctl(fd1, PERF_EVENT_IOC_PAUSE_OUTPUT,0);

	for(i=0;i<1000;i++) {
		result=instructions_million();
	}

	third_count=count;

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);

	if (!quiet) {
		printf("Total count: %d (%d %d %d)\n",count,
			first_count,second_count,third_count);
		printf("IP samples %d\n",ip_samples);
		printf("Lost samples %d\n",lost_samples);

	}

	(void)our_mmap;

	if (count==1) {
		if (!quiet) fprintf(stderr,"Only counted one overflow.\n");
		test_fail(test_string);
	}
	if ((count<2900) || (count>3100)) {
		if (!quiet) fprintf(stderr,"Wrong number of overflows.\n");
		test_fail(test_string);
	}

	if ((ip_samples<1950) || (ip_samples>2050)) {
		if (!quiet) fprintf(stderr,"Wrong number of ip_samples.\n");
		test_fail(test_string);
	}

	if ((lost_samples<950) || (lost_samples>1050)) {
		if (!quiet) fprintf(stderr,"Wrong number of lost_samples.\n");
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}

