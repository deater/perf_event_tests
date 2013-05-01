/* This tests if event constraints are enforced       */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */

/* x86 constraints not implemented until 2.6.33 */
/* b690081d4d3f6a23541493f1682835c3cd5c54a1     */
/* perf_events: Add event constraints support for Intel processors */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "matrix_multiply.h"

char test_string[]="Testing if event constraints are enforced...";

#define MAX_EVENTS 8

int main(int argc, char **argv) {
   
   int ret,quiet,cpu,i;
   struct perf_event_attr pe;
   int fd[MAX_EVENTS];
   long long counts[2];
   long long events[MAX_EVENTS];
   int num_events=0;
   
   for(i=0;i<MAX_EVENTS;i++) events[i]=0;
   
   quiet=test_quiet();
   
   cpu=detect_processor();
   
   if ( (cpu==PROCESSOR_CORE2) || 
	(cpu==PROCESSOR_PENTIUM_PRO) ||
	(cpu==PROCESSOR_PENTIUM_II)  ||
	(cpu==PROCESSOR_PENTIUM_III) ||
	(cpu==PROCESSOR_PENTIUM_M)) {
      if (!quiet) printf("Found Core2/p6!\n");
      events[0]=0x530012;  /* mul 1 only */
      events[1]=0x530013;  /* div 1 only */
      //events[1]=0x530010;  /* fp_comp_ops_exe 0 only */
      num_events=2;
   }
   else if (cpu==PROCESSOR_COREDUO) {
      if (!quiet) printf("Found CoreDuo!\n");
      events[0]=0x530012;  /* mul 1 only */
      events[1]=0x530013;  /* div 1 only */      
      num_events=2;
   }
   else if ((cpu==PROCESSOR_NEHALEM) || (cpu==PROCESSOR_NEHALEM_EX)) {
      if (!quiet) printf("Found Nehalem!\n");
      events[0]=0x530151;  /* L1D:REPL 0,1 only */      
      events[1]=0x530251;  /* L1D:M_REPL 0,1 only */      
      events[2]=0x530451;  /* L1D:M_EVICT 0,1 only */
      num_events=3;
   }
   else if ((cpu==PROCESSOR_WESTMERE) || (cpu==PROCESSOR_WESTMERE_EX)) {
      if (!quiet) printf("Found Westmere!\n");
      events[0]=0x530151;  /* L1D:REPL 0,1 only */
      events[1]=0x530251;  /* L1D:M_REPL 0,1 only */     
      events[2]=0x530451;  /* L1D:M_EVICT 0,1 only */
      num_events=3;
   }
   else if (cpu==PROCESSOR_SANDYBRIDGE) {
      if (!quiet) printf("Found Sandybridge!\n");      
      test_needtest(test_string);
   } 
   else if (cpu==PROCESSOR_SANDYBRIDGE_EP) {
      if (!quiet) printf("Found Sandybridge EP!\n");      
      test_needtest(test_string);
   } 
   else if (cpu==PROCESSOR_IVYBRIDGE) {
      if (!quiet) printf("Found Sandybridge!\n");      
      events[0]=0x530148; /* L1D_PEND_MISS:PENDING counter 2 only */
      events[1]=0x85308a3; /* CYCLE_ACTIVITY:CYCLES_L1D_PENDING counter 2 only */
      events[2]=0xc530ca3; /* CYCLE_ACTIVITY:STALLS_L1D_PENDING counter 2 only */
      num_events=3; 
   } 
   else if (cpu==PROCESSOR_AMD_FAM15H) {
      if (!quiet) printf("Found AMD Family 15h\n");
      test_needtest(test_string);
   }
   else {
     if (!quiet) printf("No known event constraints on this architecture\n");
     test_skip(test_string);
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_RAW;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=events[0];
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd[0]=perf_event_open(&pe,0,-1,-1,0);
   if (fd[0]<0) {
      fprintf(stderr,"Error opening leader %llx\n",pe.config);
      test_fail(test_string);
   }
   
   for(i=1;i<num_events;i++) {
      
      memset(&pe,0,sizeof(struct perf_event_attr));
      pe.size=sizeof(struct perf_event_attr);
      pe.type=PERF_TYPE_RAW;
      pe.config=events[i];
      pe.exclude_kernel=1;
      pe.exclude_hv=1;

      fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
      if (fd[i]<0) {
         if (!quiet) fprintf(stderr,"Error opening %llx %s\n",pe.config,strerror(errno));
         test_pass(test_string);
	 exit(1);
      }
   }
   
   ret=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error enabling events\n");
      test_fail(test_string);
   }
   
   naive_matrix_multiply(quiet);

   ret=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error disabling events\n");
      test_fail(test_string);
   }
   
   ret=read(fd[0],&counts[0],sizeof(long long));
   if (ret!=sizeof(long long)) {
      if (!quiet) fprintf(stderr,"Unepxected result from read fd[0]: %d\n",ret);
      test_fail(test_string);      
   }
   
   if (!quiet) {
      printf("Event[0]: %lld\n",counts[0]);
   }

   if (!quiet) {
      printf("Error! Constraints invalid, should not have let us measure!\n");
   }
      
   test_fail( test_string );

   return 0;
}


