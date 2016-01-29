/* This program measures the relative overhead of PAPI	*/
/* vs raw perf_event					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "instructions_testcode.h"

#define NUM_RUNS 100000



int main(int argc, char **argv) {

	int i;
	int result;

	for(i=0;i<NUM_RUNS;i++) {

		result=instructions_million();
	}

	(void)result;

	return 0;
}
