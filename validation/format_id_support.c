/* format_id_support.c  */
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

   char test_string[]="Testing for FORMAT_ID support...";
   
   quiet=test_quiet();
   
   if (!quiet) {
      printf("Simple test of FORMAT_GROUP and FORMAT_ID.\n");
   }
   
   /* set up group leader */
   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.read_format=PERF_FORMAT_ID;
   pe.disabled=1;
   pe.pinned=0;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

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
   pe.read_format=PERF_FORMAT_ID;   
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

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
      test_fail(test_string);
   }
   
   /* should be 2 */
   if (result!=(2)*sizeof(long long)) {
      fprintf(stderr,"Unexpected read result %d\n",result);
      test_fail(test_string);
   }
   
   if (!quiet) {
         printf("Value    [%d] : %lld\n",0,buffer[0]);
         printf("Format ID[%d] : %lld\n",0,buffer[1]);
   }
   
   result=read(fd2,buffer,BUFFER_SIZE*sizeof(long long));
   if (result<0) {
      fprintf(stderr,"Unexpected read result %d\n",result);
      test_fail(test_string);
   }
   
   /* should be 2 */
   if (result!=(2)*sizeof(long long)) {
      fprintf(stderr,"Unexpected read result %d\n",result);
      test_fail(test_string);
   }
   
   if (!quiet) {
         printf("Value    [%d] : %lld\n",1,buffer[0]);
         printf("Format ID[%d] : %lld\n",1,buffer[1]);
   }   
     
   test_pass(test_string);
      
   return 0;
}

