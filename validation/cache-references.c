/* This file attempts to test the L1 data cache references     */
/* performance counter on various architectures, as            */
/* implemented by the perf_events generalized event            */
/*    cache-references                                         */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu                 */

/* We assume "cache-references" means L1 Data Cache Refereneces */
/* Sometimes the C compiler doesn't help us, we should use asm  */

char test_string[]="Testing \"cache-references\" generalized event...";
int quiet=0;
int fd;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"

#define ARRAYSIZE 1000

int main(int argc, char **argv) {
   
  int num_runs=100,i,read_result;
   long long high=0,low=0,average=0;
   double error;
   struct perf_event_attr pe;
   
   long long count,total=0;
     
   quiet=test_quiet();

   if (!quiet) {
      printf("\n");   

      printf("Testing a loop writing %d doubles %d times\n",
          ARRAYSIZE,num_runs);
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   
   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_CACHE_REFERENCES;
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     fprintf(stderr,"Error opening leader %llx\n",pe.config);
     test_fail(test_string);
   }

   for(i=0;i<num_runs;i++) {
 
      /*******************************************************************/
      /* Test if the C compiler uses a sane number of data cache acceess */
      /*******************************************************************/



      double array[ARRAYSIZE];
      double aSumm = 0.0;

      if (!quiet) printf("Write test:\n");

      ioctl(fd, PERF_EVENT_IOC_RESET, 0);
      ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);

   
      for(i=0; i<ARRAYSIZE; i++) { 
	array[i]=(double)i;
      }
     
      ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
      read_result=read(fd,&count,sizeof(long long));

      for(i=0; i<ARRAYSIZE; i++) { 
	aSumm+=array[i];
      }


      if (!quiet) printf("\tL1 D accesseses: %lld %lf\n",count,aSumm);


      //      if (result==CODE_UNIMPLEMENTED) {
      //	 test_skip(test_string);
      //	 fprintf(stdout,"\tNo test code for this architecture\n");
      // }
      
      if (read_result!=sizeof(long long)) {
	 test_fail(test_string);
         fprintf(stdout,"Error extra data in read %d\n",read_result);	 
      }
      
      if (count>high) high=count;
      if ((low==0) || (count<low)) low=count;
      total+=count;
   }

   average=(total/num_runs);

   error=display_error(average,high,low,ARRAYSIZE,quiet);

   if ((error > 1.0) || (error<-1.0)) {
     if (!quiet) printf("Instruction count off by more than 1%%\n");
     test_unexplained(test_string);
   }
   if (!quiet) printf("\n");

   test_pass( test_string );
   
   return 0;
}
