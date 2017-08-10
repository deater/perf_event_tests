/* This tests if setting reserved bits in offcore events is blocked */
/* This was broken on IVB/SNB until 3.9                             */
/* Fixed in commit f1923820c447e986a9d                              */

/* by Vince Weaver, vincent.weaver _at_ maine.edu                   */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "matrix_multiply.h"

char test_string[]="Testing if setting invalid offcore bits is blocked...";

int main(int argc, char **argv) {

	int ret,quiet,cpu;
	struct perf_event_attr pe;
	int fd1,fd2;
	long long counts[2];
	long long events[2];

	events[0]=0; events[1]=0;

	quiet=test_quiet();

	cpu=detect_processor();

	if (cpu==PROCESSOR_NEHALEM) {
		if (!quiet) printf("Found Nehalem!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:LOCAL_DRAM */
		/* libpfm4 reports 0x5301b7 0x4001 */
		events[0]=0x5301b7;
		events[1]=0x4001;
	}
	else if (cpu==PROCESSOR_NEHALEM_EX) {
		if (!quiet) printf("Found Nehalem EX!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:LOCAL_DRAM */
		/* libpfm4 reports 0x5301b7 0x4001 */
		events[0]=0x5301b7;
		events[1]=0x4001;
	}
	else if (cpu==PROCESSOR_WESTMERE) {
		if (!quiet) printf("Found Westmere!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:LOCAL_DRAM */
		/* libpfm4 reports 0x5301b7 0x4001 */
		events[0]=0x5301b7;
		events[1]=0x4001;
	}
	else if (cpu==PROCESSOR_WESTMERE_EX) {
		if (!quiet) printf("Found Westmere EX!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:LOCAL_DRAM */
		/* libpfm4 reports 0x5301b7 0x4001 */
		events[0]=0x5301b7;
		events[1]=0x4001;
	}
	else if (cpu==PROCESSOR_SANDYBRIDGE) {
		if (!quiet) printf("Found Sandybridge!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:ANY_RESPONSE */
		/* libpfm4 reports 0x5301b7 0x10001 */
		events[0]=0x5301b7;
		events[1]=0x10001;
	}
	else if (cpu==PROCESSOR_SANDYBRIDGE_EP) {
		if (!quiet) printf("Found Sandybridge EP!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:ANY_RESPONSE */
		/* libpfm4 reports 0x5301b7 0x10001 */
		events[0]=0x5301b7;
		events[1]=0x10001;
	}
	else if (cpu==PROCESSOR_IVYBRIDGE) {
		if (!quiet) printf("Found IvyBridge!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:ANY_RESPONSE */
		/* libpfm4 reports 0x5301b7 0x10001 */
		events[0]=0x5301b7;
		events[1]=0x10001;
	}
	else if (cpu==PROCESSOR_HASWELL) {
		if (!quiet) printf("Found Haswell!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:ANY_RESPONSE */
		/* libpfm4 reports 0x5301b7 0x10001 */
		events[0]=0x5301b7;
		events[1]=0x10001;
	}
	else if (cpu==PROCESSOR_BROADWELL) {
		if (!quiet) printf("Found Broadwell!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:ANY_RESPONSE */
		/* libpfm4 reports 0x5301b7 0x10001 */
		events[0]=0x5301b7;
		events[1]=0x10001;
	}
	else if (cpu==PROCESSOR_SKYLAKE) {
		if (!quiet) printf("Found Skylake!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:ANY_RESPONSE */
		/* libpfm4 reports 0x5301b7 0x10001 */
		events[0]=0x5301b7;
		events[1]=0x10001;
	}
	else if (cpu==PROCESSOR_KABYLAKE) {
		if (!quiet) printf("Found Kabylake!\n");
		/* try OFFCORE_RESPONSE_0:DMND_DATA_RD:ANY_RESPONSE */
		/* libpfm4 reports 0x5301b7 0x10001 */
		events[0]=0x5301b7;
		events[1]=0x10001;
	}
	else {
		if (!quiet) printf("No known offcore response hardware\n");
		test_skip(test_string);
	}

	/* Try to measure                */
	/* cpu-appropriate offcore event */
	/* as well as PAPI_TOT_INS       */

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		fprintf(stderr,"Error opening leader %llx\n",pe.config);
		test_fail(test_string);
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.size=sizeof(struct perf_event_attr);
	pe.type=PERF_TYPE_RAW;
	pe.config=events[0];
	pe.config1=events[1];
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd2=perf_event_open(&pe,0,-1,fd1,0);
	if (fd2<0) {
		fprintf(stderr,"Error opening %llx %llx %s\n",
			pe.config,pe.config1,strerror(errno));
		test_fail(test_string);
	}

	ioctl(fd1, PERF_EVENT_IOC_RESET, 0);
	ioctl(fd2, PERF_EVENT_IOC_RESET, 0);

	ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
	if (ret<0) {
		if (!quiet) fprintf(stderr,"Error enabling events\n");
		test_fail(test_string);
	}

	naive_matrix_multiply(quiet);

	ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
	if (ret<0) {
		if (!quiet) fprintf(stderr,"Error disabling events\n");
		test_fail(test_string);
	}

	if (!quiet) printf("Config1 after: %llx\n",pe.config1);

	ret=read(fd1,&counts[0],sizeof(long long));
	if (ret!=sizeof(long long)) {
		if (!quiet) fprintf(stderr,"Unepxected result from read fd1: %d\n",ret);
		test_fail(test_string);
	}

	ret=read(fd2,&counts[1],sizeof(long long));
	if (ret!=sizeof(long long)) {
		if (!quiet) fprintf(stderr,"Unepxected result from read fd2: %d\n",ret);
		test_fail(test_string);
	}

	if (!quiet) {
		printf("Retired instructions: %lld\n",counts[0]);
	}
	if (counts[0]<1) {
		fprintf(stderr,"Retired instruction count too low\n");
		test_fail(test_string);
	}

	if (!quiet) {
		printf("Offcore response count: %lld\n\n",counts[1]);
	}
	if (counts[1]<1) {
		if (!quiet) fprintf(stderr,"Offcore response count too low\n");
		test_fail(test_string);
	}

	/* Try to measure a value with invalid bits set */
	events[0]=0x5301b7;
	events[1]=0x3fffffffff;;

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.size=sizeof(struct perf_event_attr);
	pe.type=PERF_TYPE_RAW;
	pe.config=events[0];
	pe.config1=events[1];
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	fd1=perf_event_open(&pe,0,-1,-1,0);
	if (fd1<0) {
		if (!quiet) fprintf(stderr,"Expected error opening %llx %llx %s\n",
					pe.config,pe.config1,strerror(errno));
		test_pass(test_string);
		exit(0);
	}

	if (!quiet) {
		fprintf(stderr,"perf_event allowed us to open offcore event with invalid bits set\n");
		fprintf(stderr,"This was not fixed until Linux 3.9\n");
	}
	test_fail(test_string);

	return 0;
}

