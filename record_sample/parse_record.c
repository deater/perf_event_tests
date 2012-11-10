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

#include "parse_record.h"

/* Urgh who designed this interface */
static int handle_struct_read_format(unsigned char *sample, 
				     int read_format,
				     struct validate_values *validation,
				     int quiet) {
  
  int offset=0,i;

  if (read_format & PERF_FORMAT_GROUP) {
     long long nr,time_enabled,time_running;

     memcpy(&nr,&sample[offset],sizeof(long long));
     if (!quiet) printf("\t\tNumber: %lld ",nr);
     offset+=8;

     if (validation) {
        if (validation->events!=nr) {
	  fprintf(stderr,"Error!  Wrong number of events %d != %lld\n",
		  validation->events,nr);
        }
     }

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

long long perf_mmap_read( void *our_mmap, int mmap_size, long long prev_head,
		    int sample_type, int read_format, 
		    struct validate_values *validate,
		    int quiet ) {

   struct perf_event_mmap_page *control_page = our_mmap;
   long long head,offset;
   int i,size;
   long long bytesize,prev_head_wrap;

   unsigned char *data;

   void *data_mmap=our_mmap+getpagesize();

   if (control_page==NULL) {
      fprintf(stderr,"ERROR mmap page NULL\n");
      return -1;
   }
   
   head=control_page->data_head;
   rmb(); /* Must always follow read of data_head */

   size=head-prev_head;

   //printf("Head: %lld Prev_head=%lld\n",head,prev_head);
   //printf("%d new bytes\n",size);   

   bytesize=mmap_size*getpagesize();

   if (size>bytesize) {
     printf("error!  we overflowed the mmap buffer %d>%lld!\n",
	    size,bytesize);
   }

   data=malloc(bytesize);
   if (data==NULL) {
      return -1;
   }

   prev_head_wrap=prev_head%bytesize;

   //   printf("Copying %d bytes from %d to %d\n",
   //	  bytesize-prev_head_wrap,prev_head_wrap,0);
   memcpy(data,(unsigned char*)data_mmap + prev_head_wrap, 
	  bytesize-prev_head_wrap);

   //printf("Copying %d bytes from %d to %d\n",
   //	  prev_head_wrap,0,bytesize-prev_head_wrap);

   memcpy(data+(bytesize-prev_head_wrap),(unsigned char *)data_mmap,
	  prev_head_wrap);

   struct perf_event_header *event;


   offset=0;
   while(offset<size) {

     //printf("Offset %d Size %d\n",offset,size);
      event = ( struct perf_event_header * ) & data[offset];

      if (!quiet) {
	 switch(event->type) {
	 case 1: printf("PERF_RECORD_MMAP"); break;
	 case 2: printf("PERF_RECORD_LOST"); break;
	 case 3: printf("PERF_RECORD_COMM"); break;
	 case 4: printf("PERF_RECORD_EXIT"); break;
	 case 5: printf("PERF_RECORD_THROTTLE"); break;
	 case 6: printf("PERF_RECORD_UNTHROTTLE"); break;
	 case 7: printf("PERF_RECORD_FORK"); break;
	 case 8: printf("PERF_RECORD_READ"); break;
	 case 9: printf("PERF_RECORD_SAMPLE"); break;
	 default: printf("UNKNOWN"); break;
	 }
	 printf(", MISC=%d (",event->misc);
	 switch(event->misc & PERF_RECORD_MISC_CPUMODE_MASK) {
	 case PERF_RECORD_MISC_CPUMODE_UNKNOWN: 
              printf("PERF_RECORD_MISC_CPUMODE_UNKNOWN"); break; 
         case PERF_RECORD_MISC_KERNEL: 
              printf("PERF_RECORD_MISC_KERNEL"); break;
	 case PERF_RECORD_MISC_USER:
	      printf("PERF_RECORD_MISC_USER"); break;
         case PERF_RECORD_MISC_HYPERVISOR:
	      printf("PERF_RECORD_MISC_HYPERVISOR"); break;
	 case PERF_RECORD_MISC_GUEST_KERNEL:
	      printf("PERF_RECORD_MISC_GUEST_KERNEL"); break;
	 case PERF_RECORD_MISC_GUEST_USER:
              printf("PERF_RECORD_MISC_GUEST_USER"); break;
	 default:
	   printf("Unknown!\n"); break;
	 }

	 if (event->misc & PERF_RECORD_MISC_EXACT_IP)
            printf(",PERF_RECORD_MISC_EXACT_IP ");
	 if (event->misc & PERF_RECORD_MISC_EXT_RESERVED) 
            printf(",PERF_RECORD_MISC_EXT_RESERVED ");
         printf("), Size=%d\n",event->size);
      }
      offset+=8; /* skip header */
      switch(event->type) {

      case PERF_RECORD_LOST: {
	     long long id,lost;
	     memcpy(&id,&data[offset],sizeof(long long));
	     if (!quiet) printf("\tID: %lld\n",id);
	     offset+=8;
	     memcpy(&lost,&data[offset],sizeof(long long));
	     if (!quiet) printf("\tLOST: %lld\n",lost);
	     offset+=8;
      }
	   break;

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

	      if (validate) {
		 if (validate->pid!=pid) {
		    fprintf(stderr,"Error, pid %d != %d\n",
			   validate->pid,pid);
		 }
	      }

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
					       read_format,
					       validate,quiet);
	      
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
   }

   control_page->data_tail=head;

   free(data);
   
   return head;

}


