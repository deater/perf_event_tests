/* read_group_at_once.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

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

int main(int argc, char** argv) {
   
   int ret,fd1,fd2,quiet,i;
   int result;

   struct perf_event_attr pe;

   char test_string[]="Testing if all siblings can be read from group leader...";
   
   quiet=test_quiet();
   
   if (!quiet) {
      printf("Before 2.6.34 you could not read all sibling counts\n");
      printf("  from the group leader by specifying FORMAT_GROUP\n");
   }
   
   /* set up group leader */
   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
   pe.disabled=1;
   pe.pinned=0;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd1=perf_event_open(&pe,0,-1,-1,0);
   if (fd1<0) {
      if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
      test_fail(test_string);
   }

   /* setup event 2 */
   memset(&pe,0,sizeof(struct perf_event_attr));   
   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);   
   pe.config=PERF_COUNT_HW_CPU_CYCLES;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd2=perf_event_open(&pe,0,-1,fd1,0);
   if (fd2<0) {
      if (!quiet) fprintf(stderr,"Error opening %llx\n",pe.config);
      test_fail(test_string);
   }

   ioctl(fd1, PERF_EVENT_IOC_RESET, 0);   
   ioctl(fd2, PERF_EVENT_IOC_RESET, 0);   

   /* enable counting */
   ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

   naive_matrix_multiply(quiet);

   /* disable counting */
   ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
   if (ret<0) {   
      if (!quiet) printf("Error disabling\n");
   }

   #define BUFFER_SIZE 32
   long long buffer[BUFFER_SIZE];
   for(i=0;i<BUFFER_SIZE;i++) {
      buffer[i]=-1;
   }
     
   result=read(fd1,buffer,BUFFER_SIZE*sizeof(long long));
   if (result<0) {
      fprintf(stderr,"Unexpected read result %d\n",result);
      test_kernel_fail(test_string);
   }
   
   /* should be 1 + 2*num_events */
   /* which is 5 in our case     */
   if (result!=(5)*sizeof(long long)) {
      fprintf(stderr,"Unexpected read result %d\n",result);
      test_kernel_fail(test_string);
   }
   
   if (!quiet) {
      printf("Number of events: %lld\n",buffer[0]);
      for(i=0;i<buffer[0];i++) { 
         printf("Value    [%d] : %lld\n",i,buffer[1+(i*2)]);
         printf("Format ID[%d] : %lld\n",i,buffer[1+((i*2)+1)]);
      }
   }
      
   test_kernel_pass(test_string);
      
   return 0;
}

