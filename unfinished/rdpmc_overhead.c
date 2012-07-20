/* by Vince Weaver, vweaver1@eecs.utk.edu                        */
/* Compile with gcc -O2 -o rdpmc_overhead rdpmc_overhead.c       */
/* You need at least a 3.4 kernel for this to run.               */
/* Ideally you'll turn off NMI watchdog too.                     */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/utsname.h>

#include <time.h>

#include "perf_event.h"
#include <unistd.h>
#include <asm/unistd.h>

#include <sys/ioctl.h>
#include <errno.h>

#include <sys/mman.h>

#ifndef __NR_perf_event_open

#if defined(__i386__)
#define __NR_perf_event_open    336
#elif defined(__x86_64__) 
#define __NR_perf_event_open    298
#elif defined __powerpc__
#define __NR_perf_event_open    319
#elif defined __arm__
#define __NR_perf_event_open    364
#endif
#endif


int perf_event_open(struct perf_event_attr *hw_event_uptr,
                    pid_t pid, int cpu, int group_fd, unsigned long flags) {
   
  return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
                 group_fd, flags);
}

#define MAX_EVENTS 16

int core2_events[MAX_EVENTS]={
  0x53003c, //"UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, //"INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5300c4, //"BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5300c5, //"MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x531282, //"ITLB:MISSES",                  /* PAPI_TLB_IM  */
  0x530108, //"DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  0x530080, //"L1I_READS",                    /* PAPI_L1_ICA  */
  0x530081, //"L1I_MISSES",                   /* PAPI_L1_ICM  */
  0x530143, //"L1D_ALL_REF",                  /* PAPI_L1_DCA  */
  0x530f45, //"L1D_REPL",                     /* PAPI_L1_DCM  */
  0x5300c8, //"HW_INT_RCV",                   /* PAPI_HW_INT  */
  0x530010, //"FP_COMP_OPS_EXE",              /* PAPI_FP_OPS  */
  0x5301c0, //"INST_RETIRED:LOADS",           /* PAPI_LD_INS  */
  0x5302c0, //"INST_RETIRED:STORES",          /* PAPI_SR_INS  */
  0x537f2e, //"L2_RQSTS:SELF:ANY:MESI",       /* PAPI_L2_TCA  */
  0x537024, //"L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

int atom_events[MAX_EVENTS]={
  0x53003c, //"UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5300c4, // "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5300c5, // "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x530282, // "ITLB:MISSES",                  /* PAPI_TLB_IM  */
  0x530708, // "DATA_TLB_MISSES:DTLB_MISS",    /* PAPI_TLB_DM  */
  0x530380, //"ICACHE:ACCESSES",              /* PAPI_L1_ICA  */
  0x530280, // "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  0x532140, // "L1D_CACHE:LD",                 /* PAPI_L1_DCA  */
  0x537f2e, // "L2_RQSTS:SELF",                /* PAPI_L1_DCM  */
  0x5301c8, //"HW_INT_RCV",                   /* PAPI_HW_INT  */
  0x531fc7, // "SIMD_INST_RETIRED:ANY",        /* PAPI_FP_OPS  */
  0x537f29, //"L2_LD:SELF:ANY:MESI",          /* PAPI_LD_INS  */
  0x534f2a, //"L2_ST:SELF:MESI",              /* PAPI_SR_INS  */
  0x537f29, //"L2_LD:SELF:ANY:MESI",          /* PAPI_L2_TCA  */
  0x537024, //"L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

int amd10h_events[MAX_EVENTS]={
  0x530076, // "CPU_CLK_UNHALTED",                         /* PAPI_TOT_CYC */
  0x5300c0, // "RETIRED_INSTRUCTIONS",                     /* PAPI_TOT_INS */
  0x5300c2, // "RETIRED_BRANCH_INSTRUCTIONS",              /* PAPI_BR_INS  */
  0x5300c3, // "RETIRED_MISPREDICTED_BRANCH_INSTRUCTIONS", /* PAPI_BR_MSP  */
  0x530385, // "L1_ITLB_MISS_AND_L2_ITLB_MISS:ALL",        /* PAPI_TLB_IM  */
  0x530746, // "L1_DTLB_AND_L2_DTLB_MISS",                 /* PAPI_TLB_DM  */
  0x530080, // "INSTRUCTION_CACHE_FETCHES",                /* PAPI_L1_ICA  */
  0x530081, // "INSTRUCTION_CACHE_MISSES",                 /* PAPI_L1_ICM  */
  0x530040, // "DATA_CACHE_ACCESSES",                      /* PAPI_L1_DCA  */
  0x530041, // "DATA_CACHE_MISSES",                        /* PAPI_L1_DCM  */
  0x5300cf, // "INTERRUPTS_TAKEN",                         /* PAPI_HW_INT  */
  0x530300, // "DISPATCHED_FPU:OPS_MULTIPLY:OPS_ADD",      /* PAPI_FP_OPS  */
  0x5300d0, // "DECODER_EMPTY",                            /* PAPI_LD_INS  */ /* nope */
  0x5300d1, // "DISPATCH_STALLS",                          /* PAPI_SR_INS  */ /* nope */
  0x533f7d, // "REQUESTS_TO_L2:ALL",                       /* PAPI_L2_TCA  */
  0x53037e, // "L2_CACHE_MISS:INSTRUCTIONS:DATA",          /* PAPI_L2_TCM  */
};

int nehalem_events[MAX_EVENTS]={
  0x53003c, // "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x537f88, // "BR_INST_EXEC:ANY",             /* PAPI_BR_INS  */
  0x537f89, // "BR_MISP_EXEC:ANY",             /* PAPI_BR_MSP  */
  0x530185, // "ITLB_MISSES:ANY",              /* PAPI_TLB_IM  */
  0x530149, // "DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  0x530380, // "L1I:READS",                    /* PAPI_L1_ICA  */
  0x530280, // "L1I:MISSES",                   /* PAPI_L1_ICM  */
  0x530143, // "L1D_ALL_REF:ANY",              /* PAPI_L1_DCA  */
  0x530151, // "L1D:REPL",                     /* PAPI_L1_DCM  */
  0x53011d, // "HW_INT:RCV",                   /* PAPI_HW_INT  */
  0x530410, // "FP_COMP_OPS_EXE:SSE_FP",       /* PAPI_FP_OPS  */
  0x53010b, // "MEM_INST_RETIRED:LOADS",       /* PAPI_LD_INS  */
  0x53020b, // "MEM_INST_RETIRED:STORES",      /* PAPI_SR_INS  */
  0x53ff24, // "L2_RQSTS:REFERENCES",          /* PAPI_L2_TCA  */
  0x53c024, // "L2_RQSTS:PREFETCHES",          /* PAPI_L2_TCM  */
};


int events[MAX_EVENTS];

static unsigned long long rdtsc(void) {
  unsigned a,d;

  __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

  return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}

static unsigned long long rdpmc(unsigned int counter) {
  unsigned int low, high;

  __asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

  return (unsigned long long)low | ((unsigned long long)high) <<32;
}

#define barrier() __asm__ volatile("" ::: "memory")


/* Ingo's code for using rdpmc */
static inline unsigned long long mmap_read_self(void *addr) {
  
    struct perf_event_mmap_page *pc = addr;
    unsigned int seq,idx;

    unsigned long long count;

    do {
      seq=pc->lock;
      barrier();

      idx=pc->index;
      count=pc->offset;

      if (idx) {
	count+=rdpmc(pc->index-1);
      }
      barrier();
    } while (pc->lock != seq);
  

  return count;
}


int test_rdpmc(int count) {

   int i;
   static long page_size=4096;
   
   long long before,after;
   long long start_before,start_after;
   long long stop_before,stop_after;
   long long read_before,read_after;

   long long init_ns=0,eventset_ns=0;
   void *addr[MAX_EVENTS];

   struct perf_event_attr pe;
   int fd[MAX_EVENTS],ret1,ret2;

   unsigned long long now[MAX_EVENTS],stamp[MAX_EVENTS];

   /************************/
   /* measure init latency */
   /************************/

   before=rdtsc();

   after=rdtsc();

   init_ns=after-before;

   /*****************************/
   /* measure eventset creation */
   /*****************************/

   before=rdtsc();

   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_RAW;
   pe.size=sizeof(struct perf_event_attr);
   pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;

   fd[0]=-1;

   for(i=0;i<count;i++) {
     pe.config=events[i];

     if (i==0) {
        pe.disabled=1;
        pe.pinned=1;
     }
     else {
        pe.disabled=0;
        pe.pinned=0;
     }

     fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
     if (fd[i]<0) {
        printf("error perf_event_opening\n");
	exit(1);
      }
   
      addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
      if (addr[i] == (void *)(-1)) {
        printf("Error mmaping!\n");
        exit(1);
      }
   }

   after=rdtsc();

   eventset_ns=after-before;

   /*********/
   /* start */
   /*********/
   start_before=rdtsc();

   ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

   start_after=rdtsc();

   /********/
   /* read */
   /********/

   /* read before values */
   for(i=0;i<count;i++) {
     stamp[i] = mmap_read_self(addr[i]);
   }

   /* NULL */

   /* read after values */
   for(i=0;i<count;i++) {
     now[i] = mmap_read_self(addr[i]);
   }

   read_after=rdtsc();
   
   /********/
   /* stop */
   /********/

   ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

   stop_after=rdtsc();

   /* ALL DONE */

   stop_before=read_after;
   read_before=start_after;

   printf("init/create/start/stop/read: %lld,%lld,%lld,%lld,%lld\n",
          init_ns,eventset_ns, 
          start_after-start_before,
          stop_after-stop_before,
          read_after-read_before);

   if (ret1<0) {
      printf("Error starting!\n");
      exit(1);
   }
   
   if (ret2<0) {
      printf("Error stopping!\n");
      exit(1);
   }
   
   
   for(i=0;i<count;i++) {
     printf("%x %lld\n",
	    events[i],now[i]-stamp[i]);
   }

   for(i=0;i<count;i++) {
      munmap(addr[i],page_size);
   }

   for(i=0;i<count;i++) {
      close(fd[i]);
   }

   return read_after-read_before;
}

int test_read(int count) {

   int i;
   static long page_size=4096;
   
   long long before,after;
   long long start_before,start_after;
   long long stop_before,stop_after;
   long long read_before,read_after;

   long long init_ns=0,eventset_ns=0;
   void *addr[MAX_EVENTS];

   struct perf_event_attr pe;
   int fd[MAX_EVENTS],ret1,ret2;

   /************************/
   /* measure init latency */
   /************************/

   before=rdtsc();

   after=rdtsc();

   init_ns=after-before;

   /*****************************/
   /* measure eventset creation */
   /*****************************/

   before=rdtsc();

   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_RAW;
   pe.size=sizeof(struct perf_event_attr);
   pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;

   fd[0]=-1;

   for(i=0;i<count;i++) {
     pe.config=events[i];

     if (i==0) {
        pe.disabled=1;
        pe.pinned=1;
     }
     else {
        pe.disabled=0;
        pe.pinned=0;
     }

     fd[i]=perf_event_open(&pe,0,-1,fd[0],0);
     if (fd[i]<0) {
        printf("error perf_event_opening\n");
	exit(1);
      }
   
      addr[i]=mmap(NULL,page_size, PROT_READ, MAP_SHARED,fd[i],0);
      if (addr[i] == (void *)(-1)) {
        printf("Error mmaping!\n");
        exit(1);
      }
   }

   after=rdtsc();

   eventset_ns=after-before;

   /*********/
   /* start */
   /*********/
   start_before=rdtsc();

   ret1=ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

   start_after=rdtsc();

   /* NULL */

   /********/
   /* read */
   /********/

   #define BUFFER_SIZE 256
   long long buffer[BUFFER_SIZE];

   read(fd[0],buffer,BUFFER_SIZE*sizeof(long long));

   read_after=rdtsc();
   
   /********/
   /* stop */
   /********/

   ret2=ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

   stop_after=rdtsc();

   /* ALL DONE */

   stop_before=read_after;
   read_before=start_after;

   printf("init/create/start/stop/read: %lld,%lld,%lld,%lld,%lld\n",
          init_ns,eventset_ns, 
          start_after-start_before,
          stop_after-stop_before,
          read_after-read_before);

   if (ret1<0) {
      printf("Error starting!\n");
      exit(1);
   }
   
   if (ret2<0) {
      printf("Error stopping!\n");
      exit(1);
   }
   
   
   for(i=0;i<count;i++) {
     printf("%x %lld\n",
	    events[i],buffer[1+(i*2)]);
   }

   for(i=0;i<count;i++) {
      munmap(addr[i],page_size);
   }

   for(i=0;i<count;i++) {
      close(fd[i]);
   }

   return read_after-read_before;
}

int main(int argc, char **argv) {


   char *machine_name;
   int i,j;

   /* Get machine type */

   if (argc>1) {
     machine_name=strdup(argv[1]);
   }
   else {
     machine_name=strdup("core2");
   }

   if (!strncmp(machine_name,"core2",5)) {
      memcpy(events,core2_events,MAX_EVENTS*sizeof(int));
   }
   else if (!strncmp(machine_name,"nehalem",7)) {
     memcpy(events,nehalem_events,MAX_EVENTS*sizeof(int));
   }
   else if (!strncmp(machine_name,"atom",4)) {
     memcpy(events,atom_events,MAX_EVENTS*sizeof(int));
   }
   else if (!strncmp(machine_name,"amd10h",6)) {
     memcpy(events,amd10h_events,MAX_EVENTS*sizeof(int));
   }
   else if (!strncmp(machine_name,"amd0fh",6)) {
     memcpy(events,amd10h_events,MAX_EVENTS*sizeof(int));
   }
   else {
      fprintf(stderr,"Unknown machine name %s\n",machine_name);
      exit(0);
   }

#define NUM_RUNS 100
#define NUM_COUNTS 5

   long long rdpmc_data[NUM_RUNS][NUM_COUNTS];
   long long read_data[NUM_RUNS][NUM_COUNTS];
   long long rdpmc_average[NUM_COUNTS];
   long long read_average[NUM_COUNTS];

   /* do rdpmc runs */
   for(i=0;i<NUM_RUNS;i++) {
     for(j=1;j<NUM_COUNTS;j++) {
        rdpmc_data[i][j]=test_rdpmc(j);
     }
   }

   /* do read runs */
   for(i=0;i<NUM_RUNS;i++) {
     for(j=1;j<NUM_COUNTS;j++) {
        read_data[i][j]=test_read(j);
     }
   }

   /* calculate averages */
   for(j=1;j<NUM_COUNTS;j++) {
      rdpmc_average[j]=0;
      read_average[j]=0;
      for(i=0;i<NUM_RUNS;i++) {
         rdpmc_average[j]+=rdpmc_data[i][j];
         read_average[j]+=read_data[i][j];
     }
     rdpmc_average[j]/=NUM_RUNS;
     read_average[j]/=NUM_RUNS;
   }

   /* print results */
   printf("\n\nAverage Read Overhead (Cycles)\n\n");
   printf("Events\trdpmc\tread\n");
   for(j=1;j<NUM_COUNTS;j++) {
     printf("%d\t%lld\t%lld\n",j,rdpmc_average[j],read_average[j]);
   }

   return 0;
}
