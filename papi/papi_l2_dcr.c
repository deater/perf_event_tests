/* This code runs some sanity checks on the PAPI_L2_DCR */
/*   (L2 Data Cache Reads) count.                       */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu          */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "cache_test.h"

#include "papi_test.h"

int main(int argc, char **argv) {

   int events[1],i;
   long long counts[1];
   
   int retval,num_counters;
   int l1_size,l2_size,l1_linesize,l2_entries;
   int arraysize;
   
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      test_fail(__FILE__,__LINE__,"PAPI_library_init",retval);
   }

   retval = PAPI_query_event(PAPI_L2_DCR);
   if (retval != PAPI_OK) {
      test_fail(__FILE__,__LINE__,"PAPI_L2_DCR not available",retval);
   }


   num_counters=PAPI_num_counters();

   events[0]=PAPI_L2_DCR;

   l1_size=get_cachesize(L1D_CACHE);
   l1_linesize=get_linesize(L1D_CACHE);
   l2_size=get_cachesize(L2_CACHE);
   l2_entries=get_entries(L2_CACHE);

   /*******************************************************************/
   /* Test if the C compiler uses a sane number of data cache acceess */
   /*******************************************************************/

   arraysize=l2_size/sizeof(double);

   double *array;
   double aSumm = 0.0;

   printf("Allocating %ld bytes of memory (%d doubles)\n",
          arraysize*sizeof(double),arraysize);

   array=calloc(arraysize,sizeof(double));
   if (array==NULL) {
      test_fail(__FILE__,__LINE__,"Can't allocate memory",0);      
   }

   printf("Write test:\n");
   PAPI_start_counters(events,1);
   
   for(i=0; i<arraysize; i++) { 
      array[i]=(double)i;
   }
     
   PAPI_stop_counters(counts,1);

   printf("\tL2 D reads: %lld\n",counts[0]);
   printf("\tShould be roughly: %d\n",0);

   PAPI_start_counters(events,1);
   
   for(i=0; i<arraysize; i++) { 
       aSumm += array[i]; 
   }
     
   PAPI_stop_counters(counts,1);

   printf("Read test (%lf):\n",aSumm);
   printf("\tL2 D reads: %lld\n",counts[0]);
   printf("\tShould be roughly (%d/%ld): %ld\n",
       l1_linesize,sizeof(double),
       arraysize/(l1_linesize/sizeof(double)));

   PAPI_shutdown();
   
   return 0;
}
