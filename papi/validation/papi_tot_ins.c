/* This file attempts to test the retired instruction */
/* performance counter on various architectures, as   */
/* implemented by the PAPI_TOT_INS counter.           */

/* by Vince Weaver, vweaver1@eecs.utk.edu             */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"

#include "instructions_testcode.h"

#define NUM_RUNS 100

char test_string[]="Testing \"instructions\" generalized event...";
int quiet=0;


   /* Test a simple loop of 1 million instructions             */
   /* Most implementations should count be correct within 1%   */
   /* This loop in in assembly language, as compiler generated */
   /* code varies too much.                                    */

void test_million() {

   int i;

   int events[1],result;
   long long counts[1],high=0,low=0,total=0,average=0;
   double error;

   events[0]=PAPI_TOT_INS;

   if (!quiet) printf("Testing a loop of 1 million instructions (%d times):\n",
          NUM_RUNS);

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

      result=instructions_million();
     
      PAPI_stop_counters(counts,1);

      if (result==CODE_UNIMPLEMENTED) {
	if (!quiet) fprintf(stderr,"\tCode unimplemented\n");
         test_fail(test_string); 
      }

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;

   error=display_error(average,high,low,1000000ULL,quiet);

   if ((error > 1.0) || (error<-1.0)) {

#if defined(__PPC__)

     if (!quiet) printf("If PPC is off by 50%%, this might be due to\n"
             "\"folded\" branch instructions on PPC32\n");

#endif

      if (!quiet) printf("Instruction count off by more than 1%%\n");
      test_fail(test_string);
   }
   if (!quiet) printf("\n");

}

void test_fldcw() {

#if defined(__i386__) || (defined __x86_64__)   
   int i;

   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   double error;
   int result;

   events[0]=PAPI_TOT_INS;

   if (!quiet) printf("Testing a fldcw loop of 900,000 instructions (%d times):\n",
          NUM_RUNS);

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

      result=instructions_fldcw();

      PAPI_stop_counters(counts,1);

      if (result==CODE_UNIMPLEMENTED) {
	if (!quiet) fprintf(stderr,"\tCode unimplemented\n");
	test_fail(test_string);
      }

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;

   error=display_error(average,high,low,900000ULL,quiet);

   if ((error > 1.0) || (error<-1.0)) {

     if (!quiet) {
        printf("On Pentium 4 machines, the fldcw instruction counts as 2.\n");
        printf("This will lead to an overcount of 22%%\n");

        printf("Instruction count off by more than 1%%\n");
     }
     test_fail(test_string);
   }
#endif
   if (!quiet) printf("\n");
}

void test_rep() {
#if defined(__i386__) || (defined __x86_64__)   
   int i;

   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   double error;
   int result;

   events[0]=PAPI_TOT_INS;

   if (!quiet) printf("Testing a 16k rep loop (%d times):\n",
          NUM_RUNS);

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

      result=instructions_rep();

      PAPI_stop_counters(counts,1);

      if (result==CODE_UNIMPLEMENTED) {
	if (!quiet) fprintf(stderr,"\tCode unimplemented\n");
	test_fail(test_string);
      }

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;

   error=display_error(average,high,low,6002,quiet);

   if ((error > 10.0) || (error<-10.0)) {

     if (!quiet) printf("Instruction count off by more than 10%%\n");
     test_fail(test_string);
   }
#endif
   if (!quiet) printf("\n");

}

int main(int argc, char **argv) {
   
   int retval;

   quiet=test_quiet();
   
   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("Error! PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }

   retval = PAPI_query_event(PAPI_TOT_INS);
   if (retval != PAPI_OK) {
      if (!quiet) printf("PAPI_TOT_INS not supported\n");
      test_skip(test_string);
   }

   if (!quiet) printf("\n");   
   test_million();
   test_rep();
   test_fldcw();

   PAPI_shutdown();

   test_pass(test_string);
   
   return 0;
}
