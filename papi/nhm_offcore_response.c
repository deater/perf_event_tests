/* This tests Nehalem+ Offcore Response Counters in PAPI */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu           */


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
   const PAPI_hw_info_t *info;

   int events[2];
   long long counts[2];
   double error;
   long long expected;

   char test_string[]="Testing if offcore events are supported...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("ERROR: PAPI_library_init %d\n",retval);
      test_fail(test_string);
   }

   if ( (info=PAPI_get_hardware_info())==NULL) {
      if (!quiet) printf("cannot obtain hardware info: %d\n",retval);
      test_fail(test_string);
   }

   if ((info->vendor==PAPI_VENDOR_INTEL) && (info->cpuid_family==6) && 
			((info->cpuid_model==30) || (info->cpuid_model==46) || 
			 (info->cpuid_model==30))) {

     if (!quiet) printf("Found Nehalem!\n");
   }
   else {
     if (!quiet) printf("Not a Nehalem.\n");
     test_skip(test_string);
   }

   retval=PAPI_event_name_to_code("OFFCORE_RESPONSE_0:DMND_DATA_RD:LOCAL_DRAM",                                  &events[0]);
   if (retval!=PAPI_OK) {
      if (!quiet) printf("Error: PAPI_event_name_to_code %d\n", retval);      
      test_fail(test_string);
   }

   events[1]=PAPI_TOT_INS;

   retval=PAPI_start_counters(events,2);

   naive_matrix_multiply(quiet);

   PAPI_stop_counters(counts,2);
   
   if (retval!=PAPI_OK) {
     if (!quiet) printf("Error starting!\n");
     test_fail(test_string);
   }
  
   if (counts[0]==0) {
      if (!quiet) printf("Error!  Counted 0!\n");
      test_fail(test_string);
   }

   if (!quiet) {
     printf("Found %lld offcore events\n",counts[0]);
     printf("Found %lld cycles\n",counts[1]);
   }

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
