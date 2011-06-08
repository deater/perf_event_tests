#include <stdio.h>
#include <stdlib.h>

#include "papi.h"

long long branches_test(int event, int num_runs,
			long long *high,
			long long *low) {

   int i;

   int events[1];
   long long counts[1],total=0;

   events[0]=event;
   *high=0;
   *low=0;

   for(i=0;i<num_runs;i++) {

      PAPI_start_counters(events,1);

#if defined(__i386__) || (defined __x86_64__)   
   asm("\txor %%ecx,%%ecx\n"
       "\tmov $500000,%%ecx\n"
       "test_loop:\n"
       "\tjmp test_jmp\n"
       "\tnop\n"
       "test_jmp:\n"
       "\txor %%eax,%%eax\n"
       "\tjnz test_jmp2\n"
       "\tinc %%eax\n"
       "test_jmp2:\n"
       "\tdec %%ecx\n"
       "\tjnz test_loop\n"
       : /* no output registers */
       : /* no inputs */
       : "cc", "%ecx", "%eax" /* clobbered */
    );
#else
   test_fail( __FILE__, __LINE__, "Unknown architecture", 0);
#endif
     
      PAPI_stop_counters(counts,1);

      if (counts[0]>*high) *high=counts[0];
      if ((*low==0) || (counts[0]<*low)) *low=counts[0];
      total+=counts[0];
   }

   return (total/num_runs);
}

long long random_branches(int event, int num_runs,
			long long *high,
			long long *low) {

  int i,j,junk=0;
  double junk2=5.0;

   int events[1];
   long long counts[1],total=0;

   events[0]=event;
   *high=0;
   *low=0;

   for(i=0;i<num_runs;i++) {

      PAPI_start_counters(events,1);

      for(j=0;j<500000;j++) {
	
	if (( ((random()>>2)^(random()>>4)) %1000)>500) goto label_false;


	junk++;   /* can't just add, the optimizer is way too clever */

	junk2*=junk;

	//printf("T");
      label_false:
	//printf("F");
	;
      }

     
      PAPI_stop_counters(counts,1);

      if (counts[0]>*high) *high=counts[0];
      if ((*low==0) || (counts[0]<*low)) *low=counts[0];
      total+=counts[0];
   }
   printf("Number of times false: %d (%lf)\n",
	  junk/num_runs,junk2);

   return (total/num_runs);
}
