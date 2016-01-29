/* This program measures the relative overhead of PAPI	*/
/* vs raw perf_event					*/

/* by Vince Weaver, vincent.weaver _at_ maine.edu	*/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//#include "papiStdEventDefs.h"
//#include "papi.h"

#include "instructions_testcode.h"

#define NUM_RUNS 100000



   /* Test a simple loop of 1 million instructions             */
   /* Most implementations should count be correct within 1%   */
   /* This loop in in assembly language, as compiler generated */
   /* code varies too much.                                    */





int main(int argc, char **argv) {

	int retval;

#if 0
	retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT) {
		if (!quiet) printf("Error! PAPI_library_init %d\n", retval);
	}

	retval = PAPI_query_event(PAPI_TOT_INS);
	if (retval != PAPI_OK) {
		if (!quiet) printf("PAPI_TOT_INS not supported\n");
		test_skip(test_string);
	}
#endif

	int i;
	int events[1],result;
	long long counts[1],high=0,low=0,total=0,average=0;
	double error;

#if 0
   events[0]=PAPI_TOT_INS;

   if (!quiet) printf("Testing a loop of 1 million instructions (%d times):\n",
          NUM_RUNS);
#endif

   for(i=0;i<NUM_RUNS;i++) {

//      PAPI_start_counters(events,1);

      result=instructions_million();
     
//      PAPI_stop_counters(counts,1);
#if 0
      if (result==CODE_UNIMPLEMENTED) {
	if (!quiet) fprintf(stderr,"\tCode unimplemented\n");
         test_fail(test_string); 
      }

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;

   error=display_error(average,high,low,1000000ULL,quiet);

   if ((error > 1.0) || (error<-1.0)) {


      if (!quiet) printf("Instruction count off by more than 1%%\n");
      test_fail(test_string);
   }

#endif

	}

#if 0
	PAPI_shutdown();
#endif

	return 0;
}
