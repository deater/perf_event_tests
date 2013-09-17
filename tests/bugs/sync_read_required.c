/* sync_read_required.c                                  */
/* Based on a test by Corey Ashford from here:           */
/*   https://bugzilla.kernel.org/show_bug.cgi?id=14489   */

/* If you create an event, make it the group leader, and set */
/* the PERF_FORMAT_GROUP flag, then enable the event, let it */
/* count while giving the CPU some work to do, then read the */
/* event counter back via read(), the TOTAL_TIME_ENABLED and */
/* TOTAL_TIME_RUNNING fields will be zero unless the counter */
/* if first disabled via ioctl(fd, PERF_IOC_EVENT_DISABLE,)  */

/*   This was probably fixed in commit          */
/*     abf4868b8548cae18d4fe8bbfb4e207443be01be */
/*     perf: Fix PERF_FORMAT_GROUP scale info   */
/*   which was in 2.6.33                        */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "matrix_multiply.h"

int main(int argc, char **argv)
{
	struct perf_event_attr pe;
	int fd,ret,quiet,i;

	uint64_t buffer[5];
	int cnt;

	char test_string[]="Testing if time running is correct without DISABLE...";

	quiet=test_quiet();

	if (!quiet) {
	  printf("This test checks if TOTAL_TIME_RUNNING and TOTAL_TIME_ENABLED\n");
	  printf("  Return proper results without the counter being disabled\n");
	  printf("  with PERF_EVENT_IOC_DISABLE first.  This was fixed in\n");
	  printf("  Linux 2.6.33\n");	
	}

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID
			| PERF_FORMAT_TOTAL_TIME_RUNNING
			| PERF_FORMAT_TOTAL_TIME_ENABLED;
	pe.disabled = 1;
	pe.type = PERF_TYPE_HARDWARE;
	pe.config = PERF_COUNT_HW_CPU_CYCLES;

	arch_adjust_domain(&pe,quiet);

	/* open group leader */
	fd = perf_event_open(&pe, 0, -1, -1, 0);
	if (fd<0) {
	   if (!quiet) fprintf(stderr,"perf_event_open failed: %s\n", 
			       strerror(errno));
	   test_fail(test_string);
	}

	ret = ioctl(fd, PERF_EVENT_IOC_ENABLE, NULL);
	if (ret<0) {
	   if (!quiet) fprintf(stderr,"enable failed: %s\n", strerror(errno));
	   test_fail(test_string);
	}

	naive_matrix_multiply(quiet);

#ifdef DO_DISABLE
	ret = ioctl(fd, PERF_EVENT_IOC_DISABLE, NULL);
	if (ret<0) {
	   if (!quiet) fprintf(stderr,"disable failed: %s\n", strerror(errno));
	   test_fail(test_string);
	}
#endif

	cnt = read(fd, buffer, sizeof(buffer));
	if (cnt == -1) {
	   fprintf(stderr,"read failed: %s\n", strerror(errno));
	   test_fail(test_string);
	}

	if (!quiet) {
	   for (i = 0; i < 5; i++) {
		printf("[%d] = %"PRIu64"\n", i, buffer[i]);
	   }
	}

#define TIME_ENABLED_IDX 1
#define TIME_RUNNING_IDX 2

	if (!buffer[TIME_ENABLED_IDX] || !buffer[TIME_RUNNING_IDX]) {
	   if (!quiet) fprintf(stderr,"test failed!\n");
	   test_fail(test_string);
	}
	
	test_pass(test_string);
	return 0;
}
