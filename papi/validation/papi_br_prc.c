/* This file attempts to test the predicted correctly branches */
/* performance counter on various architectures, as            */
/* implemented by the PAPI_BR_PRC counter.                     */

/* by Vince Weaver, vweaver1@eecs.utk.edu                      */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"
#include "branches_testcode.h"

int main(int argc, char **argv) {

   int retval,quiet,result;

   int num_runs=100;
   long long high=0,low=0,average=0,expected=1000000;
   double error;
   int num_random_branches=500000;

   int i;
   int events[1];
   long long counts[1],total=0;

   char test_string[]="Testing PAPI_BR_PRC predefined event...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("Error: PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_BR_PRC);
   if (retval != PAPI_OK) {
      if (!quiet) printf("PAPI_BR_PRC not supported %d\n", retval);
      test_skip(test_string);
   }

   if (!quiet) {
      printf("\n");
      printf("Testing a simple loop with %lld branches (%d times):\n",
          expected,num_runs);
      printf("Nearly all the branches should be predicted correctly.\n");
   }

   events[0]=PAPI_BR_PRC;
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

   /*******************/

   high=0; low=0; total=0;

   events[0]=PAPI_BR_CN;

   for(i=0;i<num_runs;i++) {

     PAPI_start_counters(events,1);

     result=random_branches_testcode(num_random_branches,1);

     PAPI_stop_counters(counts,1);

     if (counts[0]>high) high=counts[0];
     if ((low==0) || (counts[0]<low)) low=counts[0];
     total+=counts[0];
   }

   average=total/num_runs;

   expected=average;

   if (!quiet) {
      printf("\nTesting a function that branches based on a random number\n");
      printf("   The loop has %lld conditional branches.\n",expected);
      printf("   %d are random branches; %d of those were taken\n",num_random_branches,result);
   }

   high=0; low=0; total=0;

   events[0]=PAPI_BR_PRC;

   for(i=0;i<num_runs;i++) {

     PAPI_start_counters(events,1);

     result=random_branches_testcode(num_random_branches,1);

     PAPI_stop_counters(counts,1);

     if (counts[0]>high) high=counts[0];
     if ((low==0) || (counts[0]<low)) low=counts[0];
     total+=counts[0];
   }

   average=total/num_runs;

   if (!quiet) {

      printf("\nOut of %lld branches, %lld predicted correctly\n",expected,average);
      printf("Assuming a good random number generator and no freaky luck\n");
      printf("The TOTAL - CORRECT value is %lld\n",expected-average);
      printf("This value should be roughly between %d and %d\n",
             num_random_branches/4,(num_random_branches/4)*3);
   }

   if ( (expected-average) < (num_random_branches/4)) {
     if (!quiet) printf("Correct predicts too low\n");
     test_fail(test_string);
   }

   if ( (expected-average) > (num_random_branches/4)*3) { 

     if (!quiet) printf("Correct predicts too high\n");
     test_fail(test_string);
   }
   if (!quiet) printf("\n");

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
