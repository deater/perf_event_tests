/* This tests what happens if multiple independent perf_event */
/* instances are started in the same thread.                  */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu                */

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

char test_string[]="Testing multiple simultaneous measurements...";

#define MAX_SIMULTANEOUS 10

int main(int argc, char **argv) {
   
  int ret,quiet,i,j;
   struct perf_event_attr pe[MAX_SIMULTANEOUS];
   int fd[MAX_SIMULTANEOUS];
   long long insn_count[MAX_SIMULTANEOUS];
   
   quiet=test_quiet();

   for(i=0;i<=MAX_SIMULTANEOUS;i++) {
   
      if (!quiet) {
	printf("Now checking if %d measurements\n",i);
      }

      for(j=0;j<i;j++) {
         memset(&pe[j],0,sizeof(struct perf_event_attr));
         pe[j].type=PERF_TYPE_HARDWARE;
         pe[j].size=sizeof(struct perf_event_attr);
         pe[j].config=PERF_COUNT_HW_INSTRUCTIONS;
         pe[j].disabled=1;
         pe[j].exclude_kernel=1;
         pe[j].exclude_hv=1;

         fd[j]=perf_event_open(&pe[j],0,-1,-1,0);
         if (fd[j]<0) {
            fprintf(stderr,"Error opening event %d\n",j);
            test_fail(test_string);
	 }
      }

      for(j=0;j<i;j++) {
         ret=ioctl(fd[j], PERF_EVENT_IOC_ENABLE,0);
         if (ret<0) {
	   if (!quiet) fprintf(stderr,"Error enabling event %d\n",j);
           test_fail(test_string);
	 }
      }

      naive_matrix_multiply(quiet);

      for(j=0;j<i;j++) {
         ret=ioctl(fd[j], PERF_EVENT_IOC_DISABLE,0);
         if (ret<0) {
	   if (!quiet) fprintf(stderr,"Error disabling event %d\n",j);
           test_fail(test_string);
	 }
      }
   
      for(j=0;j<i;j++) {
         ret=read(fd[j],&insn_count[j],sizeof(long long));
         if (ret!=sizeof(long long)) {
            if (!quiet) fprintf(stderr,
                                "Unepxected result from read fd[%d]: %d\n",
				j,ret);
            test_fail(test_string);      
	 }
      }
   
      if (!quiet) {
	 for(j=0;j<i;j++) printf("\tinsn_count[%d]: %lld\n",j,insn_count[j]);
      }

      for(j=0;j<i;j++) close(fd[j]);
   }

   test_pass( test_string );

   return 0;
}


