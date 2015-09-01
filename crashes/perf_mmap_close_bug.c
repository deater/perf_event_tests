/* perf_mmap_close_bug.c */
/* By Vince Weaver <vincent.weaver at maine.edu>                             */
/* compile with "gcc -O2 -Wall -o perf_mmap_close_bug perf_mmap_close_bug.c  */
/* This will reliably OOPS my core2 Linux 3.10-rc2 machine                   */

/* The problem exists as far bacl as 3.2.  3.0 may or may not be affected */
/* The fix was merged by 3.10, 9bb5d40cd93c9dd4b                          */

#include <stdio.h>
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "perf_helpers.h"


int main(int argc, char **argv) {

	struct perf_event_attr pe1,pe2;
	int fd1,fd2;
	char *mmap_value,*mmap_value2;

	memset(&pe1,0,sizeof(struct perf_event_attr));
	memset(&pe2,0,sizeof(struct perf_event_attr));

	pe1.type=0;
	pe1.size=0x60;
	pe1.config=3;
	pe1.sample_type=0x4f0;
	pe1.read_format=5;
	pe1.exclude_kernel=1;
	pe1.bp_type=1;
	pe1.config1=0x1d469257;
	pe1.config2=2;

	fd1=perf_event_open(&pe1,0,0,-1,3);
	if (fd1<0) {
		fprintf(stderr,"Error opening fd1 %s\n",strerror(errno));
		exit(1);
	}

	mmap_value=mmap(NULL, 69632, PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);
	printf("mmap1=%p\n",mmap_value);
	if (mmap_value==MAP_FAILED) {
		printf("MMAP failed: %s\n",strerror(errno));
	}

	ioctl(fd1,PERF_EVENT_IOC_RESET,0);

	pe2.type=1;
	pe2.size=0x60;
	pe2.config=2;
	pe2.read_format=3;
	pe2.exclusive=1;
	pe2.exclude_user=1;
	pe2.mmap=1;
	pe2.inherit_stat=1;
	pe2.enable_on_exec=1;
	pe2.task=1;
	pe2.watermark=1;
	pe2.precise_ip=2;
	pe2.sample_id_all=1;
	pe2.exclude_guest=1;
	pe2.wakeup_events=1500395763;

	fd2=perf_event_open(&pe2,0,0,fd1,3);

	fd1=perf_event_open(&pe1,0,0,-1,3);
	if (fd2<0) {
		fprintf(stderr,"Error opening fd2 %s\n",strerror(errno));
		exit(1);
	}

	mmap_value2=mmap(NULL, 69632, PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
	printf("mmap2=%p\n",mmap_value2);

	ioctl(fd1,PERF_EVENT_IOC_ENABLE,0);

	ioctl(fd2,PERF_EVENT_IOC_ENABLE,0);

   	close(fd2);

	return 0;
}
