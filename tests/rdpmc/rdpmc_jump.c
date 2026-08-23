char test_string[]="Testing if hardware counters exhibit unexpected jumps with rdpmc";
int quiet=0;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#include "rdpmc_lib.h"

#include <sys/mman.h>


int main(/* int argc, char **argv */) {
	int quiet = test_quiet();
	int silent = quiet;

	if (!quiet) printf("%s.\n", test_string);

	if (!detect_rdpmc(quiet)) test_skip(test_string);

	if (instructions_million() == CODE_UNIMPLEMENTED) {
		if (! silent) fprintf(stderr, "instructions_million() not implemented\n");
		test_skip(test_string);
	}

	struct perf_event_attr attr = {
		.type=PERF_TYPE_HARDWARE,
		.size=sizeof(struct perf_event_attr),
		.config=PERF_COUNT_HW_INSTRUCTIONS,
		.pinned = 1,
	};

	int fd = perf_event_open(&attr, 0, -1, -1, 0);
	if (fd < 0) {				
		if (! silent) perror("perf_event_open() failed");
		test_fail(test_string);
	}
	
	char *map = mmap(NULL, getpagesize(), PROT_READ, MAP_SHARED, fd, 0);
	if (map == (void *)(-1)) {
		if (! silent) perror("mmap() failed");
		test_fail(test_string);
	}

	int repeat =  100000;
	if (! quiet) printf("Trying up to %d iterations ", repeat);
	int allowed = 10000000;
	if (! quiet) printf("(%d allowed per iteration)\n", allowed);

	unsigned long long max_count = 0;
	for(int i = 0; i < repeat; i++) {
		ioctl(fd, PERF_EVENT_IOC_RESET, 0);	
		instructions_million();
		unsigned long long count = mmap_read_self(map, NULL, NULL);
		if (count > allowed) {
			if (! silent) {
				fprintf(stderr, "Got spurious count of %lld "
					"in iteration %d\n", count, i);
				fprintf(stderr, "Max count for previous "
					"iterations was %lld\n", max_count);
			}
			test_fail(test_string);
		}
		if (count > max_count) max_count = count;
	}

	close(fd);
	munmap(map, getpagesize());

	if (! quiet) printf("Done. Max count was %lld\n", max_count);
	test_pass(test_string);

	return 0;
}
