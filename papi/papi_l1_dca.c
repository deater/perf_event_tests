/* This code runs some sanity checks on the PAPI_L1_DCA */
/*   (L1 Data Cache Accesses) count.                    */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu          */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "papi_test.h"

#define NUM_RUNS 100

int main(int argc, char **argv) {

   int events[1],i;
   long long counts[1];
   
   int retval,num_counters;
   
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      test_fail(__FILE__,__LINE__,"PAPI_library_init",retval);
   }

   retval = PAPI_query_event(PAPI_L1_DCA);
   if (retval != PAPI_OK) {
      test_fail(__FILE__,__LINE__,"PAPI_L1_DCA not available",retval);
   }


   num_counters=PAPI_num_counters();

   events[0]=PAPI_L1_DCA;


   /*******************************************************************/
   /* Test if the C compiler uses a sane number of data cache acceess */
   /*******************************************************************/

#define ARRAYSIZE 1000

   double array[ARRAYSIZE];
   double aSumm = 0.0;

   printf("Write test:\n");
   PAPI_start_counters(events,1);
   
   for(i=0; i<ARRAYSIZE; i++) { 
      array[i]=(double)i;
   }
     
   PAPI_stop_counters(counts,1);


   printf("\tL1 D accesseses: %lld\n",counts[0]);
   printf("\tShould be roughly: %d\n",ARRAYSIZE);

   PAPI_start_counters(events,1);
   
   for(i=0; i<ARRAYSIZE; i++) { 
       aSumm += array[i]; 
   }
     
   PAPI_stop_counters(counts,1);

   printf("Read test (%lf):\n",aSumm);
   printf("\tL1 D accesseses: %lld\n",counts[0]);
   printf("\tShould be roughly: %d\n",ARRAYSIZE);

   PAPI_shutdown();
   
   return 0;
}
