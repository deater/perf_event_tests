/* This file attempts to test the retired FP          */
/* performance counter on various architectures, as   */
/* implemented by the PAPI_FP_INS counter.            */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"
#include "matrix_multiply.h"

#define NUM_RUNS 3

int main(int argc, char **argv) {

   int retval,quiet;

   int i;

   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   double error;
   long long expected;

   char test_string[]="Testing PAPI_FP_INS predefined event...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("Error: PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_FP_INS);
   if (retval != PAPI_OK) {
      if (!quiet) printf("PAPI_FP_INS not supported");
      test_skip(test_string);
   }

   events[0]=PAPI_FP_INS;

   if (!quiet) {
      printf("\n");
      printf("Testing a sleep of 1 second (%d times):\n", NUM_RUNS);
      printf("Result should be close to 0.\n");
   }


   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

      sleep(1);
      PAPI_stop_counters(counts,1);

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;
   if (!quiet) {
      printf("Counted an average of %lld FP_INS during sleep\n\n",
              average);
   }

   if (average > 500) {
      if (!quiet) printf("More PAPI_FP_INS than expected\n");
      test_fail(test_string);
   }


   /* busy spin */

   high=0; low=0; total=0;

   if (!quiet) {
      printf("\n");
      printf("Testing a busy loop (%d times):\n",NUM_RUNS);
   }

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

      naive_matrix_multiply(quiet);

      PAPI_stop_counters(counts,1);

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];

   }

   average=total/NUM_RUNS;

   expected=naive_matrix_multiply_estimated_flops(quiet);

   error=display_error(average,high,low,expected,quiet);

   if (error > 1.0) {
      if (!quiet) printf("FP error higher than expected\n");
      test_fail(test_string);
   }

   if (average < 1000 ) {
      if (!quiet) printf("Floating Point count too small.\n");
      test_fail(test_string);
   }


   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
