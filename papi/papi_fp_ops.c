/* This file attempts to test the retired FP          */
/* performance counter on various architectures, as   */
/* implemented by the PAPI_FP_OPS counter.            */

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

long long estimated_flops() {

  long long muls,divs,adds;

  /* setup */
  muls=MATRIX_SIZE*MATRIX_SIZE;
  divs=MATRIX_SIZE*MATRIX_SIZE;
  adds=MATRIX_SIZE*MATRIX_SIZE;

  /* multiply */
  muls+=MATRIX_SIZE*MATRIX_SIZE*MATRIX_SIZE;
  adds+=MATRIX_SIZE*MATRIX_SIZE*MATRIX_SIZE;

  /* sum */
  adds+=MATRIX_SIZE*MATRIX_SIZE;

  printf("adds: %lld muls: %lld divs: %lld\n",adds,muls,divs);

  return adds+muls+divs;
}

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

   retval = PAPI_query_event(PAPI_FP_OPS);
   if (retval != PAPI_OK) {
      test_fail( __FILE__, __LINE__ ,"PAPI_FP_OPS not supported", retval);
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
   long long expected;

   events[0]=PAPI_FP_OPS;

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
 
   expected=0;

   printf("   Expected: %lld\n", expected);
   printf("   High: %lld   Low:  %lld   Average:  %lld\n",
          high,low,average);

   printf("   ( note, a small value above %lld may be expected due\n",expected);
   printf("     to PAPI overhead and interrupt noise, among other reasons)\n");

   error=((double)average-expected);
   printf("   Average Error = %.2f\n",error);

   if (error > 500.0) {

      test_fail( __FILE__, __LINE__, "Instruction count higher than expected", 0);
   }
   printf("\n");


   /* busy spin */
#define NUM_MATRIX_RUNS 5
   printf("Testing a busy loop (%d times):\n",
          NUM_MATRIX_RUNS);

   for(i=0;i<NUM_MATRIX_RUNS;i++) {

      PAPI_start_counters(events,1);

      matrix_multiply();

      PAPI_stop_counters(counts,1);

      expected=estimated_flops();
      error=(((double)counts[0]-(double)expected)/(double)expected)*100.0;
      printf("   Expected: %lld  Actual: %lld   Error: %.2lf\n", 
             expected, counts[0],error);

      if (error>highd) highd=error;
      if (error<lowd) lowd=error;
   }
 
   printf("   High: %lf   Low:  %lf\n",
          highd,lowd);


   printf("\n");

   if (highd > 1.0) {
      test_fail( __FILE__, __LINE__, "FP error higher than expected", 0);
   }

   PAPI_shutdown();

   test_pass( __FILE__ , NULL, 0);

   return 0;
}
