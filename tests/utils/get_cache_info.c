#include <stdio.h>

#include "test_utils.h"
#include "detect_cache.h"

int main(int argc, char **argv) {

	int quiet;
	int i,j,max_level=0;
	int result=0;
	char test_string[]="Seeing if cache info is provided by the kernel...";

	quiet=test_quiet();

	result=gather_cache_info(quiet,test_string);
	if (result<0) {
		test_skip(test_string);
	}

	max_level=cache_get_max_levels(quiet,test_string);
	if (!quiet) {
		printf("Max level of cache=%d\n",max_level);
	}

	for(i=1;i<=max_level;i++) {
		for(j=0;j<MAX_CACHE_TYPE;j++) {
			print_cache_info(quiet,&cache_info[i][j]);
		}
	}

	test_pass(test_string);

	return 0;
}
