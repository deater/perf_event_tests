/* This file attempts to test the cycles              */
/* performance counter on various architectures, as   */
/* implemented by the PAPI_TOT_CYC counter.           */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "papiStdEventDefs.h"
#include "papi.h"


#include "test_utils.h"

#include "matrix_multiply.h"
#include "nops_testcode.h"


#define SLEEP_RUNS 3

int main(int argc, char **argv) {
   
   int retval,quiet;
   double mhz,mmm_mhz;
   const PAPI_hw_info_t *info;

   double error;

   int i;
   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   long long total_usecs,usecs,usec_start,usec_end;
   long long mmm_count;
   long long expected;

   char test_string[]="Testing PAPI_TOT_CYC predefined event...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_TOT_CYC);
   if (retval != PAPI_OK) {
      if (!quiet) printf("PAPI_TOT_CYC not supported\n");
      test_skip(test_string);
   }

   if (!quiet) printf("\n");   

   if ( (info=PAPI_get_hardware_info())==NULL) {
      if (!quiet) printf("Error! cannot obtain hardware info\n");
      test_fail(test_string);
   }
   mhz=info->mhz;
   if (!quiet) printf("System MHZ reported by PAPI = %f\n\n",mhz);

   events[0]=PAPI_TOT_CYC;

   if (!quiet) printf("Testing a sleep of 1 second (%d times):\n",
          SLEEP_RUNS);


   for(i=0;i<SLEEP_RUNS;i++) {

      PAPI_start_counters(events,1);

      sleep(1);
     
      PAPI_stop_counters(counts,1);

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/SLEEP_RUNS;

   if (!quiet) {

     printf("\tAverage should be low, as no user cycles when sleeping\n");
     printf("\tMeasured average: %lld\n",average);

   }

   if (average>1000000) {
     if (!quiet) printf("Average cycle count too high!\n");
     test_fail(test_string);
   }

   /*****************************/
   /* testing Matrix Matrix MHz */
   /*****************************/

   if (!quiet) printf("\nTesting MHz with matrix matrix multiply\n");
   total_usecs=0;

   usec_start=PAPI_get_real_usec();
   PAPI_start_counters(events,1);

   naive_matrix_multiply(quiet);

   PAPI_stop_counters(counts,1);
   usec_end=PAPI_get_real_usec();

   usecs=usec_end-usec_start;
      
   expected=(long long)(mhz*usecs);

   if (!quiet) {
     printf("\tExpected cycles = %.2lfMHz * %lld usecs = %lld\n",
	    mhz,usecs,expected);
     printf("\tActual measured cycles = %lld\n",counts[0]);
     printf("\tEstimated actual MHz = %.2lfMHz\n",
	    (double)counts[0]/(double)usecs);
   }

   error=  100.0 * (double)(counts[0]-expected) / (double)expected;

   if ((error>10.0) || (error<-10.0)) {

     if (!quiet) printf("\tWarning: Error %.2lf%% too high!\n",error);
   }

   if (!quiet) printf("\n");

   mmm_mhz=(double)counts[0]/(double)usecs;
   mmm_count=counts[0];

   /* Linear Speedup */

   if (!quiet) printf("Testing for a linear cycle increase\n");

#define REPITITIONS 2

   usec_start=PAPI_get_real_usec();
   PAPI_start_counters(events,1);

   for(i=0;i<REPITITIONS;i++) {
      naive_matrix_multiply(quiet);
   }

   PAPI_stop_counters(counts,1);
   usec_end=PAPI_get_real_usec();

   usecs=usec_end-usec_start;
      
   expected=mmm_count*REPITITIONS;

   error=  100.0 * (double)(counts[0]-expected) / (double)expected;

   if (!quiet) {
     printf("\tExpected %lld, got %lld\n",expected,counts[0]);

   }

   if ((error>10.0) || (error<-10.0)) {

     if (!quiet) printf("Error too high!\n");
     test_fail(test_string);
   }

   if (!quiet) printf("\n");

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
