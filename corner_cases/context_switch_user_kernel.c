/* This tests if the context-switches event is user or kernel */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu                */

/* context-switches were reported as user only until kernel 2.6.34 */
/* e49a5bd38159dfb1928fd25b173bc9de4bbadb21                        */

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

char test_string[]="Testing if context-switches are kernel only...";

int main(int argc, char **argv) {
   
   int ret,quiet;
   struct perf_event_attr pe;
   int fd;
   long long user_count,kernel_count;
   
   quiet=test_quiet();

   /* user count */

   if (!quiet) {
      printf("First checking if context-switches recoded in user-only\n");
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_SOFTWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
      fprintf(stderr,"Error opening leader %llx\n",pe.config);
      test_fail(test_string);
   }
      
   ret=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error enabling events\n");
      test_fail(test_string);
   }
   
   naive_matrix_multiply(quiet);

   ret=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error disabling events\n");
      test_fail(test_string);
   }
   
   ret=read(fd,&user_count,sizeof(long long));
   if (ret!=sizeof(long long)) {
      if (!quiet) fprintf(stderr,"Unepxected result from read fd: %d\n",ret);
      test_fail(test_string);      
   }
   
   if (!quiet) {
      printf("\tuser_count: %lld\n\n",user_count);
   }
   close(fd);

   /* Kernel Count */

   if (!quiet) {
      printf("Now checking if context-switches reported as kernel-only\n");
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_SOFTWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_SW_CONTEXT_SWITCHES;
   pe.disabled=1;
   pe.exclude_kernel=0;
   pe.exclude_user=1;
   pe.exclude_hv=1;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
      fprintf(stderr,"Error opening leader %llx\n",pe.config);
      test_fail(test_string);
   }
      
   ret=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error enabling events\n");
      test_fail(test_string);
   }
   
   naive_matrix_multiply(quiet);

   ret=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error disabling events\n");
      test_fail(test_string);
   }
   
   ret=read(fd,&kernel_count,sizeof(long long));
   if (ret!=sizeof(long long)) {
      if (!quiet) fprintf(stderr,"Unepxected result from read fd: %d\n",ret);
      test_fail(test_string);      
   }
   
   if (!quiet) {
      printf("\tkernel_count: %lld\n\n",kernel_count);
   }
   close(fd);

   /* Sort out result */

   /* expected before 2.6.34 */
   if ((kernel_count==0) && (user_count!=0)) {
      test_yellow_no(test_string);
   }

   /* expected as of 2.6.34 */
   if ((kernel_count!=0) && (user_count==0)) {
      test_green_yes(test_string);
   }

   if (!quiet) {
     printf("Unexpected result, both user _and_ kernel!\n");
   }
   test_fail( test_string );

   return 0;
}


