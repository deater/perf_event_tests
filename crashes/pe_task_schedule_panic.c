/* pe_task_schedule_panic.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Running has a good chance of panicing unpatched kernel */
/* Based on behavior of mpx-pthread.c from perfsuite      */

/* Fixed by commit ab711fe08297de1485fff0a366e6db8828cafd6a */
/* Which is included in Linux 2.6.39                        */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>

#include "perf_event.h"
#include "perf_helpers.h"


double busywork(int count) {
 
   int i;
   double sum=0.0012;
   
   for(i=0;i<count;i++) {
      sum+=0.01;
   }
   return sum;
   
}


void *thread_work(void *blah) {
   
   int fd1,fd2;
   int ret,result;
   unsigned char buffer[16384];
   
   struct perf_event_attr pe;
   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_HARDWARE;
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.disabled=1;
   pe.inherit=0;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;
   pe.read_format=7;
      
   fd1=perf_event_open(&pe,0,-1,-1,0);
   if (fd1<0) {
     fprintf(stderr,"Error opening master\n");
   }
   /* read */
   result=read(fd1,buffer,16384);

   pe.type=PERF_TYPE_HARDWARE;
   pe.config=PERF_COUNT_HW_CPU_CYCLES;
   pe.disabled=0;
   pe.inherit=0;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;
   pe.read_format=7;

   fd2=perf_event_open(&pe,0,-1,fd1,0);
   if (fd2<0) {
     fprintf(stderr,"Error opening sub\n");
   }
   /* read */
   result=read(fd2,buffer,16384);

   busywork(10000000);
   
   ret=ioctl(fd1,PERF_EVENT_IOC_DISABLE,NULL);
   if (ret<0) {
      fprintf(stderr,"Error disabling fd %d\n",fd1);
   }

   close(fd2);
   close(fd1);

   (void) result;

   pthread_exit(NULL);
}



#define NUM_THREADS 16

int main(int argc, char** argv) {
   
   pthread_t *threads;
   int ret,i,j;   
   
   printf("\nOn unpatched kernels, running will eventually lock or \n");
   printf("\tpanic the system.\n\n");
   
   threads = (pthread_t *)malloc(NUM_THREADS*sizeof(pthread_t));
   if (threads==NULL) {
      fprintf(stderr,"Could not allocate memory\n");
      exit(1);
   }
   
   for(i=0;i<NUM_THREADS;i++) {
      
      ret=pthread_create(&threads[i],NULL,thread_work,NULL);	 
      if (ret!=0) {
	 fprintf(stderr,"Could not create thread %i\n",i);
	 break;
      }
   }
      
   for(j=0;j<i;j++) {
      ret=pthread_join(threads[j],NULL);	 
      if (ret!=0) {
	 fprintf(stderr,"Could not join thread %i\n",j);
      }
   }

   (void) ret;
   
   return 0;
}

