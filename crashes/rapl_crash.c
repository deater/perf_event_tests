/* rapl_crash.c -- bug found with perf_fuzzer     */
/* by Vince Weaver <vincent.weaver _at_ maine.edu */

/* This was introduced by a misplaced parenthesis in	*/
/*  89cbc76768c2fa4ed95545bf961f3a14ddfeed21  (3.18)	*/

/* Is fixed in 3.19					*/
/*  98b008dff8452653909d9263efda925873e8d8bb		*/


/* You need Intel RAPL support and 			*/
/*   /proc/sys/kernel/perf_event_paranoid set to 0	*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syscall.h>
#include <linux/perf_event.h>

#include "perf_helpers.h"

int main(int argc, char **argv) {

	int fd;
	static struct perf_event_attr pe;

	/* Random Seed was 1421689769 */
	/* /proc/sys/kernel/perf_event_max_sample_rate was 100000 */

	memset(&pe,0,sizeof(struct perf_event_attr));
	pe.type=6;
	pe.config=0x2ULL;
	pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_ID; /* 5 */
	pe.pinned=1;
	pe.config1=0x39ULL;

	fd=perf_event_open(&pe,
				-1, /* all processes */
				5, /* Only cpu 5 */
				-1, /* no group leader */
				PERF_FLAG_FD_NO_GROUP /*1*/ );

	close(fd);

	return 0;
}
