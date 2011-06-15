/* This measures read latency */

/* by Vince Weaver, vweaver1@eecs.utk.edu                      */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <time.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "test_utils.h"
#include "instructions_testcode.h"

#define TIMES_TO_REPEAT 50

long long convert_to_ns(struct timespec *before,
			struct timespec *after) {

  long long seconds;
  long long ns;

  seconds=after->tv_sec - before->tv_sec;
  ns = after->tv_nsec - before->tv_nsec;

  ns = (seconds*1000000000ULL)+ns;

  return ns;
}

int main(int argc, char **argv) {

  int retval,quiet,result;

   long long high=0,low=0;

   int i;

   int EventSet=PAPI_NULL;

   int events[1];
   long long counts[1];

   char test_string[]="Testing PAPI latency...";
   
   struct timespec before,after;
   struct timespec start_before, start_after;
   struct timespec stop_before, stop_after;
   struct timespec read_before, read_after;

   long long init_ns=0,query_ns=0,start_ns=0,stop_ns=0,read_ns=0;
   long long start_high,stop_high,read_high;
   long long start_low,stop_low,read_low;
   long long start_total,stop_total,read_total;
   long long ns=0,total=0;

   quiet=test_quiet();

   clock_gettime(CLOCK_REALTIME,&before);

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      if (!quiet) printf("ERROR: PAPI_library_init %d\n", retval);
      test_fail(test_string);
   }

   clock_gettime(CLOCK_REALTIME,&after);

   init_ns=convert_to_ns(&before,&after);

   if (!quiet) printf("PAPI_library_init latency: %lld nsecs\n",init_ns);

   total=0; high=0; low=0;

   for(i=0;i<TIMES_TO_REPEAT;i++) {

      clock_gettime(CLOCK_REALTIME,&before);
      retval = PAPI_query_event(PAPI_TOT_INS);
      clock_gettime(CLOCK_REALTIME,&after);

      if (retval != PAPI_OK) {
         if (!quiet) printf("PAPI_BR_INS not supported %d\n", retval);
         test_skip(test_string);
      }

      ns=convert_to_ns(&before,&after);
      //      printf("%d %lld\n",i,ns);
      if (ns > high) high=ns;
      if ((ns < low) || (low==0)) low=ns;
      total+=ns;
   }

   query_ns=total/TIMES_TO_REPEAT;

   if (!quiet) printf("PAPI_query_event average_latency: %lld nsecs (%lld %lld)\n",query_ns,high,low);


   retval = PAPI_create_eventset(&EventSet);


   if (retval!=PAPI_OK) {
     if (!quiet) printf("PAPI_create_eventset failed\n");
     test_fail(test_string);
   }

   retval=PAPI_event_name_to_code("PAPI_BR_INS",&events[0]);

   if (retval!=PAPI_OK) {
     if (!quiet) printf("PAPI_event_name_to_code failed\n");
     test_fail(test_string);
   }

   retval=PAPI_add_event(EventSet,events[0]);

   if (retval!=PAPI_OK) {
     if (!quiet) printf("PAPI_add_event failed\n");
     test_fail(test_string);
   }


   events[0]=PAPI_BR_INS;

   start_total=0; stop_total=0; read_total=0;
   start_high=0; stop_high=0; read_high=0;
   start_low=0;  stop_low=0;  read_low=0;

   for(i=0;i<TIMES_TO_REPEAT;i++) {


      clock_gettime(CLOCK_REALTIME,&start_before);
      PAPI_start(EventSet);
      clock_gettime(CLOCK_REALTIME,&start_after);

      clock_gettime(CLOCK_REALTIME,&read_before);

      clock_gettime(CLOCK_REALTIME,&read_after);

      result=instructions_million();

      clock_gettime(CLOCK_REALTIME,&read_before);
      PAPI_read(EventSet,counts);
      clock_gettime(CLOCK_REALTIME,&read_after);

      clock_gettime(CLOCK_REALTIME,&stop_before);
      PAPI_stop(EventSet,counts);
      clock_gettime(CLOCK_REALTIME,&stop_after);

      if (result==CODE_UNIMPLEMENTED) {
         if (!quiet) printf("\tNo test code for this architecture\n");
	 test_skip(test_string);
      }

      ns=convert_to_ns(&start_before,&start_after);
      if (ns > start_high) start_high=ns;
      if ((ns < start_low) || (start_low==0)) start_low=ns;
      start_total+=ns;

      ns=convert_to_ns(&read_before,&read_after);
      if (ns > read_high) read_high=ns;
      if ((ns < read_low) || (read_low==0)) read_low=ns;
      read_total+=ns;

      ns=convert_to_ns(&stop_before,&stop_after);
      if (ns > stop_high) stop_high=ns;
      if ((ns < stop_low) || (stop_low==0)) stop_low=ns;
      stop_total+=ns;

   }

   start_ns=start_total/TIMES_TO_REPEAT;
   if (!quiet) {
      printf("PAPI_start average_latency: %lld nsecs (%lld %lld)\n",start_ns,start_high,start_low);
   }

   read_ns=read_total/TIMES_TO_REPEAT;
   if (!quiet) {
      printf("PAPI_read average_latency: %lld nsecs (%lld %lld)\n",read_ns,read_high,read_low);
   }

   stop_ns=stop_total/TIMES_TO_REPEAT;
   if (!quiet) {
      printf("PAPI_stop average_latency: %lld nsecs (%lld %lld)\n",stop_ns,stop_high,stop_low);
   }

   PAPI_shutdown();

   test_pass(test_string);
   
   return 0;
}
