/* overflow_required_mmap.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Compile with gcc -O2 -Wall -o overflow_required_mmap overflow_required_mmap.c */

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
#include <linux/perf_event.h>



static int count=0;

static void our_handler(int signum,siginfo_t *oh, void *blah) {
  count++;
}

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


int main(int argc, char** argv) {
   
  int ret,fd1,fd2;   
  double result;

   struct perf_event_attr pe;

   struct sigaction sa;

   printf("Before 2.6.40 you'd get no overflows on counters\n");
   printf(" if they didn't have an associated mmap'd ring buffer\n");
   
   memset(&sa, 0, sizeof(struct sigaction));
   sa.sa_sigaction = our_handler;
   sa.sa_flags = SA_SIGINFO;

   if (sigaction( SIGIO, &sa, NULL) < 0) {
     fprintf(stderr,"Error setting up signal handler\n");
     exit(1);
   }
   
   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.sample_period=000000;
   pe.sample_type=0;
   pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
   pe.disabled=1;
   pe.pinned=0;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;
   pe.wakeup_events=1;

   fd1=perf_event_open(&pe,0,-1,-1,0);
   if (fd1<0) {
      fprintf(stderr,"Error opening leader %llx\n",pe.config);
      exit(1);
   }

   pe.type=PERF_TYPE_HARDWARE;
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.sample_period=1000000;
   pe.sample_type=PERF_SAMPLE_IP;
   pe.read_format=0;
   pe.disabled=0;
   pe.pinned=0;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;
   pe.wakeup_events=1;

   fd2=perf_event_open(&pe,0,-1,fd1,0);
   if (fd2<0) {
      fprintf(stderr,"Error opening %llx\n",pe.config);
      exit(1);
   }


   /* Before 2.6.40 you'd get no overflows w/o this mmap */
#if 0
     {
   void *blargh;
   
   blargh=mmap(NULL, (1+1)*4096, 
         PROT_READ|PROT_WRITE, MAP_SHARED, fd2, 0);
     }
#endif
   
   fcntl(fd2, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
   fcntl(fd2, F_SETSIG, SIGIO);
   fcntl(fd2, F_SETOWN,getpid());
   
   ioctl(fd1, PERF_EVENT_IOC_RESET, 0);   
   ioctl(fd2, PERF_EVENT_IOC_RESET, 0);   

   ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

   if (ret<0) {
     fprintf(stderr,"Error with PERF_EVENT_IOC_REFRESH of group leader: "
	     "%d %s\n",errno,strerror(errno));
     exit(1);
   }

   result=busywork(10000000);
   
   ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
      
   printf("Count: %d %lf\n",count,result);
   if (count>0) {
      printf("PASSED\n");
   }
   else {
      printf("FAILED\n");
   }
   
   return 0;
}

