/* signal_after_exec.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Compile with gcc -O2 -Wall -o signal_after_exec signal_after_exec.c */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <signal.h>

#include <sys/mman.h>

#include <sys/ioctl.h>

#include <asm/unistd.h>
#include <sys/prctl.h>
#include "perf_event.h"

int perf_event_open(struct perf_event_attr *hw_event_uptr,
                    pid_t pid, int cpu, int group_fd, unsigned long flags) {
   
  return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		 group_fd, flags);
}


static int count=0;

static void our_handler(int signum,siginfo_t *oh, void *blah) {
  count++;
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
   
  int fd1,pid,ret;
  double result;

   struct perf_event_attr pe;
   struct sigaction sa;

   if (argc>1) {
      result=busywork(10000000);
      printf("Count in execed=%d\n",count);
      exit(0);
   }

   /* set up signal handler */
   memset(&sa, 0, sizeof(struct sigaction));
   sa.sa_sigaction = our_handler;
   sa.sa_flags = SA_SIGINFO;

   if (sigaction( SIGIO, &sa, NULL) < 0) {
     fprintf(stderr,"Error setting up signal handler\n");
     exit(1);
   }

   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_HARDWARE;
   pe.config=PERF_COUNT_HW_CPU_CYCLES;
   pe.sample_period=10000;
   pe.sample_type=PERF_SAMPLE_IP;
   pe.read_format=0;
   pe.wakeup_events=1;

   pe.disabled=1;
   pe.inherit=1;
   
   fd1=perf_event_open(&pe,0,-1,-1,0);
   if (fd1<0) {
      fprintf(stderr,"Error opening\n");
      exit(1);
   }

   void *blargh;
   
   blargh=mmap(NULL, (1+1)*4096, 
	       PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);


   /* setup event 2 to have overflow signals */
   fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
   fcntl(fd1, F_SETSIG, SIGIO);
   fcntl(fd1, F_SETOWN,getpid());
   
   ioctl(fd1, PERF_EVENT_IOC_RESET, 0);   

   /* enable counting */
   ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
      
   pid=fork();
   
   if (pid!=0) {
     /* in the parent */

     /* exec ourselves, but call a busy function */
     execl(argv[0],argv[0],"busy",NULL);

   }
   result=busywork(10000000);

   printf("Count in child=%d\n",count);

   return 0;
}
