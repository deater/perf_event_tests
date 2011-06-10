/* This tests perf_event software events              */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"
#include "matrix_multiply.h"


int main(int argc, char **argv) {
   
   int retval,quiet;
   int events[2];
   long long counts[2];
   double error;
   char test_string[]="Testing perf_event generalized instruction event...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("ERROR: PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }
   
   retval=PAPI_event_name_to_code("perf::PERF_COUNT_HW_INSTRUCTIONS",&events[0]);
   if (retval!=PAPI_OK) {
      if (!quiet) printf("ERROR PAPI_event_name_to_code: %d\n", retval);      
      test_skip(test_string);
   }

   events[1]=PAPI_TOT_INS;

   if (!quiet) printf("Measuring %x %x\n",events[0],events[1]);
   
   PAPI_start_counters(events,2);

   naive_matrix_multiply(quiet);

   PAPI_stop_counters(counts,2);

   if (!quiet) {
      printf("   perf::PERF_COUNT_HW_INSTRUCTIONS %lld\n",counts[0]);
      printf("   PAPI_TOT_INS: %lld\n",counts[1]);
   }

   error=display_error(counts[0],counts[0],counts[0],counts[1],quiet);

   if ((error>1.0) || (error < -1.0)) {

     if (!quiet) printf("ERROR: PAPI_TOT_INS perf::PERF_COUNT_HW_INSTRUCTIONS mismatch!\n");
     test_fail(test_string);
   }

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
