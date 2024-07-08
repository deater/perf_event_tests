/* This file attempts to test the number of hardware */
/* interrupts, as provided by PAPI_HW_INT            */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu       */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"
#include "instructions_testcode.h"

#define NUM_RUNS 10000

int main(int argc, char **argv) {

   int retval,quiet,result;
   int i,events[1];
   long long counts[1];


   char test_string[]="Testing PAPI_HW_INT predefined event...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("Error: PAPI_library_init: %d\n", retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_HW_INT);
   if (retval != PAPI_OK) {
      if (!quiet) printf("PAPI_HW_INT not supported");
      test_skip(test_string);
   }

   events[0]=PAPI_HW_INT;

   if (!quiet) {
      printf("\n");
      printf("Testing a loop of 1 million instructions (%d times):\n",
          NUM_RUNS);
      printf("A certain number of interrupts should happen (mostly timer)\n");
   }

   PAPI_start_counters(events,1);


   for(i=0;i<NUM_RUNS;i++) {
      result=instructions_million();
   }

   PAPI_stop_counters(counts,1);

   if (result==CODE_UNIMPLEMENTED) {
      fprintf(stderr,"\tCode unimplemented\n");
      test_fail(test_string);
   }

   if (!quiet) {
      printf("   Expected: >0\n");
      printf("   Obtained: %lld\n",counts[0]);
      printf("\n");
   }

   if (counts[0] == 0) {
      if (!quiet) printf("Error: Interrupt count was zero\n");
      test_fail(test_string);
   }

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
