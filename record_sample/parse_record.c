/* pare_record.c  */
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
#include "perf_helpers.h"

/* Urgh who designed this interface */
static int handle_struct_read_format(unsigned char *sample, 
				     int read_format,
				     int quiet) {
  
  int offset=0,i;

  if (read_format & PERF_FORMAT_GROUP) {
     long long nr,time_enabled,time_running;

     memcpy(&nr,&sample[offset],sizeof(long long));
     if (!quiet) printf("\t\tNumber: %lld ",nr);
     offset+=8;

     if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) {
        memcpy(&time_enabled,&sample[offset],sizeof(long long));
        if (!quiet) printf("enabled: %lld ",time_enabled);
        offset+=8;
     }
     if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) {
        memcpy(&time_running,&sample[offset],sizeof(long long));
        if (!quiet) printf("running: %lld ",time_running);
        offset+=8;
     }

     if (!quiet) printf("\n");

     for(i=0;i<nr;i++) {
        long long value, id;
        memcpy(&value,&sample[offset],sizeof(long long));
        if (!quiet) printf("\t\t\tValue: %lld ",value);
	offset+=8;

        if (read_format & PERF_FORMAT_ID) {
           memcpy(&id,&sample[offset],sizeof(long long));
           if (!quiet) printf("id: %lld ",id);
           offset+=8;
	}

        if (!quiet) printf("\n");
     }
  }
  else {
    long long value,time_enabled,time_running,id;
    memcpy(&value,&sample[offset],sizeof(long long));
    if (!quiet) printf("\t\tValue: %lld ",value);
    offset+=8;

    if (read_format & PERF_FORMAT_TOTAL_TIME_ENABLED) {
       memcpy(&time_enabled,&sample[offset],sizeof(long long));
       if (!quiet) printf("enabled: %lld ",time_enabled);
       offset+=8;
    }
    if (read_format & PERF_FORMAT_TOTAL_TIME_RUNNING) {
       memcpy(&time_running,&sample[offset],sizeof(long long));
       if (!quiet) printf("running: %lld ",time_running);
       offset+=8;
    }
    if (read_format & PERF_FORMAT_ID) {
       memcpy(&id,&sample[offset],sizeof(long long));
       if (!quiet) printf("id: %lld ",id);
       offset+=8;
    }
    if (!quiet) printf("\n");
  }

  return offset;
}

int perf_mmap_read( void *our_mmap, int sample_type, 
		    int read_format, int quiet ) {

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

   //printf("Head: %d %d\n",head,tail);
   rmb();

   /* data starts one page after control */
   data=((unsigned char*)our_mmap) + getpagesize(  );

   /*FIXME..crosspage boundary?*/

   struct perf_event_header *event;

   while(tail!=head) {

      event = ( struct perf_event_header * ) & data[tail];

      if (!quiet) {
	 printf("Header: %d %d %d\n",event->type,event->misc,event->size);
      }
      offset=tail+8; /* skip header */
      switch(event->type) {

         case PERF_RECORD_SAMPLE:
	   if (sample_type & PERF_SAMPLE_IP) {
	     long long ip;
	     memcpy(&ip,&data[offset],sizeof(long long));
	     if (!quiet) printf("\tIP: %llx\n",ip);
	     offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_TID) {
	      int pid, tid;
              memcpy(&pid,&data[offset],sizeof(int));
              memcpy(&tid,&data[offset+4],sizeof(int));
	      if (!quiet) printf("\tpid: %d  tid %d\n",pid,tid);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_TIME) {
	      long long time;
              memcpy(&time,&data[offset],sizeof(long long));
	      if (!quiet) printf("\ttime: %lld\n",time);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_ADDR) {
	      long long addr;
              memcpy(&addr,&data[offset],sizeof(long long));
	      if (!quiet) printf("\taddr: %llx\n",addr);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_ID) {
	      long long sample_id;
              memcpy(&sample_id,&data[offset],sizeof(long long));
	      if (!quiet) printf("\tsample_id: %lld\n",sample_id);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_STREAM_ID) {
	      long long sample_stream_id;
              memcpy(&sample_stream_id,&data[offset],sizeof(long long));
	      if (!quiet) {
                 printf("\tsample_stream_id: %lld\n",sample_stream_id);
	      }
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_CPU) {
	     int cpu, res;
              memcpy(&cpu,&data[offset],sizeof(int));
              memcpy(&res,&data[offset+4],sizeof(int));
	      if (!quiet) printf("\tcpu: %d  res %d\n",cpu,res);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_PERIOD) {
	      long long period;
              memcpy(&period,&data[offset],sizeof(long long));
	      if (!quiet) printf("\tperiod: %lld\n",period);
	      offset+=8;
	   }
	   if (sample_type & PERF_SAMPLE_READ) {
	      int length;

	      if (!quiet) printf("\tread_format\n");
	      length=handle_struct_read_format(&data[offset],
					       read_format,quiet);
	      
	      if (length>=0) offset+=length;
	   }
	   if (sample_type & PERF_SAMPLE_CALLCHAIN) {
	     long long nr,ip;
	      memcpy(&nr,&data[offset],sizeof(long long));
	      if (!quiet) printf("\tcallchain length: %lld\n",nr);
	      offset+=8;
	      
	      for(i=0;i<nr;i++) {
	         memcpy(&ip,&data[offset],sizeof(long long));
	         if (!quiet) printf("\t\t ip[%d]: %llx\n",i,ip);
	         offset+=8;
	      }
	   }
	   if (sample_type & PERF_SAMPLE_RAW) {
	      int size;
	     
	      memcpy(&size,&data[offset],sizeof(int));
	      if (!quiet) printf("\tRaw length: %d\n",size);
	      offset+=4;

	      if (!quiet) printf("\t\t");
	      for(i=0;i<size;i++) {	         
		if (!quiet) printf("%d ",data[offset]);
	         offset+=1;
	      }
	      if (!quiet) printf("\n");
	   }
	   if (sample_type & PERF_SAMPLE_BRANCH_STACK) {
	      long long bnr;
	      memcpy(&bnr,&data[offset],sizeof(long long));
	      if (!quiet) printf("\tbranch_stack entries: %lld\n",bnr);
	      offset+=8;
	      
	      for(i=0;i<bnr;i++) {
		 long long from,to,flags;
		 memcpy(&from,&data[offset],sizeof(long long));
	         offset+=8;
		 memcpy(&to,&data[offset],sizeof(long long));
	         offset+=8;
		 memcpy(&flags,&data[offset],sizeof(long long));
	         offset+=8;
	         if (!quiet) {
                    printf("\t\t lbr[%d]: %llx %llx %llx\n",i,from,to,flags);
		 }
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


