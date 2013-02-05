/* sw_start_leader.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* Test if starting a group (by having the group leader enabled */
/* at event creation time) of mixed sw/hw events works if the   */
/* group leader is a software event.                            */
/* Andreas Hollmann reported this as a problem on the           */
/*    linux-perf-users list                                     */

/* Bug not present through at least 2.6.36 */
/* Bug exists in 2.6.38 through at least Linux 3.7 */

/* Jiri Olsa reports it was introduced in 2.6.37         */
/* With commit b04243ef7006cda301819f54ee7ce0a3632489e3  */
/* A patch has been sent which hopefully will get in 3.8 */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/ioctl.h>


#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"


#define EVENTS 3

#define READ_SIZE (EVENTS + 1)

int main(int argc, char** argv) {
   
   int fd[EVENTS],ret,quiet;
   int result;
   int read_result;
   long long count[READ_SIZE];
   int i;

   struct perf_event_attr pe[EVENTS];

   char test_string[]="Testing start with sw event group leader...";

   quiet=test_quiet();

   if (!quiet) {
      printf("Testing if starting a group works if the group leader "
	     "is a software event.\n");
      printf("This is known to be broken from 2.6.37 through 3.8\n");
   }

   
   /******************************/
   /* TEST 1:                    */
   /*         hw-event as leader */
   /******************************/
   
   /* setup instruction event, group leader */

   memset(&pe[0],0,sizeof(struct perf_event_attr));

   pe[0].type=PERF_TYPE_HARDWARE;
   pe[0].size=sizeof(struct perf_event_attr);
   pe[0].config=PERF_COUNT_HW_CPU_CYCLES;   
   //pe[0].disabled=1;
   pe[0].exclude_kernel=1;
   pe[0].read_format=PERF_FORMAT_GROUP;
   
   fd[0]=perf_event_open(&pe[0],0,-1,-1,0);
   if (fd[0]<0) {
      fprintf(stderr,"Error opening\n");
      test_fail(test_string);
      exit(1);
   }

   /* setup cycles event, group child */

   memset(&pe[1],0,sizeof(struct perf_event_attr));
   pe[1].type=PERF_TYPE_SOFTWARE;
   pe[1].size=sizeof(struct perf_event_attr);
   pe[1].config=PERF_COUNT_SW_CPU_CLOCK;
   pe[1].disabled=0;
   pe[1].exclude_kernel=1;
   
   fd[1]=perf_event_open(&pe[1],0,-1,fd[0],0);
   if (fd[1]<0) {
      fprintf(stderr,"Error opening\n");
      test_fail(test_string);
      exit(1);
   }
   
      /* setup cycles event, group child */

   memset(&pe[2],0,sizeof(struct perf_event_attr));

   pe[2].type=PERF_TYPE_SOFTWARE;
   pe[2].size=sizeof(struct perf_event_attr);
   pe[2].config=PERF_COUNT_SW_TASK_CLOCK;
   pe[2].disabled=0;
   pe[2].exclude_kernel=1;
   
   fd[2]=perf_event_open(&pe[2],0,-1,fd[0],0);
   if (fd[2]<0) {
      fprintf(stderr,"Error opening\n");
      test_fail(test_string);
      exit(1);
   }

//   ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
//   ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

   result=instructions_million();
   if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

   ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

   read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
   if (read_result!=sizeof(long long)*READ_SIZE) {
      printf("Unexpected read size\n");
   }

   if (!quiet) {
      printf("Run with HW leader\n");
      for(i=0;i<count[0];i++) {
	 printf("\t%i Counted %lld\n",i,count[1+i]);
      }
   }

   for(i=0;i<EVENTS;i++) {
      if (count[i]==0) {
         if (!quiet) fprintf(stderr,"Counter %d did not start as expected\n",i);
         test_fail(test_string);
      }   
   }
   
   for(i=0;i<EVENTS;i++) {
      close(fd[i]);
   }
   
   /******************************/
   /* TEST 2:                    */
   /*         sw-event as leader */
   /******************************/
   
   /* setup instruction event, group leader */

   memset(&pe[0],0,sizeof(struct perf_event_attr));

   pe[0].type=PERF_TYPE_SOFTWARE;
   pe[0].size=sizeof(struct perf_event_attr);
   pe[0].config=PERF_COUNT_SW_CPU_CLOCK ;
   //pe[0].disabled=1;
   pe[0].exclude_kernel=1;
   pe[0].read_format=PERF_FORMAT_GROUP;
   
   fd[0]=perf_event_open(&pe[0],0,-1,-1,0);
   if (fd[0]<0) {
      fprintf(stderr,"Error opening\n");
      test_fail(test_string);
      exit(1);
   }

   /* setup cycles event, group child */

   memset(&pe[1],0,sizeof(struct perf_event_attr));

   pe[1].type=PERF_TYPE_SOFTWARE;
   pe[1].size=sizeof(struct perf_event_attr);
   pe[1].config=PERF_COUNT_SW_TASK_CLOCK;
   pe[1].disabled=0;
   pe[1].exclude_kernel=1;
   
   fd[1]=perf_event_open(&pe[1],0,-1,fd[0],0);
   if (fd[1]<0) {
      fprintf(stderr,"Error opening\n");
      test_fail(test_string);
      exit(1);
   }
   
      /* setup cycles event, group child */

   memset(&pe[2],0,sizeof(struct perf_event_attr));

   pe[2].type=PERF_TYPE_HARDWARE;
   pe[2].size=sizeof(struct perf_event_attr);
   pe[2].config=PERF_COUNT_HW_CPU_CYCLES;
   pe[2].disabled=0;
   pe[2].exclude_kernel=1;
   
   fd[2]=perf_event_open(&pe[2],0,-1,fd[0],0);
   if (fd[2]<0) {
      fprintf(stderr,"Error opening\n");
      test_fail(test_string);
      exit(1);
   }

//   ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
//   ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

   result=instructions_million();
   if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

   ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

   read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
   if (read_result!=sizeof(long long)*READ_SIZE) {
      printf("Unexpected read size\n");
   }

   if (!quiet) {
      printf("Second run, with SW group leader\n");
      for(i=0;i<count[0];i++) {
	 printf("\t%i Counted %lld\n",i,count[1+i]);
      }
   }

   read_result=read(fd[0],&count,sizeof(long long)*READ_SIZE);
   if (read_result!=sizeof(long long)*READ_SIZE) {
      printf("Unexpected read size\n");
   }

   for(i=0;i<EVENTS;i++) {
      if (count[i]==0) {
         if (!quiet) fprintf(stderr,"Counter %d did not start as expected\n",i);
         test_fail(test_string);
      }   
   }
   

   for(i=0;i<EVENTS;i++) {
      close(fd[i]);
   }

   (void) ret;

   test_pass(test_string);

   return 0;
}
