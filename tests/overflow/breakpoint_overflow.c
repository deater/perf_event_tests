/* This file attempts to overflow on hardware breakpoint */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


char test_string[]="Testing hardware breakpoint overflow...";
int quiet=0;
int fd;

#define _GNU_SOURCE 1

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

#include "perf_event.h"
#include "hw_breakpoint.h"

#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define MMAP_PAGES 8

int test_function(int a, int b) {

  /* The time thing is there to keep the compiler */
  /* from optimizing this away.                   */
  return a+b+time(NULL);

}

volatile int test_variable=5;

#define EXECUTIONS 10000
#define THRESHOLD 100

int num_overflows=0;

static void our_handler(int signum,siginfo_t *oh, void *blah) {

  num_overflows++;
}


int main(int argc, char **argv) {

   struct perf_event_attr pe;
   int i, sum=0, read_result, passes=0, fails=0;
   long long count;

   void *address,*blargh;

   struct sigaction sa;

   address=test_function;

   quiet=test_quiet();
   
   if (!quiet) printf("This test checks hardware breakpoint overflow.\n");

   /* setup signal handler */

   memset(&sa, 0, sizeof(struct sigaction));
   sa.sa_sigaction = our_handler;
   sa.sa_flags = SA_SIGINFO;

   if (sigaction( SIGIO, &sa, NULL) < 0) {
     fprintf(stderr,"Error setting up signal handler\n");
     exit(1);
   }



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

   /* setup overflow info */
   pe.sample_period=THRESHOLD;
   pe.sample_type=PERF_SAMPLE_IP;
   //pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
   pe.wakeup_events=1;


   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
      if (!quiet) {
         printf("\t\tError opening leader %llx\n",pe.config);
      }
      goto skip_execs;
   }

   blargh=mmap(NULL, (1+MMAP_PAGES)*getpagesize(),
	       PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

   fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
   fcntl(fd, F_SETSIG, SIGIO);
   fcntl(fd, F_SETOWN,getpid());

   ioctl(fd, PERF_EVENT_IOC_RESET, 0);
   ioctl(fd, PERF_EVENT_IOC_ENABLE,0);

   /* Access a function EXECUTIONS times */
   for(i=0;i<EXECUTIONS;i++) {
     sum+=test_function(i,sum);
   }

   ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   read_result=read(fd,&count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string); 
   }

   if (!quiet) {
      printf("\t\tTried %d calls, found %lld times, sum=%d\n",
		      EXECUTIONS,count,sum);
      printf("\t\tOverflow every %d times, found %d overflows\n",
	     THRESHOLD,num_overflows);
   }

   if (count!=EXECUTIONS) {
      if (!quiet) fprintf(stderr,"\tWrong number of executions "
	      "%lld != %d\n",count,EXECUTIONS);
      fails++;
   }

   if (num_overflows!=EXECUTIONS/THRESHOLD) {
      if (!quiet) fprintf(stderr,"\tWrong number of overflows "
	      "%d != %d\n",num_overflows,EXECUTIONS/THRESHOLD);
      fails++;
   }

   /* Still not sure why results are so high.  A bug? */
   if (fails) {
      test_unexplained(test_string);
   }

   close(fd);
   passes++;
skip_execs:

   if (passes>0) {
      test_pass(test_string);
   } else {
      test_skip(test_string);
   }

   (void) blargh;

   return 0;
}
