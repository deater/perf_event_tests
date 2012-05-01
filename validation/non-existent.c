/* This file attempts to test if non-existent events fail */

/* The reason for this test is that in the various linux-kernel  */
/*   pre-defined event definitions, sometimes unavailable events */
/*   are implicitly set to 0 and sometimes set to -1.            */
/*   This test makes sure both fail properly.                    */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */


char test_string[]="Testing if non-existent events fail...";
int quiet=0;
int fd;

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"

#define NUM_RUNS 100

int main(int argc, char **argv) {

   struct perf_event_attr pe;
   int fails=0,expected_fails=0,cpu;
   int zero_event=0,negone_event=0;
   int zero_type=0,negone_type=0;

   quiet=test_quiet();

   cpu=detect_processor();

   /* TODO:
        PROCESSOR_PENTIUM_PRO  1
        PROCESSOR_PENTIUM_II   2
        PROCESSOR_PENTIUM_III  3
        PROCESSOR_PENTIUM_4    4
        PROCESSOR_PENTIUM_M    5
        PROCESSOR_COREDUO      6
        PROCESSOR_AMD_FAM15H  19
        PROCESSOR_POWER3      103
        PROCESSOR_POWER4      104
        PROCESSOR_POWER5      105
        PROCESSOR_POWER6      106
        PROCESSOR_POWER7      107
   */

   if (( cpu==PROCESSOR_CORTEX_A8) ||
       ( cpu==PROCESSOR_CORTEX_A9)) {

     /* ARM does things a bit differently; it has HW_OP_UNSUPPORTED, */
     /* and CACHE_OP_UNSUPPORTED, bot set to 0xffff                  */

     /* HW_OP_UNSUPPORTED */
     zero_type=PERF_TYPE_HARDWARE;
     zero_event=PERF_COUNT_HW_BUS_CYCLES;

     /* CACHE_OP_UNSUPPORTED */
     negone_type=PERF_TYPE_HW_CACHE;
     negone_event= PERF_COUNT_HW_CACHE_BPU |
                   ( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
                   (PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);

   }

   else if (( cpu==PROCESSOR_SANDYBRIDGE) ||
       ( cpu==PROCESSOR_WESTMERE) ||
       ( cpu==PROCESSOR_WESTMERE_EX) ||
       ( cpu==PROCESSOR_NEHALEM) ||
       ( cpu==PROCESSOR_NEHALEM_EX) ||
       ( cpu== PROCESSOR_CORE2) ||
       ( cpu== PROCESSOR_ATOM)) {

      zero_type=PERF_TYPE_HW_CACHE;
      zero_event= PERF_COUNT_HW_CACHE_DTLB |
                   ( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
                   (PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);

      negone_type=PERF_TYPE_HW_CACHE;
      negone_event= PERF_COUNT_HW_CACHE_BPU |
                   ( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
                   (PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);
   }
   
   else if ( (cpu==PROCESSOR_K7) ||
        (cpu==PROCESSOR_K8) ||
        (cpu==PROCESSOR_AMD_FAM10H) ||
        (cpu==PROCESSOR_AMD_FAM11H) ||
        (cpu==PROCESSOR_AMD_FAM14H)) {

     /* event implicitly set to 0 */ 
     zero_type=PERF_TYPE_HARDWARE;
     zero_event=PERF_COUNT_HW_BUS_CYCLES;

     /* event set to -1 in linux-2.6/arch/x86/kernel/cpu/perf_event_amd.h */
     negone_type=PERF_TYPE_HW_CACHE;
     negone_event= PERF_COUNT_HW_CACHE_BPU |
                   ( PERF_COUNT_HW_CACHE_OP_PREFETCH <<8) |
                   (PERF_COUNT_HW_CACHE_RESULT_ACCESS <<16);
   }
   else {
     test_skip(test_string);
   }

   
   if (!quiet) {
      printf("This test checks if non-existent events fail\n\n");
      printf("First testing non-existent (set to 0) events\n");
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=zero_type;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=zero_event;
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     if (!quiet) printf("\tCorrectly failed!\n");
     fails++;
   }
   else {
     if (!quiet) printf("\tERROR: we succeded!\n");
   }

   expected_fails++;

   close(fd);

   if (!quiet) {
      printf("\nNow testing non-existent (set to -1) events\n");
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=negone_type;
   pe.size=sizeof(struct perf_event_attr);

   pe.config=negone_event;
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
     if (!quiet) printf("\tCorrectly failed!\n");
     fails++;
   }
   else {
     if (!quiet) printf("\tERROR: we succeded!\n");
   }

   expected_fails++;

   close(fd);

   if (!quiet) printf("\n");

   if (fails!=expected_fails) {
      if (!quiet) printf("Not enough failures!\n");
      test_fail(test_string);
   }

   test_pass(test_string);

   return 0;
}
