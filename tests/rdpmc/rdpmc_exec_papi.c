/* This attempts to use rdpmc after we've execed */

/* For "security" reasons this was made difficult under Linux	*/

/* Chasing down a Linux bug showed by the PAPI tests		*/
/* where if we exec without closing the mmap() then the		*/
/* reference count can go negative and rdpmc() accesses GPF	*/

/* This was fixed by bfe334924ccd9f4a53f30240c03cf2f43f5b2df1 (Linux 4.14) */

/* by Vince Weaver, vincent.weaver _at_ maine.edu       */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"

#include "rdpmc_lib.h"

#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <signal.h>

static char test_string[]="Testing if we can rdpmc in execed process (PAPI) ...";
static int quiet=0;


static void segv_handler(int sig, siginfo_t *si, void *unused) {
	if (!quiet) printf("Got SIGSEGV\n");

	if (check_linux_version_newer(4,14,0)) {
		test_fail(test_string);
	}
	else {
		test_known_kernel_bug(test_string);
	}
}







int main(int argc, char **argv) {

	long page_size=getpagesize();
	void *addr;
	struct perf_event_attr pe;
	int fd,ret1,ret2;
	int in_child=0,result;
	char current_dir[BUFSIZ];
	char exe_path[BUFSIZ];

	unsigned long long values,enabled,running;

	struct sigaction sa;

	quiet=test_quiet();

	/* Setup SIGSEGV handler */
	/* as failure case when rdpmc() is denied is a GPF/segfault */
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segv_handler;
	if (sigaction(SIGSEGV, &sa, NULL) == -1) {
		if (!quiet) printf("Failure setting up SEGV handler\n");
		test_fail(test_string);
	}

	if (argc>1) in_child=1;

	getcwd(current_dir,BUFSIZ);

	if (argv[0][0]=='/') {
		snprintf(exe_path,BUFSIZ,"%s",argv[0]);
	}
	else {
		snprintf(exe_path,BUFSIZ,"%s/%s",current_dir,argv[0]);
	}

	if (!quiet) {
		if (!in_child) {
			printf("This test checks if rdpmc events work in exec()ed process (PAPI).\n\n");
			printf("+ Before exec\n");
		}
		else {
			printf("+ After exec\n");
		}
	}


	/* See if we support rdpmc access */
	if (!detect_rdpmc(quiet)) {
		test_skip(test_string);
	}

	/* Open event */
	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);

	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.pinned=1;

	fd=perf_event_open(&pe, 0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening event\n");
		test_fail(test_string);
	}

	/* mmap() event */
	addr=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd,0);
	if (addr == MAP_FAILED) {
		fprintf(stderr,"Error mmap()ing event\n");
		test_fail(test_string);
	}


	/* start */

	ret1=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

	/* read */
	values = mmap_read_self(addr, &enabled, &running);


	/* stop */
	ret2=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);

	if (ret1<0) {
		fprintf(stderr,"Error starting!\n");
		test_fail(test_string);
	}

	if (ret2<0) {
		fprintf(stderr,"Error stopping!\n");
		test_fail(test_string);
	}

	if (values<0) {
		if (!quiet) printf("rdpmc support not available.\n");
		test_yellow_no(test_string);
	}


	/* The PAPI issue is if we don't close before execing */

//	close(fd);
//	munmap(addr,page_size);

	if (!quiet) {
		printf("\t* RDPMC 1 Event %x -- count: %lld enabled %llx running: %llx\n",
				0,values,enabled,running);
	}

	if (enabled!=running) {
		if (!quiet) printf("enabled doesn't match running!\n");
		test_fail(test_string);
	}

	if (!in_child) {
		result=execl( exe_path, exe_path ,"1",NULL);
		if (result<0) {
			if (!quiet) printf("Failed exec!\n");
			test_fail(test_string);
		}
	}

	test_pass(test_string);

	return 0;
}
