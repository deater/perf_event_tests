/* This file attempts to test the retired branches instruction */
/* performance counter on various architectures, as            */
/* implemented by the perf_events generalized event            */
/*    branches                                                 */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu                 */

char test_string[]="Testing \"branches\" generalized event...";
int quiet=0;
int fd;


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <asm/unistd.h>

#include "perf_event.h"
#include "test_utils.h"

#include "branches_test.h"
#include "helper.h"

int perf_event_open(struct perf_event_attr *hw_event_uptr,
                    pid_t pid, int cpu, int group_fd, unsigned long flags) {
 
  return syscall(__NR_perf_event_open, hw_event_uptr, pid, cpu, group_fd,flags);
}


int main(int argc, char **argv) {
   
   int num_runs=100;
   long long high=0,low=0,average=0,expected=1500000;
   double error;
   struct perf_event_attr pe;

   quiet=test_quiet();

   if (!quiet) {
      printf("\n");   

      printf("Testing a loop with %lld branches (%d times):\n",
          expected,num_runs);
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     fprintf(stderr,"Error opening leader %llx\n",pe.config);
     test_fail(test_string);
   }


   average=branches_test(fd, num_runs, &high,&low, quiet, test_string);

   error=handle_result(average,high,low,expected,quiet);

   if ((error > 1.0) || (error<-1.0)) {
     if (!quiet) printf("Instruction count off by more than 1%%\n");
     test_fail(test_string);
   }
   if (!quiet) printf("\n");

   test_pass( test_string );
   
   return 0;
}
