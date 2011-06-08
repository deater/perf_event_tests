/* This file attempts to test the number of hardware */
/* interrupts, as provided by PAPI_HW_INT            */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu       */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "papi_test.h"

#define NUM_RUNS 10000


   /* Test a simple loop of 1 million instructions             */
   /* Most implementations should count be correct within 1%   */
   /* This loop in in assembly language, as compiler generated */
   /* code varies too much.                                    */

void test_million() {

   int i;

   int events[1];
   long long counts[1];

   events[0]=PAPI_HW_INT;

   printf("Testing a loop of 1 million instructions (%d times):\n",
          NUM_RUNS);
      
   PAPI_start_counters(events,1);


   for(i=0;i<NUM_RUNS;i++) {


#if defined(__i386__) || (defined __x86_64__)   
   asm("\txor %%ecx,%%ecx\n"
       "\tmov $499999,%%ecx\n"
       "test_loop:\n"
       "\tdec %%ecx\n"
       "\tjnz test_loop\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "%ecx" /* clobbered */
    );
#elif defined(__PPC__)
   asm("\tnop                           # to give us an even million\n"
       "\tlis     15,499997@ha          # load high 16-bits of counter\n"
       "\taddi    15,15,499997@l        # load low 16-bits of counter\n"
       "55:\n"
       "\taddic.  15,15,-1              # decrement counter\n"
       "\tbne     0,55b                  # loop until zero\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "15" /* clobbered */
    );
#elif defined(__ia64__)
   asm("\tmov     loc6=166666           // below is 6 instr.\n"
       ";;                              // because of that we count 4 too few\n"
       "55:\n"
       "\tadd     loc6=-1,loc6          // decrement count\n"
       ";;\n"
       "\tcmp.ne  p2,p3=0,loc6\n"
       "(p2)    br.cond.dptk    55b     // if not zero, loop\n"
       : /* no output registers */
       : /* no inputs */
       : "p2", "loc6" /* clobbered */
    );
#elif defined(__sparc__)
   asm("\tsethi     %%hi(333333), %%l0\n"
       "\tor        %%l0,%%lo(333333),%%l0\n"
       "test_loop:\n"
       "\tdeccc   %%l0             ! decrement count\n"
       "\tbnz     test_loop        ! repeat until zero\n"
       "\tnop                      ! branch delay slot\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "l0" /* clobbered */
    );
#else
   test_fail( __FILE__, __LINE__, "Unknown architecture", 0);
#endif


   }

   PAPI_stop_counters(counts,1);

   printf("   Expected: >0\n");
   printf("   Obtained: %lld\n",counts[0]);

   if (counts[0] == 0) {
      test_fail( __FILE__, __LINE__, "Interrupt count was zero", 0);
   }
   printf("\n");

}


int main(int argc, char **argv) {
   
   int retval;
   
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      test_fail( __FILE__, __LINE__, "PAPI_library_init", retval);
   }

   retval = PAPI_query_event(PAPI_HW_INT);
   if (retval != PAPI_OK) {
      test_fail_exit( __FILE__, __LINE__ ,"PAPI_HW_INT not supported", retval);
   }

   printf("\n");   
   test_million();

   PAPI_shutdown();

   test_pass( __FILE__ , NULL, 0);
   
   return 0;
}
