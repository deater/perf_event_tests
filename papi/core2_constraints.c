/* This test attempts to see if on core2 processors   */
/* counter constraints are handled properly.          */
/* Some events, such as FP_COMP_OPS_EXE can only run  */
/* on one of the two counters.  Kernels before 2.6.34 */
/* Did not catch this, and thus would silently return */
/* a value of 0 if run on such a kernel.              */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "papiStdEventDefs.h"
#include "papi.h"
#include "test_utils.h"
#include "matrix_multiply.h"


int main(int argc, char **argv) {

   int retval,quiet;
   const PAPI_hw_info_t *info;

	int eventset = PAPI_NULL;
	int eventset2 = PAPI_NULL;

   long long counts[2];
   double error;
   long long expected;

   char test_string[]="Testing core2_constraints...";

   quiet=test_quiet();

   retval = PAPI_library_init(PAPI_VER_CURRENT);
   if (retval != PAPI_VER_CURRENT) {
     if (!quiet) printf("ERROR: PAPI_library_init %d\n", retval);
        test_fail(test_string);
   }

   if ( (info=PAPI_get_hardware_info())==NULL) {
     if (!quiet) printf("cannot obtain hardware info %d\n",retval);
      test_fail(test_string);
   }

   if ((info->vendor==PAPI_VENDOR_INTEL) && (info->cpuid_family==6) && 
			((info->cpuid_model==15) || (info->cpuid_model==23) || (info->cpuid_model==29))) {

     if (!quiet) printf("Found core2!\n");
   }
   else {
     if (!quiet) printf("Not a core2.\n");
     test_skip(test_string);
   }

   expected=naive_matrix_multiply_estimated_flops(quiet);

	retval=PAPI_create_eventset(&eventset);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("PAPI_create_eventset() failed\n");
		test_fail(test_string);
	}

   retval=PAPI_add_named_event(eventset,"FP_COMP_OPS_EXE");
   if (retval!=PAPI_OK) {
      if (!quiet) printf("PAPI_event_name_to_code %d\n", retval);
      test_fail(test_string);
   }

	retval=PAPI_add_named_event(eventset,"PAPI_TOT_INS");
	if (retval!=PAPI_OK) {
		if (!quiet) printf("Error adding PAPI_TOT_INS\n");
		test_fail(test_string);
	}

   PAPI_start(eventset);

   naive_matrix_multiply(quiet);

   PAPI_stop(eventset,counts);

   error=(((double)counts[0]-(double)expected)/(double)expected)*100.0;
   if (!quiet) printf("   Expected: %lld  Actual: %lld   Error: %.2lf\n", 
             expected, counts[0],error);

   if (error > 1.0) {
      if (!quiet) printf("FP error higher than expected\n");
      test_fail(test_string);
   }

   /* set FP_COMP_OPS_EXE to be in slot 2 */

	retval=PAPI_create_eventset(&eventset2);
	if (retval!=PAPI_OK) {
		if (!quiet) printf("PAPI_create_eventset() failed\n");
		test_fail(test_string);
	}

	retval=PAPI_add_named_event(eventset2,"FP_COMP_OPS_EXE");
	if (retval!=PAPI_OK) {
		if (!quiet) printf("PAPI_event_name_to_code %d\n", retval);
		test_fail(test_string);
	}
	retval=PAPI_add_named_event(eventset2,"FP_COMP_OPS_EXE");
	if (retval!=PAPI_OK) {
		if (!quiet) printf("PAPI_event_name_to_code %d\n", retval);
		test_fail(test_string);
	}

   PAPI_start(eventset2);

   naive_matrix_multiply(quiet);

   PAPI_stop(eventset2,counts);

   error=(((double)counts[1]-(double)expected)/(double)expected)*100.0;
   if (!quiet) printf("   Expected: %lld  Actual: %lld   Error: %.2lf\n", 
             expected, counts[1],error);

   if (error > 1.0) {
      if (!quiet) printf("FP error higher than expected\n");
      test_fail(test_string);
   }

   PAPI_shutdown();

   test_pass(test_string);

   return 0;
}
