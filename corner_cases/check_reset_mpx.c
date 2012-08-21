/* check_reset_mpx.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Check to see what happens to multiplexing fields */
/* When a PERF_EVENT_IOC_RESET happens.             */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <errno.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define NUM_EVENTS 10

int fd[NUM_EVENTS];
long long events[NUM_EVENTS]={
   PERF_COUNT_HW_CPU_CYCLES,
   PERF_COUNT_HW_INSTRUCTIONS,
   PERF_COUNT_HW_CPU_CYCLES,
   PERF_COUNT_HW_INSTRUCTIONS,
   PERF_COUNT_HW_CPU_CYCLES,
   PERF_COUNT_HW_INSTRUCTIONS,
   PERF_COUNT_HW_CPU_CYCLES,
   PERF_COUNT_HW_INSTRUCTIONS,
   PERF_COUNT_HW_CPU_CYCLES,
   PERF_COUNT_HW_INSTRUCTIONS,
};

long long base_results[NUM_EVENTS][3];
long long mpx_results[NUM_EVENTS][5];
long long scaled_results[NUM_EVENTS];
long long scale;
double error[NUM_EVENTS];

int test_routine(void) {

  int i,result;

  for(i=0;i<500;i++) {
     result=instructions_million();
  }

  return result;
}

int main(int argc, char** argv) {
   
   int ret,quiet,i;

   struct perf_event_attr pe;

   char test_string[]="Testing if reset clears multiplex fields...";

   quiet=test_quiet();
   
   if (!quiet) {
      printf("The PERF_EVENT_IOC_RESET ioctl() clears the event count.\n");
      printf("  Check to see if it clears other fields too, such as\n");
      printf("  time_enabled and time_running.  Traditionall it does not.\n");
   }

   /* Setup 10 counters */

   for(i=0;i<NUM_EVENTS;i++) {

      memset(&pe,0,sizeof(struct perf_event_attr));
      pe.type=PERF_TYPE_HARDWARE;
      pe.size=sizeof(struct perf_event_attr);
      pe.config=events[i];
      pe.disabled=1;
      pe.exclude_kernel=1;
      pe.exclude_hv=1;
      pe.read_format=PERF_FORMAT_TOTAL_TIME_ENABLED | 
	             PERF_FORMAT_TOTAL_TIME_RUNNING;

      arch_adjust_domain(&pe,quiet);

      fd[i]=perf_event_open(&pe,0,-1,-1,0);
      if (fd[i]<0) {
	 fprintf(stderr,"Failed adding mpx event %d %s\n",i,strerror(errno));
         test_fail(test_string);
	 return -1;
      }
   }


   /**************************/
   /* Start all the counters */
   /**************************/

   for(i=0;i<NUM_EVENTS;i++) {
      ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
      if (ret<0) {
	 fprintf(stderr,"Error starting event %d\n",i);
      }
   }

   test_routine();

   /*************************/
   /* Stop all the counters */
   /*************************/

   for(i=0;i<NUM_EVENTS;i++) {
      ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
      if (ret<0) {
	 fprintf(stderr,"Error stopping event %d\n",i);
      }
   }

   if (!quiet) printf("Initial results\n");

   /*********************/
   /* Read the counters */
   /*********************/

   for(i=0;i<NUM_EVENTS;i++) {
      ret=read(fd[i],&base_results[i],5*sizeof(long long));
      if (ret<3*sizeof(long long)) {
	fprintf(stderr,"Event %d unexpected read size %d\n",i,ret);
        test_fail(test_string);
      }
      if (!quiet) {
         printf("\t%d %lld %lld %lld\n",i,
	     base_results[i][0],base_results[i][1],base_results[i][2]);
      }
   }

   /**********************/
   /* Reset the counters */
   /**********************/

   for(i=0;i<NUM_EVENTS;i++) {
      ret=ioctl(fd[i], PERF_EVENT_IOC_RESET,0);
      if (ret<0) {
	 fprintf(stderr,"Error resetting event %d\n",i);
      }
   }

   /************************/
   /* Re-read the counters */
   /************************/
     

   if (!quiet) printf("After reset:\n");

   for(i=0;i<NUM_EVENTS;i++) {
      ret=read(fd[i],&mpx_results[i],5*sizeof(long long));
      if (ret<3*sizeof(long long)) {
	fprintf(stderr,"Event %d unexpected read size %d\n",i,ret);
        test_fail(test_string);
      }
      if (!quiet) {
         printf("\t%d %lld %lld %lld\n",i,
	     mpx_results[i][0],mpx_results[i][1],mpx_results[i][2]);
      }
      if (mpx_results[i][0]!=0) {
	 fprintf(stderr,"Event %d not reset to 0\n",i);
         test_fail(test_string);
      }

      if (mpx_results[i][1]==0) {
	 fprintf(stderr,"Enabled %d should not be 0\n",i);
         test_fail(test_string);
      }

      if (mpx_results[i][2]==0) {
	 fprintf(stderr,"Running %d should not be  0\n",i);
         test_fail(test_string);
      }

   }

   test_pass(test_string);
      
   return 0;
}

