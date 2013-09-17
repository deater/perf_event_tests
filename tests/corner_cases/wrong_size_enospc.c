/* wrong_size_enospc.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Until 3.x reading into a wrong-sized buffer    */
/*  returned the confusing -ENOSPC                */
/*  this might be changed in a forthcoming kernel */


#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"


int main(int argc, char** argv) {
   
   int ret,fd1,fd2,quiet,i;
   int result;

   struct perf_event_attr pe;

   char test_string[]="Testing if ENOSPC returned for wrong-sized buffer...";
   
   quiet=test_quiet();

   if (!quiet) {
      printf("\nBefore 3.x reading perf results into a too-small\n");
      printf("buffer results in a somewhat unusual ENOSPC error\n\n");
   }

   /* set up group leader */
   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
   pe.disabled=1;
   pe.pinned=1;
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

   /* disable counting */
   ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
   if (ret<0) {   
      if (!quiet) printf("Error disabling\n");
   }

   /* deliberately create a too-small buffer size */

   #define BUFFER_SIZE 1

   long long buffer[BUFFER_SIZE];
   for(i=0;i<BUFFER_SIZE;i++) {
      buffer[i]=-1;
   }
     
   result=read(fd1,buffer,BUFFER_SIZE*sizeof(long long));
   if (result<0) {
     if (!quiet) {
        fprintf(stderr,"Read result %d errno %d = %s\n\n",
		result,errno,strerror(errno));
     }
     if (errno==ENOSPC) {
        test_yellow_old_behavior(test_string);
     }
     else {
        test_green_new_behavior(test_string);
     }

     test_kernel_fail(test_string);
   }
         
   test_pass(test_string);
      
   return 0;
}

