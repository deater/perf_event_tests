#include <stdio.h>

#include "test_utils.h"

int main(int argc, char **argv) {

	int quiet;
	int result=0;
	char test_string[]="Checking /proc/sys/kernel/perf_event_paranoid setting...";

	quiet=test_quiet();

	result=check_paranoid_setting(2,quiet);
	if (result<0) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
