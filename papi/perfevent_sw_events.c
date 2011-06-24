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
   int EventSet=PAPI_NULL;
   char test_string[]="Testing perf_event software events...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("ERROR: PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }

   /* On some kernels this seems to be user only, on some kernel only? */
   PAPI_set_domain(PAPI_DOM_KERNEL | PAPI_DOM_USER);
   
   if (PAPI_create_eventset(&EventSet)!=PAPI_OK) {
      if (!quiet) printf("PAPI_create_eventset %d\n",retval);
      test_fail(test_string);
   }
  
   if (PAPI_event_name_to_code("perf::PERF_COUNT_SW_CONTEXT_SWITCHES",&events[0])!=PAPI_OK) {
      if (!quiet) printf("Event not found perf::PERF_COUNT_SW_CONTEXT_SWITCHES: %d\n",retval);
      test_skip(test_string);
   }
   
   if (!quiet) printf("Adding %x\n",events[0]);
	  
   if (PAPI_add_event(EventSet,events[0])!=PAPI_OK) {
      if (!quiet) printf("Error adding event %d\n",retval);
      test_fail(test_string);
   }

   if (PAPI_add_event(EventSet,PAPI_TOT_INS)!=PAPI_OK) {
      if (!quiet) printf("Error adding event %d\n",retval);
      test_fail(test_string);
   }

   if (!quiet) printf("Measuring %x %x\n",events[0],events[1]);
   
   PAPI_start(EventSet);

   naive_matrix_multiply(quiet);

   PAPI_stop(EventSet,counts);

   if (!quiet) {
      printf("   perf::PERF_COUNT_SW_CONTEXT_SWITCHES %lld\n",counts[0]);
      printf("   PAPI_TOT_INS: %lld\n",counts[1]);
   }

   if (counts[0]==0) {
      if (!quiet) {
	 printf("Context switches was 0. This probably shouldn't\n");
         printf("Unless you have a very fast processor.\n");
      }
      test_fail(test_string);
   }
   
   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
