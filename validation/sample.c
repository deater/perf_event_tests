/* simple.c  */
/* by Vince Weaver   vincent.weaver _at_ maine.edu */

/* This just tests perf_event sampling */

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
#include "instructions_testcode.h"

#define SAMPLE_FREQUENCY 100000

int sample_type=PERF_SAMPLE_IP | PERF_SAMPLE_TID | PERF_SAMPLE_TIME |
                  PERF_SAMPLE_ADDR | PERF_SAMPLE_READ | PERF_SAMPLE_CALLCHAIN |
                  PERF_SAMPLE_ID | PERF_SAMPLE_CPU | PERF_SAMPLE_PERIOD |
                  PERF_SAMPLE_STREAM_ID | PERF_SAMPLE_RAW ;
   //                  PERF_SAMPLE_BRANCH_STACK;


int read_format=
                PERF_FORMAT_GROUP |
                PERF_FORMAT_ID |
                PERF_FORMAT_TOTAL_TIME_ENABLED |
                PERF_FORMAT_TOTAL_TIME_RUNNING;


static struct signal_counts {
  int in,out,msg,err,pri,hup,unknown,total;
} count = {0,0,0,0,0,0,0,0};

static int fd1,fd2;

void *our_mmap;


/* Urgh who designed this interface */
int handle_struct_read_format(unsigned char *sample, int read_format) {
  
  int offset=0,i;

  if (read_format & PERF_FORMAT_GROUP) {
     long long nr,time_enabled,time_running;

     memcpy(&nr,&sample[offset],sizeof(long long));
     printf("\t\tNumber: %lld ",nr);
     offset+=8;

     if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) {
        memcpy(&time_enabled,&sample[offset],sizeof(long long));
        printf("enabled: %lld ",time_enabled);
        offset+=8;
     }
     if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) {
        memcpy(&time_running,&sample[offset],sizeof(long long));
        printf("running: %lld ",time_running);
        offset+=8;
     }

     printf("\n");

     for(i=0;i<nr;i++) {
        long long value, id;
        memcpy(&value,&sample[offset],sizeof(long long));
        printf("\t\t\tValue: %lld ",value);
	offset+=8;

        if (read_format & PERF_FORMAT_ID) {
           memcpy(&id,&sample[offset],sizeof(long long));
           printf("id: %lld ",id);
           offset+=8;
	}

        printf("\n");
     }
  }
  else {
    long long value,time_enabled,time_running,id;
    memcpy(&value,&sample[offset],sizeof(long long));
    printf("\t\tValue: %lld ",value);
    offset+=8;

    if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) {
       memcpy(&time_enabled,&sample[offset],sizeof(long long));
       printf("enabled: %lld ",time_enabled);
       offset+=8;
    }
    if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) {
       memcpy(&time_running,&sample[offset],sizeof(long long));
       printf("running: %lld ",time_running);
       offset+=8;
    }
    if (read_format & PERF_FORMAT_ID) {
       memcpy(&id,&sample[offset],sizeof(long long));
       printf("id: %lld ",id);
       offset+=8;
    }
    printf("\n");
  }

  return offset;
}

int perf_mmap_read(void) {

   struct perf_event_mmap_page *control_page = our_mmap;
   int head,tail,offset;
   int i;

   unsigned char *data;

   if (control_page==NULL) {
      fprintf(stderr,"ERROR mmap page NULL\n");
      return -1;
   }
   
   head=control_page->data_head;
   tail=control_page->data_tail;

   printf("Head: %d %d\n",head,tail);
   rmb();

   /* data starts one page after control */
   data=((unsigned char*)our_mmap) + getpagesize(  );

   /*FIXME..crosspage boundary?*/

   struct perf_event_header *event;

   while(tail!=head) {

      event = ( struct perf_event_header * ) & data[tail];

      printf("Header: %d %d %d\n",event->type,event->misc,event->size);
      offset=tail+8; /* skip header */
      switch(event->type) {

         case PERF_RECORD_SAMPLE:
	   if (sample_type & PERF_SAMPLE_IP) {
	     long long ip;
	     memcpy(&ip,&data[offset],sizeof(long long));
	     printf("\tIP: %llx\n",ip);
	     offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_TID) {
	      int pid, tid;
              memcpy(&pid,&data[offset],sizeof(int));
              memcpy(&tid,&data[offset+4],sizeof(int));
	      printf("\tpid: %d  tid %d\n",pid,tid);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_TIME) {
	      long long time;
              memcpy(&time,&data[offset],sizeof(long long));
	      printf("\ttime: %lld\n",time);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_ADDR) {
	      long long addr;
              memcpy(&addr,&data[offset],sizeof(long long));
	      printf("\taddr: %llx\n",addr);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_ID) {
	      long long sample_id;
              memcpy(&sample_id,&data[offset],sizeof(long long));
	      printf("\tsample_id: %lld\n",sample_id);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_STREAM_ID) {
	      long long sample_stream_id;
              memcpy(&sample_stream_id,&data[offset],sizeof(long long));
	      printf("\tsample_stream_id: %lld\n",sample_stream_id);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_CPU) {
	     int cpu, res;
              memcpy(&cpu,&data[offset],sizeof(int));
              memcpy(&res,&data[offset+4],sizeof(int));
	      printf("\tcpu: %d  res %d\n",cpu,res);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_PERIOD) {
	      long long period;
              memcpy(&period,&data[offset],sizeof(long long));
	      printf("\tperiod: %lld\n",period);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_READ) {
	      int length;

	      printf("\tread_format\n");
	      length=handle_struct_read_format(&data[offset],read_format);
	      
	      if (length>=0) offset+=length;
	   }
	   if (sample_type & PERF_SAMPLE_CALLCHAIN) {
	     long long nr,ip;
	      memcpy(&nr,&data[offset],sizeof(long long));
	      printf("\tcallchain length: %lld\n",nr);
	      offset+=8;
	      
	      for(i=0;i<nr;i++) {
	         memcpy(&ip,&data[offset],sizeof(long long));
	         printf("\t\t ip[%d]: %llx\n",i,ip);
	         offset+=8;
	      }
	   }
	   if (sample_type & PERF_SAMPLE_RAW) {
	      int size;
	     
	      memcpy(&size,&data[offset],sizeof(int));
	      printf("\tRaw length: %d\n",size);
	      offset+=4;

	      printf("\t\t");
	      for(i=0;i<size;i++) {	         
	         printf("%d ",data[offset]);
	         offset+=1;
	      }
	      printf("\n");
	   }
	   if (sample_type & PERF_SAMPLE_BRANCH_STACK) {
	      long long bnr;
	      memcpy(&bnr,&data[offset],sizeof(long long));
	      printf("\tbranch_stack entries: %lld\n",bnr);
	      offset+=8;
	      
	      for(i=0;i<bnr;i++) {
		 long long from,to,flags;
		 memcpy(&from,&data[offset],sizeof(long long));
	         offset+=8;
		 memcpy(&to,&data[offset],sizeof(long long));
	         offset+=8;
		 memcpy(&flags,&data[offset],sizeof(long long));
	         offset+=8;
	         printf("\t\t lbr[%d]: %llx %llx %llx\n",i,from,to,flags);
	      }
	   }
           break;
         default:printf("Unknown type %d\n",event->type);
      }
      tail+=event->size;
   }

   control_page->data_tail=head;
   
   return 0;

}


static void our_handler(int signum,siginfo_t *oh, void *blah) {
  int ret;

  ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE, 0);


  perf_mmap_read(); 

  switch(oh->si_code) {
     case POLL_IN:  count.in++;  break;
     case POLL_OUT: count.out++; break;
     case POLL_MSG: count.msg++; break;
     case POLL_ERR: count.err++; break;
     case POLL_PRI: count.pri++; break;
     case POLL_HUP: count.hup++; break;
     default: count.unknown++; break;
  }

  count.total++;

  ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH, 1);

  (void) ret;
  
}


int main(int argc, char** argv) {
   
   int ret,quiet;
   int mmap_pages=1+8;

   struct perf_event_attr pe;

   struct sigaction sa;
   char test_string[]="Testing sampled overflow...";
   
   quiet=test_quiet();

   if (!quiet) printf("This tests the sampling interface.\n");
   
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
   pe.sample_period=SAMPLE_FREQUENCY;
   pe.sample_type=sample_type;

   pe.read_format=read_format;
   pe.disabled=1;
   pe.pinned=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;
   pe.wakeup_events=1;

   arch_adjust_domain(&pe,quiet);

   fd1=perf_event_open(&pe,0,-1,-1,0);
   if (fd1<0) {
     if (!quiet) fprintf(stderr,"Error opening leader %llx\n",pe.config);
     test_fail(test_string);
   }

   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_HARDWARE;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=PERF_COUNT_HW_CPU_CYCLES;
   pe.sample_type=PERF_SAMPLE_IP;
   pe.read_format=PERF_FORMAT_GROUP|PERF_FORMAT_ID;
   pe.disabled=0;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   arch_adjust_domain(&pe,quiet);

   fd2=perf_event_open(&pe,0,-1,fd1,0);
   if (fd2<0) {
     if (!quiet) fprintf(stderr,"Error opening %llx\n",pe.config);
     test_fail(test_string);
   }

   our_mmap=mmap(NULL, mmap_pages*4096, 
                 PROT_READ|PROT_WRITE, MAP_SHARED, fd1, 0);

   
   fcntl(fd1, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
   fcntl(fd1, F_SETSIG, SIGIO);
   fcntl(fd1, F_SETOWN,getpid());
   
   ioctl(fd1, PERF_EVENT_IOC_RESET, 0);   

   ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);

   if (ret<0) {
     if (!quiet) fprintf(stderr,"Error with PERF_EVENT_IOC_ENABLE of group leader: "
	     "%d %s\n",errno,strerror(errno));
     exit(1);
   }

   instructions_million();
   
   ret=ioctl(fd1, PERF_EVENT_IOC_REFRESH,0);

   if (!quiet) {
     printf("Counts, using mmap buffer %p\n",our_mmap);
      printf("\tPOLL_IN : %d\n",count.in);
      printf("\tPOLL_OUT: %d\n",count.out);
      printf("\tPOLL_MSG: %d\n",count.msg);
      printf("\tPOLL_ERR: %d\n",count.err);
      printf("\tPOLL_PRI: %d\n",count.pri);
      printf("\tPOLL_HUP: %d\n",count.hup);
      printf("\tUNKNOWN : %d\n",count.unknown);
   }

   if (count.total==0) {
      if (!quiet) printf("No overflow events generated.\n");
      test_fail(test_string);
   }

   munmap(our_mmap,mmap_pages*4096);

   close(fd2);
   close(fd1);

   test_pass(test_string);
   
   return 0;
}

