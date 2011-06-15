/* This code runs some sanity checks on the PAPI_L2_DCA */
/*   (L2 Data Cache Accesses) count.                    */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu          */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"

#include "papi_cache_info.h"

int main(int argc, char **argv) {

   int events[1],i;
   long long counts[1];
   
   int retval,quiet;
   int l1_size,l2_size,l1_linesize,l2_entries;
   int arraysize;

   char test_string[]="Testing PAPI_L2_DCA predefined event...";
   
   quiet=test_quiet();
   
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      printf("Error! PAPI_library_init %d\n",retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_L2_DCA);
   if (retval != PAPI_OK) {
      printf("PAPI_L2_DCA not available\n");
      test_skip(test_string);
   }

   events[0]=PAPI_L2_DCA;

   l1_size=get_cachesize(L1D_CACHE,quiet,test_string);
   l1_linesize=get_linesize(L1D_CACHE,quiet,test_string);
   l2_size=get_cachesize(L2_CACHE,quiet,test_string);
   l2_entries=get_entries(L2_CACHE,quiet,test_string);

   /*******************************************************************/
   /* Test if the C compiler uses a sane number of data cache acceess */
   /*******************************************************************/

   arraysize=l2_size/sizeof(double);

   double *array;
   double aSumm = 0.0;

   if (!quiet) {
      printf("Allocating %ld bytes of memory (%d doubles)\n",
          arraysize*sizeof(double),arraysize);
   }

   array=calloc(arraysize,sizeof(double));
   if (array==NULL) {
      if (!quiet) printf("Error: Can't allocate memory\n");
      test_fail(test_string);
   }

   if (!quiet) printf("Write test:\n");

   PAPI_start_counters(events,1);
   
   for(i=0; i<arraysize; i++) { 
      array[i]=(double)i;
   }
     
   PAPI_stop_counters(counts,1);

   if (!quiet) {
      printf("\tL2 D accesseses: %lld\n",counts[0]);
      printf("\tShould be roughly (%d/%ld): %ld\n",
             l1_linesize,sizeof(double),
             arraysize/(l1_linesize/sizeof(double)));
   }

   if (counts[0]<1) {
      if (!quiet) printf("Error! Count too low\n");
      test_fail(test_string);
   }

   /* reads */

   PAPI_start_counters(events,1);
   
   for(i=0; i<arraysize; i++) { 
       aSumm += array[i]; 
   }
     
   PAPI_stop_counters(counts,1);

   if (!quiet) {
      printf("Read test (%lf):\n",aSumm);
      printf("\tL2 D accesseses: %lld\n",counts[0]);
      printf("\tShould be roughly (%d/%ld): %ld\n",
          l1_linesize,sizeof(double),
          arraysize/(l1_linesize/sizeof(double)));
   }

   if (counts[0]<1) {
      if (!quiet) printf("Error! Count too low\n");
      test_fail(test_string);
   }

   PAPI_shutdown();

   test_pass(test_string);
   
   return 0;
}
