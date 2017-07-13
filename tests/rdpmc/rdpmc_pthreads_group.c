/* rdpmc and pthreads have problems  */
/* most likely due to the rdpmc security "fix" */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */

static char test_string[]="Testing if rdpmc with pthreads works...";
static int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include "rdpmc_lib.h"

#include <sys/mman.h>

#include <pthread.h>

#define MAX_EVENTS 16


struct thread_params {
	int num;
	int repeats;
};

static void *our_thread(void *arg) {

	struct perf_event_attr pe;
	void *addr[MAX_EVENTS];
	int fd[MAX_EVENTS],ret1;
	int count=4;
	int i,result=0;
	long page_size=getpagesize();
	unsigned long long values[MAX_EVENTS],enabled[MAX_EVENTS],running[MAX_EVENTS];

	struct thread_params *p = (struct thread_params *)arg;

	/*****************************/
	/* TEST START/WORK/READ/STOP */
	/*****************************/

	/* Open event  */
	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	fd[0]=-1;

	for(i=0;i<count;i++) {
		pe.config=PERF_COUNT_HW_INSTRUCTIONS;

		if (i==0) {
			pe.disabled=1;
			pe.pinned=1;
		}
		else {
			pe.disabled=0;
			pe.pinned=0;
		}

		fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
		if (fd[i]<0) {
			fprintf(stderr,"Error opening event %d\n",i);
			test_fail(test_string);
		}
	}

	for(i=0;i<count;i++) {
		/* mmap() events */
		addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
		if (addr[i] == (void *)(-1)) {
			fprintf(stderr,"Error mmap()ing event %d!\n",i);
			test_fail(test_string);
		}
	}


	/* start */
	ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);
	if (ret1<0) fprintf(stderr,"ERROR IOCTL!\n");

	for(i=0;i<p->repeats;i++) {
		result=instructions_million();
	}

	if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");


	/* stop */
	ret1=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	/* read */
	for(i=0;i<count;i++) {
		values[i] = mmap_read_self(addr[i], &enabled[i], &running[i]);
	}


	/* unmap */
	for(i=1;i<count;i++) {
		if (addr[i]) {
			ret1=munmap(addr[i], page_size);
			if (ret1<0) fprintf(stderr,"ERROR MUNMAP!\n");
		}
	}

	/* close */
	for(i=1;i<count;i++) {
		close(fd[i]);
	}


	ret1=munmap(addr[0], page_size);
	if (ret1<0) fprintf(stderr,"ERROR MUNMAP!\n");
	close(fd[0]);

	/* Print results */
	if (!quiet) {
		for(i=0;i<count;i++) {
			printf("%d %d %lld\n",p->num,i,values[i]);
		}
	}

	return NULL;
}

#define NUM_THREADS 8

int main(int argc, char **argv) {

	int i;
	pthread_t threads[NUM_THREADS];
	struct thread_params p[NUM_THREADS+1];

	quiet=test_quiet();

	if (!quiet) {
		printf("This test checks if rdpmc works with pthreads\n\n");
	}

	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/* create 4 threads */

	for(i=0;i<NUM_THREADS;i++) {
		p[i].num=i;
		p[i].repeats=i*100;
		pthread_create( &threads[i], NULL, our_thread,
				(void *) &p[i] );
	}

	p[NUM_THREADS].num=99;
	p[NUM_THREADS].repeats=99*10;

	our_thread( (void *)&p[NUM_THREADS]);

	for(i=0;i<NUM_THREADS;i++) {
		pthread_join(threads[i],NULL);
	}

	test_pass(test_string);

	return 0;
}
