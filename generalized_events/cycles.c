/* This file attempts to test the cycles              */
/* performance counter on various architectures, as   */
/* implemented by the cycles generalized event        */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <time.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"

#include "matrix_multiply.h"

#define SLEEP_RUNS 3


long long convert_to_ns(struct timespec *before,
                        struct timespec *after) {

  long long seconds;
  long long ns;

  seconds=after->tv_sec - before->tv_sec;
  ns = after->tv_nsec - before->tv_nsec;

  ns = (seconds*1000000000ULL)+ns;

  return ns;
}


int main(int argc, char **argv) {
   
   int quiet;
   double mmm_ghz;

   double error;

   int i,fd,read_result;
   long long count,high=0,low=0,total=0,average=0;
   long long nsecs;
   long long mmm_count;
   long long expected;

   struct perf_event_attr pe;

   struct timespec before,after;

   char test_string[]="Testing \"cycles\" generalized event...";

   quiet=test_quiet();

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_CPU_CYCLES;
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
     test_fail(test_string);
   }

   if (!quiet) {
      printf("Testing a sleep of 1 second (%d times):\n",SLEEP_RUNS);
   }


   for(i=0;i<SLEEP_RUNS;i++) {

      ioctl(fd, PERF_EVENT_IOC_RESET, 0);
      ioctl(fd, PERF_EVENT_IOC_ENABLE,0);
     
      sleep(1);
     
      ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
      read_result=read(fd,&count,sizeof(long long));
      if (read_result!=sizeof(long long)) printf("Read wrong size\n");

      if (count>high) high=count;
      if ((low==0) || (count<low)) low=count;
      total+=count;
   }

   average=total/SLEEP_RUNS;

   if (!quiet) {

     printf("\tAverage should be low, as no user cycles when sleeping\n");
     printf("\tMeasured average: %lld\n",average);

   }

   if (average>100000) {
     if (!quiet) printf("Average cycle count too high!\n");
     test_fail(test_string);
   }

   /*****************************/
   /* testing Matrix Matrix GHz */
   /*****************************/

   if (!quiet) {
      printf("\nEstimating GHz with matrix matrix multiply\n");
   }

   clock_gettime(CLOCK_REALTIME,&before);

   ioctl(fd, PERF_EVENT_IOC_RESET, 0);
   ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

   naive_matrix_multiply(quiet);

   ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   read_result=read(fd,&count,sizeof(long long));
   if (read_result!=sizeof(long long)) printf("Read wrong size\n");

   clock_gettime(CLOCK_REALTIME,&after);

   nsecs=convert_to_ns(&before,&after);

   mmm_ghz=(double)count/(double)nsecs;
      
   if (!quiet) {
     printf("\tActual measured cycles = %lld\n",count);
     printf("\tEstimated actual GHz = %.2lfGHz\n",mmm_ghz);
   }


   mmm_count=count;

   /************************************/
   /* Check for Linear Speedup         */
   /************************************/
   if (!quiet) printf("Testing for a linear cycle increase\n");

#define REPITITIONS 2

   clock_gettime(CLOCK_REALTIME,&before);

   ioctl(fd, PERF_EVENT_IOC_RESET, 0);
   ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

   for(i=0;i<REPITITIONS;i++) {
      naive_matrix_multiply(quiet);
   }

   ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   read_result=read(fd,&count,sizeof(long long));
   if (read_result!=sizeof(long long)) printf("Read wrong size\n");

   clock_gettime(CLOCK_REALTIME,&after);

   nsecs=convert_to_ns(&before,&after);
      
   expected=mmm_count*REPITITIONS;

   error=  100.0 * (double)(count-expected) / (double)expected;

   if (!quiet) {
     printf("\tExpected %lld, got %lld\n",expected,count);
     printf("\tError=%.2f%%\n",error);
   }

   if ((error>10.0) || (error<-10.0)) {

     if (!quiet) printf("Error too high!\n");
     test_fail(test_string);
   }

   if (!quiet) printf("\n");

   test_pass(test_string);

   return 0;
}
