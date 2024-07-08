/* This code runs some sanity checks on the PAPI_L1_DCA */
/*   (L1 Data Cache Accesses) count.                    */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu          */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"


#define NUM_RUNS 100

int main(int argc, char **argv) {

   int quiet;
   int events[1],i;
   long long counts[1];

   int retval,num_counters;

   char test_string[]="Testing PAPI_L1_DCA predefined event...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("Error PAPI_library_init: %d\n",retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_L1_DCA);
   if (retval != PAPI_OK) {
      if (!quiet) printf("PAPI_L1_DCA not available\n");
      test_fail(test_string);
   }


   num_counters=PAPI_num_counters();

   events[0]=PAPI_L1_DCA;


   /*******************************************************************/
   /* Test if the C compiler uses a sane number of data cache acceess */
   /*******************************************************************/

#define ARRAYSIZE 1000

   double array[ARRAYSIZE];
   double aSumm = 0.0;

   if (!quiet) printf("Write test:\n");
   PAPI_start_counters(events,1);
   
   for(i=0; i<ARRAYSIZE; i++) { 
      array[i]=(double)i;
   }
     
   PAPI_stop_counters(counts,1);

   if (!quiet) {
      printf("\tL1 D accesseses: %lld\n",counts[0]);
      printf("\tShould be roughly: %d\n",ARRAYSIZE);
   }

   PAPI_start_counters(events,1);
   
   for(i=0; i<ARRAYSIZE; i++) { 
       aSumm += array[i]; 
   }
     
   PAPI_stop_counters(counts,1);

   if (!quiet) {
      printf("Read test (%lf):\n",aSumm);
      printf("\tL1 D accesseses: %lld\n",counts[0]);
      printf("\tShould be roughly: %d\n",ARRAYSIZE);
   }

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
