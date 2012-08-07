/* This file attempts to test the hardware breakpoint support */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


char test_string[]="Testing hardware breakpoints...";
int quiet=0;
int fd;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include "perf_event.h"
#include "hw_breakpoint.h"

#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"


int test_function(int a, int b) {

  /* The time thing is there to keep the compiler */
  /* from optimizing this away.                   */
  return a+b+time(NULL);

}

volatile int test_variable=5;

#define EXECUTIONS 10
#define READS 15
#define WRITES 20

int main(int argc, char **argv) {

   struct perf_event_attr pe;
   int i, sum=0, read_result, passes=0;
   long long count;

   void *address;

   address=test_function;

   quiet=test_quiet();
   
   if (!quiet) printf("This test checks that hardware breakpoints work.\n");

   /*******************************/
   /* Test execution breakpoint   */
   /*******************************/

   if (!quiet) printf("\tTesting HW_BREAKPOINT_X\n");

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_BREAKPOINT;
   pe.size=sizeof(struct perf_event_attr);

   /* setup for an execution breakpoint */
   pe.config=0;
   pe.bp_type=HW_BREAKPOINT_X;
   pe.bp_addr=(unsigned long)address;
   pe.bp_len=sizeof(long);

   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     fprintf(stderr,"Error opening leader %llx\n",pe.config);
     test_fail(test_string);
   }

   ioctl(fd, PERF_EVENT_IOC_RESET, 0);
   ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

   /* Access a function 10 times */
   for(i=0;i<EXECUTIONS;i++) {
     sum+=test_function(i,sum);
   }

   ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   read_result=read(fd,&count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string); 
   }

   if (!quiet) printf("\t\tTried %d calls, found %lld times, sum=%d\n",
		      EXECUTIONS,count,sum);

   if (count!=EXECUTIONS) {
      fprintf(stderr,"\tWrong number of executions "
	      "%lld != %d\n",count,EXECUTIONS);
      test_fail(test_string);
   }

   close(fd);
   passes++;

   /*******************************/
   /* Test write breakpoint       */
   /*******************************/

   if (!quiet) printf("\tTesting HW_BREAKPOINT_W\n");

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_BREAKPOINT;
   pe.size=sizeof(struct perf_event_attr);

   /* setup for an execution breakpoint */
   pe.config=0;
   pe.bp_type=HW_BREAKPOINT_W;
   pe.bp_addr=(long)&test_variable;
   pe.bp_len=HW_BREAKPOINT_LEN_4;

   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     fprintf(stderr,"Error opening leader %llx\n",pe.config);
     test_fail(test_string);
   }

   ioctl(fd, PERF_EVENT_IOC_RESET, 0);
   ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

   /* Read a variable WRITES times */
   for(i=0;i<WRITES;i++) {
      test_variable=sum+i;
   }

   ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   read_result=read(fd,&count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string); 
   }

   if (!quiet) printf("\t\tTried %d writes, found %lld times, sum=%d\n",
		      WRITES,count,sum);

   if (count!=WRITES) {
      fprintf(stderr,"\tWrong number of writes "
	      "%lld != %d\n",count,WRITES);
      test_fail(test_string);
   }

   close(fd);
   passes++;

   /*******************************/
   /* Test read breakpoint        */
   /*******************************/

   if (!quiet) printf("\tTesting HW_BREAKPOINT_R\n");

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_BREAKPOINT;
   pe.size=sizeof(struct perf_event_attr);

   /* setup for an execution breakpoint */
   pe.config=0;
   pe.bp_type=HW_BREAKPOINT_R;
   pe.bp_addr=(long)&test_variable;
   pe.bp_len=HW_BREAKPOINT_LEN_4;

   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     if (!quiet) {
        printf("\t\tError opening leader %llx\n",pe.config);
        printf("\t\tRead breakpoints probably not supported, skipping\n");
     }
     goto skip_reads;

   }

   ioctl(fd, PERF_EVENT_IOC_RESET, 0);
   ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

   /* Read a variable READS times */
   for(i=0;i<READS;i++) {
      sum+=test_variable;
   }

   ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   read_result=read(fd,&count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string); 
   }

   if (!quiet) printf("\t\tTrying %d reads, found %lld times, sum=%d\n",
		      READS,count,sum);

   if (count!=READS) {
      fprintf(stderr,"\tWrong number of reads "
	      "%lld != %d\n",count,READS);
      test_fail(test_string);
   }

   close(fd);
   passes++;

skip_reads:

   if (passes>0) {
      test_pass(test_string);
   } else {
      test_skip(test_string);
   }

   return 0;
}
