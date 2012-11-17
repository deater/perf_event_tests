/* ioctl_refresh_0.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Currently (as of 2.6.39) you can send a PERF_EVENT_IOC_REFRESH */
/*   ioctl with a "0" argument and that will arm refresh          */
/*   for all events in a group.                                   */
/* This is apparently undocumented/undefined behavior.            */
/* See https://lkml.org/lkml/2011/5/24/337                        */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "matrix_multiply.h"

static int count=0;
static int fd1;

static void our_handler(int signum,siginfo_t *oh, void *blah) {

  int ret;

  count++;

  ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,0);
  (void) ret;
}

int main(int argc, char** argv) {
   
   int ret,quiet;   

   struct perf_event_attr pe;

   struct sigaction sa;
   void *blargh;
   char test_string[]="Testing if PERF_IOC_REFRESH with 0 works...";
   
   quiet=test_quiet();

   if (!quiet) printf("This tests if PERF_IOC_REFRESH with 0 works.\n");
   
   memset(&sa, 0, sizeof(struct sigaction));
   sa.sa_sigaction = our_handler;
   sa.sa_flags = SA_SIGINFO;

   if (sigaction( SIGIO, &sa, NULL) < 0) {
     fprintf(stderr,"Error setting up signal handler\n");
     exit(1);
   }
   
   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.sample_period=100000;
   pe.sample_type=PERF_SAMPLE_IP;
   pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
   pe.disabled=1;
   pe.pinned=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;
   pe.wakeup_events=1;

   arch_adjust_domain(&pe,quiet);

   fd1=perf_event_open(&pe,0,-1,-1,0);
   if (fd1<0) {
      if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
      test_fail(test_string);
   }

   /* needed even if we don't access it */

   blargh=mmap(NULL, (1+1)*4096, 
         PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

   
   fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
   fcntl(fd1, F_SETSIG, SIGIO);
   fcntl(fd1, F_SETOWN,getpid());
   
   ioctl(fd1, PERF_EVENT_IOC_RESET, 0);   

   ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH, 1);

   if (ret<0) {
     if (!quiet) fprintf(stderr,"Error with PERF_EVENT_IOC_REFRESH of group leader: "
	     "%d %s\n",errno,strerror(errno));
     exit(1);
   }

   naive_matrix_multiply(quiet);
   
   ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
    
   if (!quiet) printf("Count: %d %p\n",count,blargh);

   if (count==1) {
     if (!quiet) fprintf(stderr,"Only counted one overflow.\n");
     test_fail(test_string);     
   }
   else if (count>1) {
     test_caution(test_string);
   }
   else {
      if (!quiet) printf("No overflow events generated.\n");
      test_fail(test_string);
   }
   
   return 0;
}

