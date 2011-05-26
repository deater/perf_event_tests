/* This file attempts to test the retired branches instruction */
/* performance counter on various architectures, as            */
/* implemented by the perf_events generalized event            */
/*    branches                                                 */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu                 */

/* prior to 2.6.35 the wrong branch event was used on AMD machines */


char test_string[]="Testing \"branches\" generalized event...";
int quiet=0;
int fd;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "branches_testcode.h"
#include "perf_helpers.h"


int main(int argc, char **argv) {
   
   int num_runs=100,i,read_result,result;
   long long high=0,low=0,average=0,expected=1500000;
   double error;
   struct perf_event_attr pe;
   
   long long count,total=0;
     
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

   for(i=0;i<num_runs;i++) {
      ioctl(fd, PERF_EVENT_IOC_RESET, 0);
      ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
 
      result=branches_testcode();
      
      ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
      read_result=read(fd,&count,sizeof(long long));
      
      if (result==CODE_UNIMPLEMENTED) {
	 test_skip(test_string);
	 fprintf(stdout,"\tNo test code for this architecture\n");
      }
      
      if (read_result!=sizeof(long long)) {
	 test_fail(test_string);
         fprintf(stdout,"Error extra data in read %d\n",read_result);	 
      }
      
      if (count>high) high=count;
      if ((low==0) || (count<low)) low=count;
      total+=count;
   }

   average=(total/num_runs);

   error=display_error(average,high,low,expected,quiet);

   if ((error > 1.0) || (error<-1.0)) {
     if (!quiet) printf("Instruction count off by more than 1%%\n");
     test_fail(test_string);
   }
   if (!quiet) printf("\n");

   test_pass( test_string );
   
   return 0;
}
