/* openmp_sample -- Test how sampling works on openmp programs		*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu			*/


/* for performance reasons the kernel does not let you have inherited   */
/* events share one single mmap buffer (too much cross-CPU mem traffic) */
/* So instead you need to create one mmap buffer per CPU		*/

/* Note, the samples are not distributed evenly */
/* Still need to track down why */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>

#include <omp.h>

#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <signal.h>
#include <fcntl.h>

#include <sys/mman.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "perf_barrier.h"
#include "instructions_testcode.h"
#include "parse_record.h"

#include <syscall.h>

int gettid(void) {
	return syscall( SYS_gettid );
}

static char test_string[]="Testing OpenMP sampling results...";
static int quiet=0;

#define MAX_CPUS	1024

struct mmap_info_t {
	int fd;
	int count;
	char *mmap;
	long long prev_head;
} mmap_info[MAX_CPUS];

static int fds[MAX_CPUS];

static int num_cpus=0;

static long long sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_TID;
static long long sample_regs_user=0;

#define MMAP_PAGES 3

static void our_handler(int signum, siginfo_t *info, void *uc) {

        int ret,i;

	int fd = info->si_fd;

	for(i=0;i<num_cpus;i++) {
		if (mmap_info[i].fd==fd) break;
	}

	/*  Disable will disable count for *all* threads */

	mmap_info[i].prev_head=perf_mmap_read(mmap_info[i].mmap,
						MMAP_PAGES-1,
						mmap_info[i].prev_head,
						sample_type,0,
						sample_regs_user,
						NULL,quiet,NULL, RAW_NONE);


//        ret=ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

	mmap_info[i].count++;

	/* why not refresh? */
//	ret=ioctl(fd, PERF_EVENT_IOC_ENABLE, 1);

	(void) ret;

}



int main (int argc, char **argv) {

	int nthreads, tid, result;
	struct perf_event_attr pe;
	struct sigaction sa;
	int i, errors=0,warnings=0;
	int total=0;

	quiet=test_quiet();

	if (!quiet) {
		printf("Testing OpenMP sample behavior\n");
	}


	/****************************************/
	/* Inherit				*/
	/****************************************/

	if (!quiet) {
		printf("Note!!! Inherit only works if the event is created\n");
		printf("before the fork happens!  OpenMP creates thread pools\n");
		printf("so if you create the event after the thread pools hae been\n");
		printf("created it may be too late and you won't get child events!\n\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	pe.inherit=1;
//	pe.wakeup_events=1;
	pe.sample_period=1000000;
	pe.sample_type=sample_type;
//	pe.freq=1;
//	pe.sample_freq=4000;

	arch_adjust_domain(&pe,quiet);

	for(i=0;i<MAX_CPUS;i++) {
		fds[i]=perf_event_open(&pe,0,i,-1,0);
		if (fds[i]<0) {
			num_cpus=i;
			break;
		}
	}

	if (!quiet) printf("NUM CPUS=%d\n",num_cpus);

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sa, NULL) < 0) {
		fprintf(stderr,"Error setting up signal handler\n");
		exit(1);
	}

	for(i=0;i<num_cpus;i++) {
		fcntl(fds[i], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(fds[i], F_SETSIG, SIGIO);

		struct f_owner_ex {
			int   type;
			pid_t pid;
		} owner;

	//	owner.type=F_OWNER_PGRP;
		owner.type=F_OWNER_PID;
		owner.pid=getpid();
		fcntl(fds[i], F_SETOWN_EX,&owner);
	}

	for(i=0;i<num_cpus;i++) {
		mmap_info[i].count=0;
		mmap_info[i].fd=fds[i];
		mmap_info[i].mmap=mmap(NULL, MMAP_PAGES*4096,
			PROT_READ|PROT_WRITE, MAP_SHARED, fds[i], 0);

		if (mmap_info[i].mmap==MAP_FAILED) {
			printf("MMAP FAILED!\n");
			test_fail(test_string);
		}
	}



	for(i=0;i<num_cpus;i++) {
		ioctl(fds[i], PERF_EVENT_IOC_ENABLE,0);
	}

	/* Start a parallel group of threads */
#pragma omp parallel private(nthreads, tid, i)
{

	/* Obtain thread number */
	tid = omp_get_thread_num();

	/* Only master thread does this */
	if (tid == 0) {
		nthreads = omp_get_num_threads();
		if (!quiet) {
			printf("Running with %d threads\n", nthreads);
		}
	}

	if (!quiet) {
		printf("\t+ Running 10 million instructions in thread %d %d\n",
			tid,gettid());
	}

	for(i=0;i<10;i++) {
		result=instructions_million();
	}


}
	/* All threads join master thread and disband */

	for(i=0;i<num_cpus;i++) {
		ioctl(fds[i], PERF_EVENT_IOC_DISABLE,0);
	}

	for(i=0;i<num_cpus;i++) {
		close(fds[i]);
	}

	if (result==CODE_UNIMPLEMENTED) {
		test_skip(test_string);
	}


	if (!quiet) {
		for(i=0;i<num_cpus;i++) {
			printf("\tCPU=%d count=%d\n",
				i,mmap_info[i].count);
		}
	}


	for(i=0;i<num_cpus;i++) {
		if (mmap_info[i].count < 9 ) {
			if (!quiet) printf("WARNING! CPU%d overflow count low!\n",i);
			warnings++;
		}
		total+=mmap_info[i].count;
	}

	if (!quiet) {
		printf("Total overflows: %d\n",total);
	}

	if (total<90) {
		if (!quiet) printf("Unexpected low number of overflows!\n");
		errors++;
	}


	if (errors) {
		test_unexplained(test_string);
	}

	if (warnings) {
		test_unexplained(test_string);
	}

	test_pass(test_string);


	return 0;
}

