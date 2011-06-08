/* This file attempts to test the retired instruction */
/* performance counter on various architectures, as   */
/* implemented by the PAPI_TOT_INS counter.           */

/* by Vince Weaver, vweaver1@eecs.utk.edu             */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "papi_test.h"

#define NUM_RUNS 100


   /* Test a simple loop of 1 million instructions             */
   /* Most implementations should count be correct within 1%   */
   /* This loop in in assembly language, as compiler generated */
   /* code varies too much.                                    */

void test_million() {

   int i;

   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   double error;

   events[0]=PAPI_TOT_INS;

   printf("Testing a loop of 1 million instructions (%d times):\n",
          NUM_RUNS);

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

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
     
      PAPI_stop_counters(counts,1);

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;
   printf("   Expected: %lld\n", 1000000ULL);
   printf("   High: %lld   Low:  %lld   Average:  %lld\n",
          high,low,average);

   printf("   ( note, a small value above 1,000,000 may be expected due\n");
   printf("     to PAPI overhead and interrupt noise, among other reasons)\n");

   error=(((double)average-1.0e6)/1.0e6)*100.0;
   printf("   Average Error = %.2f%%\n",error);

   if ((error > 1.0) || (error<-1.0)) {

#if defined(__PPC__)

      printf("If PPC is off by 50%%, this might be due to\n"
             "\"folded\" branch instructions on PPC32\n");

#endif

      test_fail( __FILE__, __LINE__, "Instruction count off by more than 1%", 0);
   }
   printf("\n");

}

void test_fldcw() {

#if defined(__i386__) || (defined __x86_64__)   
   int i;

   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   double error,three=3.0;
   int saved_cw,result,cw;

   events[0]=PAPI_TOT_INS;

   printf("Testing a fldcw loop of 900,000 instructions (%d times):\n",
          NUM_RUNS);

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

   asm( "mov     $100000,%%ecx\n"
        "big_loop:\n"
        "\tfldl    %1                   # load value onto fp stack\n"
        "\tfnstcw  %0                # store control word to mem\n"
        "\tmovzwl  %0, %%eax          # load cw from mem, zero extending\n"
        "\tmovb    $12, %%ah                # set cw for \"round to zero\"\n"
        "\tmovw    %%ax, %3                 # store back to memory\n"
        "\tfldcw   %3                      # save new rounding mode\n"
        "\tfistpl  %2                  # save stack value as integer to mem\n"
        "\tfldcw   %0                # restore old cw\n"
        "\tloop    big_loop                # loop to make the count more obvious\n"
       : /* no output registers */
       : "m"(saved_cw), "m"(three), "m"(result), "m"(cw) /* inputs */
       : "cc", "%ecx","%eax" /* clobbered */
    );

      PAPI_stop_counters(counts,1);

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;
   printf("   Expected: %lld\n", 900000ULL);
   printf("   High: %lld   Low:  %lld   Average:  %lld\n",
          high,low,average);

   printf("   ( note, a small value above 900,000 may be expected due\n");
   printf("     to PAPI overhead and interrupt noise, among other reasons)\n");

   error=(((double)average-9.0e5)/9.0e5)*100.0;
   printf("   Average Error = %.2f%%\n",error);

   if ((error > 1.0) || (error<-1.0)) {

     printf("On Pentium 4 machines, the fldcw instruction counts as 2.\n");
     printf("This will lead to an overcount of 22%%\n");

     test_fail( __FILE__, __LINE__, "Instruction count off by more than 1%", 0);
   }
#endif
   printf("\n");
}

void test_rep() {
#if defined(__i386__) || (defined __x86_64__)   
   int i;

   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   double error;
   char buffer_out[16384];
   long long expected=6002;

   events[0]=PAPI_TOT_INS;

   printf("Testing a 16k rep loop (%d times):\n",
          NUM_RUNS);

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);
#if defined(__i386__) 
   asm("\tmov       $1000,%%edx\n"
       "\tcld\n"
       "loadstore:                       # test 8-bit store\n"
       "\tmov     $0xd, %%al             # set eax to d\n"
       "\tmov     $16384, %%ecx\n"
       "\tmov     %0, %%edi              # set destination\n"
       "\trep     stosb                  # store d 16384 times, auto-increment\n"
       "\tdec     %%edx\n"
       "\tjnz     loadstore\n"
       : /* outputs */
       : "rm" (buffer_out) /* inputs */
       : "cc", "%esi","%edi","%edx","%ecx","%eax","memory" /* clobbered */
    );
#elif defined (__x86_64__)   
   asm("\tmov       $1000,%%edx\n"
       "\tcld\n"
       "loadstore:                       # test 8-bit store\n"
       "\tmov     $0xd, %%al             # set eax to d\n"
       "\tmov     $16384, %%ecx\n"
       "\tmov     %0, %%rdi              # set destination\n"
       "\trep     stosb                  # store d 16384 times, auto-increment\n"
       "\tdec     %%edx\n"
       "\tjnz     loadstore\n"
       : /* outputs */
       : "rm" (buffer_out) /* inputs */
       : "cc", "%esi","%edi","%edx","%ecx","%eax","memory" /* clobbered */
    );
#endif
      PAPI_stop_counters(counts,1);

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;
   printf("   Expected: %lld\n", expected);
   printf("   High: %lld   Low:  %lld   Average:  %lld\n",
          high,low,average);

   printf("   ( note, a small value above %lld may be expected due\n",expected);
   printf("     to PAPI overhead and interrupt noise, among other reasons)\n");

   error=(((double)average-expected)/expected)*100.0;
   printf("   Average Error = %.2f%%\n",error);

   if ((error > 10.0) || (error<-10.0)) {

     test_fail( __FILE__, __LINE__, "Instruction count off by more than 10%", 0);
   }
#endif
   printf("\n");

}

int main(int argc, char **argv) {
   
   int retval;
   
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      test_fail( __FILE__, __LINE__, "PAPI_library_init", retval);
   }

   retval = PAPI_query_event(PAPI_TOT_INS);
   if (retval != PAPI_OK) {
      test_fail( __FILE__, __LINE__ ,"PAPI_TOT_INS not supported", retval);
   }

   printf("\n");   
   test_million();
   test_rep();
   test_fldcw();

   PAPI_shutdown();

   test_pass( __FILE__ , NULL, 0);
   
   return 0;
}
