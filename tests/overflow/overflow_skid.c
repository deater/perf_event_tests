/* overflow_skid.c  */
/* by Vince Weaver   vincent.weaver@maine.edu */

/* Test what happens if we have a user event */
/* but skid means it returns results from in the kernel */

/* Original behavior is to return the kernel sample */
/* This leaks kernel address info though. */

/* On x86_64 the two addresses you usually see are: */
/*	entry_SYSCALL_64	*/
/*	irq_work_interrupt	*/

/* This test became relevant with 4.12-rc4 */
/* cc1582c231ea041fbc68861dfaf957eaf902b829 */
/* The "security" fix that dropped user samples with kernel addresses */
/* This was reverted in 6a8a75f3235724c5941a33e287b2f98966ad14c5 4.13-rc2 */


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
#include "test_utils.h"
#include "perf_helpers.h"
#include "matrix_multiply.h"

#include "perf_barrier.h"

#include "parse_record.h"

static int quiet;
static char test_string[]="Testing if we can skid into kernel...";

#define MMAP_PAGES 16

#define NUM_EVENTS 2

struct {
	int fd;
	int overflows;
	int individual_overflow;
} events[NUM_EVENTS];


struct {
	char name[BUFSIZ];
	int type;
	int config;
	int period;
} event_values[NUM_EVENTS] = {
	{ "perf::instructions",
		PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 1000000},
	{ "perf::instructions",
		PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS, 2000000},
};


#define MAX_ADDR_LIST	256

static struct found_address_type {
	long long address;
	int count;
	int type;
	int valid;
} found_addresses[MAX_ADDR_LIST];


int add_address(long long addr, int type) {

	int i;

	for(i=0;i<MAX_ADDR_LIST;i++) {
		if ((found_addresses[i].valid) &&
			(found_addresses[i].address==addr)) {

			if (type!=found_addresses[i].type) {
				if (!quiet) printf("Types don't match!\n");
				test_fail(test_string);
			}

			found_addresses[i].count++;
			return 0;
		}

	}

	for(i=0;i<MAX_ADDR_LIST;i++) {
		if (!found_addresses[i].valid) {


			found_addresses[i].address=addr;
			found_addresses[i].type=type;
			found_addresses[i].valid=1;
			found_addresses[i].count=1;
			return 0;
		}
	}

	if (!quiet) printf("Ran out of address slots\n");
	test_fail(test_string);


	return 0;
}

/* Linux KASLR randomization makes things a pain */
/* You can use cat /proc/kallsyms as root to get the addresses */

long long perf_mmap_addr( void *our_mmap, int mmap_size,
                        long long prev_head,
                        int sample_type, int read_format, long long reg_mask,
                        struct validate_values *validate,
                        int quiet, int *events_read,
                        int raw_type ) {

	struct perf_event_mmap_page *control_page = our_mmap;
	long long head,offset;
	int size;
	long long bytesize,prev_head_wrap;

	unsigned char *data;

        struct perf_event_header *event;

	void *data_mmap=our_mmap+getpagesize();

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
	//printf("Allocated %lld bytes at %p\n",bytesize,data);

	prev_head_wrap=prev_head%bytesize;

	//printf("Copying %lld bytes from (%p)+%lld to (%p)+%d\n",
	//        bytesize-prev_head_wrap,data_mmap,prev_head_wrap,data,0);

	memcpy(data,(unsigned char*)data_mmap + prev_head_wrap,
		bytesize-prev_head_wrap);
        //printf("Copying %d bytes from %d to %d\n",
        //        prev_head_wrap,0,bytesize-prev_head_wrap);

	memcpy(data+(bytesize-prev_head_wrap),(unsigned char *)data_mmap,
		prev_head_wrap);

	offset=0;

	while(offset<size) {

		//printf("Offset %d Size %d\n",offset,size);
		event = ( struct perf_event_header * ) & data[offset];

		/********************/
		/* Print event Type */
		/********************/

		offset+=8; /* skip header */

		switch(event->type) {

		case PERF_RECORD_SAMPLE:
			if (sample_type & PERF_SAMPLE_IP) {
				long long ip;
				memcpy(&ip,&data[offset],sizeof(long long));

				add_address(ip,event->misc);

//				if (!quiet) {
//					printf("\tPERF_SAMPLE_IP, "
//						"IP: %llx\n",ip);
//				}
                                offset+=8;
                        }
			break;
		default:
			printf("Unknown sample type!\n");
			break;
		}
	}

	control_page->data_tail=head;

	free(data);

	return head;
}


static int validate_results(void) {

	int i;
	int unfiltered_kernel_count=0;

	if (!quiet) printf("\tAddress\t\tCount\tType\n");
	for(i=0;i<MAX_ADDR_LIST;i++) {
		if (found_addresses[i].valid) {
			if (!quiet) {
				printf("%16llx\t%d\t",
					found_addresses[i].address,
					found_addresses[i].count);
			switch(found_addresses[i].type & PERF_RECORD_MISC_CPUMODE_MASK) {
				case PERF_RECORD_MISC_CPUMODE_UNKNOWN:
					printf("PERF_RECORD_MISC_CPUMODE_UNKNOWN"); break;
				case PERF_RECORD_MISC_KERNEL:
					printf("PERF_RECORD_MISC_KERNEL"); break;
				case PERF_RECORD_MISC_USER:
					printf("PERF_RECORD_MISC_USER"); break;
				case PERF_RECORD_MISC_HYPERVISOR:
					printf("PERF_RECORD_MISC_HYPERVISOR"); break;
				case PERF_RECORD_MISC_GUEST_KERNEL:
					printf("PERF_RECORD_MISC_GUEST_KERNEL"); break;
				case PERF_RECORD_MISC_GUEST_USER:
					printf("PERF_RECORD_MISC_GUEST_USER"); break;
				default:
					printf("Unknown %d!\n",found_addresses[i].type); break;
			}
			printf("\n");
			}
			if (found_addresses[i].type!=PERF_RECORD_MISC_USER) {
				if (found_addresses[i].address!=0) {
					unfiltered_kernel_count++;
				}
			}

		}
	}
	return unfiltered_kernel_count;
}

/* Count the events */
/* Don't bother disabling/refreshing? */
static void our_handler(int signum,siginfo_t *info, void *uc) {

	int ret,i;
	int fd = info->si_fd;

	//  ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	for(i=0;i<NUM_EVENTS;i++) {
		if (events[i].fd==fd) {
			events[i].overflows++;
			break;
		}
	}

	if (i==NUM_EVENTS) printf("fd %d not found\n",fd);

	//printf("fd: %d overflowed\n",fd);
	//ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,1);

	(void) ret;

}


int main(int argc, char** argv) {

	int ret,i,matches=0;

	struct perf_event_attr pe;

	struct sigaction sa;
	void *our_mmap[NUM_EVENTS];


	long long counts[NUM_EVENTS];
	long long prev_head=0;

	long long global_sample_type=0,global_sample_regs_user=0;

	for(i=0;i<NUM_EVENTS;i++) {
		events[i].fd=-1;
		events[i].overflows=0;
	}

	memset(found_addresses,0,
			sizeof(struct found_address_type)*MAX_ADDR_LIST);

	quiet=test_quiet();

	/*********************************************************************/
	if (!quiet) printf("This tests simultaneous overflow.\n");
	/*********************************************************************/

	/*****************************/
	/* set up our signal handler */
	/*****************************/

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	/* Use Real Time signal, because it is queued                */
	/* If we just use SIGIO then we lose signals if one comes in */
	/* while we were handling a previous overflow.               */

	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	/***********************/
	/* get expected counts */
	/***********************/

	for(i=0;i<NUM_EVENTS;i++) {
		memset(&pe,0,sizeof(struct perf_event_attr));

		pe.type=event_values[i].type;
		pe.size=sizeof(struct perf_event_attr);
		pe.config=event_values[i].config;
		pe.sample_period=event_values[i].period;
		pe.sample_type=0;
		pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
		pe.disabled=1;
		pe.pinned=0;
		pe.exclude_kernel=1;
		pe.exclude_hv=1;
		pe.wakeup_events=1;

		arch_adjust_domain(&pe,quiet);

		events[i].fd=perf_event_open(&pe,0,-1,-1,0);
		if (events[i].fd<0) {
			fprintf(stderr,"Error opening leader %llx\n",pe.config);
			test_fail(test_string);
		}

		our_mmap[i]=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
			PROT_READ|PROT_WRITE, MAP_SHARED, events[i].fd, 0);

		fcntl(events[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(events[i].fd, F_SETSIG, SIGRTMIN+2);
		fcntl(events[i].fd, F_SETOWN,getpid());

		ioctl(events[i].fd, PERF_EVENT_IOC_RESET, 0);

		if (!quiet) {
			printf("\tEvent %s with period %d\n",
				event_values[i].name,
				event_values[i].period);
		}

		ret=ioctl(events[i].fd, PERF_EVENT_IOC_ENABLE,0);

		if (!quiet) printf("\t");
		naive_matrix_multiply(quiet);

		ret=ioctl(events[i].fd, PERF_EVENT_IOC_DISABLE,0);

		if (!quiet) {
			printf("\tfd %d overflows: %d (%s/%d)\n",
				events[i].fd,events[i].overflows,
				event_values[i].name,
				event_values[i].period);
		}

		if (events[i].overflows==0) {
			if (!quiet) printf("No overflow events generated.\n");
			test_fail(test_string);
		}

		munmap(our_mmap[i],(1+MMAP_PAGES)*getpagesize());
		close(events[i].fd);

		events[i].individual_overflow=events[i].overflows;

		events[i].overflows=0;
		events[i].fd=-1;
	}


	/**********************************/
	/* test overflow for both         */
	/**********************************/

	if (!quiet) {
		printf("Testing matrix matrix multiply\n");
		for(i=0;i<NUM_EVENTS;i++) {
			printf("\tEvent %s with period %d\n",
				event_values[i].name,
				event_values[i].period);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {

		memset(&pe,0,sizeof(struct perf_event_attr));

		pe.type=event_values[i].type;
		pe.size=sizeof(struct perf_event_attr);
		pe.config=event_values[i].config;
		pe.sample_period=event_values[i].period;
		//xpe.sample_type=PERF_SAMPLE_ID;
		// pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
		pe.disabled=1;
		pe.pinned=1;
		pe.exclude_kernel=1;
		pe.exclude_hv=1;
		pe.wakeup_events=1;

		pe.sample_type=PERF_SAMPLE_IP;
		global_sample_type=pe.sample_type;

		arch_adjust_domain(&pe,quiet);

		events[i].fd=perf_event_open(&pe,0,-1,-1,0);
		if (events[i].fd<0) {
			fprintf(stderr,"Error opening %llx\n",pe.config);
			test_fail(test_string);
		}

		our_mmap[i]=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
				PROT_READ|PROT_WRITE, MAP_SHARED,
				events[i].fd, 0);

		fcntl(events[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(events[i].fd, F_SETSIG, SIGRTMIN+2);
		fcntl(events[i].fd, F_SETOWN,getpid());

		ioctl(events[i].fd, PERF_EVENT_IOC_RESET, 0);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(events[i].fd, PERF_EVENT_IOC_ENABLE,0);
	}

	if (ret<0) {
		if (!quiet) {
			printf("Error with PERF_EVENT_IOC_ENABLE of "
				"group leader: %d %s\n",
				errno,strerror(errno));
		}
		test_fail(test_string);
	}

	if (!quiet) printf("\t");
	naive_matrix_multiply(quiet);

	for(i=0;i<NUM_EVENTS;i++) {
		ret=ioctl(events[i].fd, PERF_EVENT_IOC_DISABLE,0);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		if (!quiet) {
			printf("\tfd %d overflows: %d (%s/%d)\n",
				events[i].fd,events[i].overflows,
				event_values[i].name,event_values[i].period);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		if (events[i].overflows==0) {
			if (!quiet) printf("No overflow events generated.\n");
			test_fail(test_string);
		}
	}

	for(i=0;i<NUM_EVENTS;i++) {
		read(events[i].fd,&counts[i],8);
		if (!quiet) printf("\tRead %d %lld\n",events[i].fd,counts[i]);
	}

	for(i=0;i<NUM_EVENTS;i++) {
		close(events[i].fd);
	}

	/* test validity */
	for(i=0;i<NUM_EVENTS;i++) {

		if (!quiet) {
			printf("Event %s/%d Expected %lld Got %d\n",
				event_values[i].name, event_values[i].period,
				counts[i]/event_values[i].period,events[i].overflows);
		}
		if (counts[i]/event_values[i].period!=events[i].overflows) {
		}
		else {
			matches++;
		}
	}

	if (matches!=NUM_EVENTS) {
		fprintf(stderr,"Wrong number of overflows!\n");
	}

	/* Finished, read MMAP */
	perf_mmap_addr(our_mmap[0],MMAP_PAGES,prev_head,
		global_sample_type,0,global_sample_regs_user,
		NULL,quiet,NULL,RAW_NONE);

	perf_mmap_addr(our_mmap[1],MMAP_PAGES,prev_head,
		global_sample_type,0,global_sample_regs_user,
		NULL,quiet,NULL,RAW_NONE);

	ret=validate_results();

	if (ret!=0) {
		if (!quiet) printf("Returned %d unfiltered kernel addresses\n",ret);
		test_known_kernel_bug(test_string);
	}

	/* This is from the earlier failure, but for debugging want */
	/* to print the address info */

	if (matches!=NUM_EVENTS) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}

