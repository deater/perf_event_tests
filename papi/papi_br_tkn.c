/* This file attempts to test the retired branches taken       */
/* performance counter on various architectures, as            */
/* implemented by the PAPI_BR_TKN counter.                     */

/* by Vince Weaver, vweaver1@eecs.utk.edu                      */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"
#include "branches_testcode.h"

int main(int argc, char **argv) {

   int retval,quiet;

   int num_runs=100;
   long long high=0,low=0,average=0,expected=1000000;
   double error;

   int i,result;
   int events[1];
   long long counts[1],total=0;

   char test_string[]="Testing PAPI_BR_TKN predefined event...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      printf("Error: PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_BR_TKN);
   if (retval != PAPI_OK) {
      printf("PAPI_BR_TKN not supported\n");
      test_skip(test_string);
   }

   if (!quiet) {
      printf("\n");

      printf("Testing a loop with %lld taken branches (%d times):\n",
          expected,num_runs);
   }

   events[0]=PAPI_BR_TKN;
   high=0;
   low=0;

   for(i=0;i<num_runs;i++) {

     PAPI_start_counters(events,1);

     result=branches_testcode();

     PAPI_stop_counters(counts,1);

     if (result==CODE_UNIMPLEMENTED) {
        if (!quiet) printf("\tNo test code for this architecture\n");
        test_skip(test_string);
     }

     if (counts[0]>high) high=counts[0];
     if ((low==0) || (counts[0]<low)) low=counts[0];
     total+=counts[0];
   }

   average=total/num_runs;

   error=display_error(average,high,low,expected,quiet);

   if ((error > 1.0) || (error<-1.0)) {
      if (!quiet) printf("Instruction count off by more than 1%%\n");
      test_fail(test_string);
   }
   if (!quiet) printf("\n");

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
