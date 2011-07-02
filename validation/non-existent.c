/* This file attempts to test if non-existent events fail */

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
   int zero_event,negone_event;

   quiet=test_quiet();

   cpu=detect_processor();
   
   if ( (cpu==PROCESSOR_K7) ||
        (cpu==PROCESSOR_K8) ||
        (cpu==PROCESSOR_AMD_FAM10H) ||
        (cpu==PROCESSOR_AMD_FAM11H) ||
        (cpu==PROCESSOR_AMD_FAM14H)) {

     zero_event=PERF_COUNT_HW_BUS_CYCLES;
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
   pe.type=PERF_TYPE_HARDWARE;
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
   pe.type=PERF_TYPE_HARDWARE;
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
