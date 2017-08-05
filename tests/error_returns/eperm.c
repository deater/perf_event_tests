/* eperm.c */
/* Tests to see if EPERM properly returned by perf_event_open() */

/* By Vince Weaver vincent.weaver _at_ maine.edu */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <errno.h>

#include "perf_event.h"
#include "hw_breakpoint.h"

#include "perf_helpers.h"
#include "test_utils.h"

int main(int argc, char **argv) {

	int fd,arch;
	int quiet,errors=0;
	struct perf_event_attr attr;

	char test_string[]="Testing EPERM generation...";

	quiet=test_quiet();

	if (!quiet) {
		printf("This tries to get perf_event_open() to generate "
		"an EPERM errno.\n\n");
	}

	if (!quiet) {
		printf("+ Attempting to set a breakpoint on a kernel address\n");
	}

        memset(&attr,0,sizeof(struct perf_event_attr));
	attr.size=sizeof(struct perf_event_attr);
	attr.type=PERF_TYPE_BREAKPOINT;
	attr.bp_type=HW_BREAKPOINT_X;

	/* If exclude_kernel=1 then we get EINVAL */
	attr.exclude_kernel=0;

	/* FIXME: these are going to be different on different archs? */

	arch=detect_architecture();

	if ((arch==ARCH_X86) || (arch==ARCH_X86_64)) {
		if (!quiet) {
			printf("+ Trying x86 values\n");
		}
		if (sizeof(long)==8) {
			attr.bp_addr=(unsigned long long)0xffffffff8148c9d5;
		}
		else {
			attr.bp_addr=(unsigned long)0xc148c9d5;

		}
	}
	else if (arch==ARCH_ARM64) {
		if (!quiet) {
			printf("+ Trying ARM64 values\n");
		}
		if (sizeof(long)==8) {
			attr.bp_addr=(unsigned long long)0xffffffc00015d4b8;
		}
		else {
			attr.bp_addr=(unsigned long)0xc148c9d5;

		}
	} else {
		if (!quiet) {
			printf("+ Unknown arch\n");
		}
		if (sizeof(long)==8) {
			attr.bp_addr=(unsigned long long)0xffffffc00015d4b8;
		}
		else {
			attr.bp_addr=(unsigned long)0xc148c9d5;

		}
	}

	attr.bp_len=sizeof(long);

	if (!quiet) {
		printf("\tTrying to set breakpoint at 0x%llx\n",attr.bp_addr);
	}

	if (arch==ARCH_ARM64) {
		/* ARM64 returns EINVAL if you try a per-task */
		/* kernel breakpoint */
		fd=perf_event_open(&attr,
				-1, /* any thread */
				0, /* cpu 0 */
				-1, /* New Group Leader */
				0 /*0*/ );
	}
	else {
		fd=perf_event_open(&attr,
				0, /* current thread */
				-1, /* any cpu */
				-1, /* New Group Leader */
				0 /*0*/ );
	}

	if (fd<0) {
		if (errno==EPERM) {
			if (!quiet) {
				printf("\tProperly triggered EPERM\n");
			}
		}
		else {
			if (!quiet) {
				printf("Unexpectedly got: %s\n",strerror(errno));
			}
			errors++;
		}
	}
	else {
		if (getuid()==0) {
			if (!quiet) {
				printf("\tWas able to set kernel breakpoint because you are running as root\n");
			}
		}
		else {
			if (!quiet) {
				printf("\tUnexpectedly was able to set kernel breakpoint\n");
				errors++;
			}
		}
		close(fd);
	}

	if (errors) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
