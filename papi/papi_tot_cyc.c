/* This file attempts to test the retired instruction */
/* performance counter on various architectures, as   */
/* implemented by the PAPI_TOT_CYC counter.           */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "papiStdEventDefs.h"
#include "papi.h"

#include "papi_test.h"

#define NUM_RUNS 3

#define MATRIX_SIZE 512
double a[MATRIX_SIZE][MATRIX_SIZE];
double b[MATRIX_SIZE][MATRIX_SIZE];
double c[MATRIX_SIZE][MATRIX_SIZE];

void matrix_multiply() {

  double s;
  int i,j,k;

  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      a[i][j]=(double)i*(double)j;
      b[i][j]=(double)i/(double)(j+5);
    }
  }

  for(j=0;j<MATRIX_SIZE;j++) {
     for(i=0;i<MATRIX_SIZE;i++) {
        s=0;
        for(k=0;k<MATRIX_SIZE;k++) {
	   s+=a[i][k]*b[k][j];
	}
        c[i][j] = s;
     }
  }

  s=0.0;
  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      s+=c[i][j];
    }
  }

    printf("Validation: s=%lf\n",s);

  return;
}

int main(int argc, char **argv) {
   
   int retval;
   double mhz;
   const PAPI_hw_info_t *info;

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
      test_fail( __FILE__, __LINE__, "PAPI_library_init", retval);
   }

   retval = PAPI_query_event(PAPI_TOT_CYC);
   if (retval != PAPI_OK) {
      test_fail( __FILE__, __LINE__ ,"PAPI_TOT_CYC not supported", retval);
   }

   printf("\n");   

   if ( (info=PAPI_get_hardware_info())==NULL) {
      test_fail( __FILE__, __LINE__ ,"cannot obtain hardware info", retval);
   }
   mhz=info->mhz;
   printf("System MHZ = %f\n",mhz);


   /* Test a simple loop of 1 million instructions             */
   /* Most implementations should count be correct within 1%   */
   /* This loop in in assembly language, as compiler generated */
   /* code varies too much.                                    */

   int i;

   int events[1];
   long long counts[1],high=0,low=0,total=0,average=0;
   double error,highd=-1e6,lowd=1e6;
   long long seconds,usecs,usec_start,usec_end;
   long long expected;
   struct timeval start_time,end_time;

   events[0]=PAPI_TOT_CYC;

   printf("Testing a sleep of 1 second (%d times):\n",
          NUM_RUNS);

   for(i=0;i<NUM_RUNS;i++) {

      PAPI_start_counters(events,1);

      sleep(1);
     
      PAPI_stop_counters(counts,1);

      if (counts[0]>high) high=counts[0];
      if ((low==0) || (counts[0]<low)) low=counts[0];
      total+=counts[0];
   }

   average=total/NUM_RUNS;
 
   expected=(long long)(mhz*1.0e6);

   printf("   Expected: %lld\n", expected);
   printf("   High: %lld   Low:  %lld   Average:  %lld\n",
          high,low,average);

   printf("   ( note, a small value above %lld may be expected due\n",expected);
   printf("     to PAPI overhead and interrupt noise, among other reasons)\n");

   error=(((double)average-expected)/expected)*100.0;
   printf("   Average Error = %.2f%%\n",error);

   if ((error > 1.0) || (error<-1.0)) {

     //      test_fail( __FILE__, __LINE__, "Instruction count off by more than 1%", 0);
   }
   printf("\n");


   /* busy spin */
#define NUM_MATRIX_RUNS 5
   printf("Testing a busy loop (%d times):\n",
          NUM_MATRIX_RUNS);

   for(i=0;i<NUM_MATRIX_RUNS;i++) {

      PAPI_start_counters(events,1);
      //      gettimeofday(&start_time,NULL);

      usec_start=PAPI_get_real_usec();

      matrix_multiply();

      usec_end=PAPI_get_real_usec();

      //      gettimeofday(&end_time,NULL);
      PAPI_stop_counters(counts,1);

      //      seconds=end_time.tv_sec-start_time.tv_sec;
      // usecs=(end_time.tv_usec-start_time.tv_usec);
      //if (usecs<0) {
      //   seconds--;
      //   usecs=-usecs;
      //}

      usecs=usec_end-usec_start;
      printf("   Took %d.%06ds ",(usecs/1000000),usecs%1000000);

      //      usecs=usecs+1000000*seconds;
      expected=(long long)(mhz*usecs);
      error=(((double)counts[0]-(double)expected)/(double)expected)*100.0;
      printf("   Expected: %lld  Actual: %lld   Error: %.2lf\n", 
             expected, counts[0],error);

      if (error>highd) highd=error;
      if (error<lowd) lowd=error;
   }
 
   printf("   High: %lf   Low:  %lf\n",
          highd,lowd);


   printf("\n");

   PAPI_shutdown();

   test_pass( __FILE__ , NULL, 0);

   return 0;
}
