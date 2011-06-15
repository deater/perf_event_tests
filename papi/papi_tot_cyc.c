/* This file attempts to test the retired instruction */
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

int main(int argc, char **argv) {
   
   int retval,quiet;
   double mhz;
   const PAPI_hw_info_t *info;

   double error;

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
   if (!quiet) printf("System MHZ = %f\n",mhz);

#define NUM_RUNS 3

   int i;
   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   long long total_usecs,avg_usecs,usecs,usec_start,usec_end;
   long long expected;

   events[0]=PAPI_TOT_CYC;

   if (!quiet) printf("Testing a sleep of 1 second (%d times):\n",
          NUM_RUNS);


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

     printf("Average should be low, as no user cycles when sleeping\n");
     printf("Measured average: %lld\n",average);

   }


   if (average>1000000) {
     if (!quiet) printf("Average cycle count too high!\n");
     test_fail(test_string);
   }



   /* busy spin */
#define NUM_MATRIX_RUNS 5

   if (!quiet) printf("Testing a busy loop (%d times):\n",
          NUM_MATRIX_RUNS);

   high=0; low=0; total=0;
   total_usecs=0;

   for(i=0;i<NUM_MATRIX_RUNS;i++) {


      usec_start=PAPI_get_real_usec();
      PAPI_start_counters(events,1);

      naive_matrix_multiply(quiet);

      PAPI_stop_counters(counts,1);
      usec_end=PAPI_get_real_usec();

      usecs=usec_end-usec_start;
      total_usecs+=usecs;

      total+=counts[0];

      if (counts[0]>high) high=counts[0];
      if ((counts[0]<low) || (low==0)) low=counts[0];

   }
 
   avg_usecs=total_usecs/NUM_MATRIX_RUNS;
   
   expected=(long long)(mhz*avg_usecs);

   average=total/NUM_MATRIX_RUNS;

   error=display_error(average,high,low,expected,quiet);

   if ((error>10.0) || (error<-10.0)) {

     if (!quiet) printf("Error too high!\n");
     test_fail(test_string);
   }

   if (!quiet) printf("\n");

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
