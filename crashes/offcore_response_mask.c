/* This tests for a crash when the offcore response masks were not  */
/* being properly set for sandybridge / ivybridge                   */

/* This was fixed in kernel 3.9                                     */
/* perf/x86: Fix offcore_rsp valid mask for SNB/IVB                 */
/* f1923820c447e986a9da0fc6bf60c1dccdf0408e                         */

/* This is CVE-2013-2146					    */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu        */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "matrix_multiply.h"

int main(int argc, char **argv) {

   int ret;
   struct perf_event_attr pe;
   int fd,fd2;
   long long counts[2];
   long long events[2];

   fprintf(stderr,"\n");
   fprintf(stderr,"This test attempts to write reserved bits in the offcore response registers\n");
   fprintf(stderr,"This could in theory cause a GPF on kernels that do not block this (before 3.9)\n\n");

   /* Working values on IVB */
   //events[0]=0x5301b7;
   //events[1]=0x10001;

   /* Potentially Dangerous Values */
   events[0]=0x5301b7;
   events[1]=0x3fffffffffull;

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.size=sizeof(struct perf_event_attr);
   pe.type=PERF_TYPE_RAW;
   pe.config=events[0];
   pe.config1=events[1];
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd=perf_event_open(&pe,0,-1,-1,0);
   if (fd<0) {
      fprintf(stderr,"Kernel properly blocked opening of config %llx config1 %llx %s\n",
              pe.config,pe.config1,strerror(errno));
      exit(0);
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.size=sizeof(struct perf_event_attr);
   pe.type=PERF_TYPE_RAW;
   pe.config=events[0];
   pe.config1=events[1];
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd2=perf_event_open(&pe,0,-1,fd,0);
   if (fd2<0) {
      fprintf(stderr,"Error opening %llx %llx %s\n",
              pe.config,pe.config1,strerror(errno));
      exit(1);
   }

   ioctl(fd, PERF_EVENT_IOC_RESET, 0);

   ret=ioctl(fd, PERF_EVENT_IOC_ENABLE,0);
   if (ret<0) {
      fprintf(stderr,"Error enabling events\n");
      exit(1);
   }

   /* do something */
   naive_matrix_multiply(0);

   ret=ioctl(fd, PERF_EVENT_IOC_DISABLE,0);
   if (ret<0) {
      fprintf(stderr,"Error disabling events\n");
      exit(1);
   }

   ret=read(fd,&counts[0],sizeof(long long));
   if (ret!=sizeof(long long)) {
      fprintf(stderr,"Unepxected result from read fd: %d\n",ret);
      exit(1);
   }

   ret=read(fd2,&counts[1],sizeof(long long));
   if (ret!=sizeof(long long)) {
      fprintf(stderr,"Unepxected result from read fd2: %d\n",ret);
      exit(1);
   }

   fprintf(stderr,"Read count %llx %llx\n\n",counts[0],counts[1]);

   return 0;
}


