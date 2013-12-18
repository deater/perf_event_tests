/* enable_on_exec.c  */
/* by Vince Weaver   vweaver1 _at_ eecs.utk.edu */

/* Test if the enable_on_exec flag works */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/ioctl.h>


#include "perf_event.h"
#include "test_utils.h"
#include "perf_helpers.h"
#include "instructions_testcode.h"


int main(int argc, char** argv) {
   
   int fd,ret,quiet;
   int result;
   int read_result;
   long long count;

   struct perf_event_attr pe;

   char test_string[]="Testing enable_on_exec...";

   char fd_string[BUFSIZ];

   quiet=test_quiet();

   /* First exec, without enable_on_exec */
   if (argc==2) {

      fd=atoi(argv[1]);

      if (!quiet) {
	 printf("\tWe've been exec'd without enable_on_exec, fd=%d!\n",fd);
      }

      result=instructions_million();
      if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

      read_result=read(fd,&count,sizeof(long long));
      if (read_result!=sizeof(long long)) printf("Unexpected read size\n");

      if (!quiet) printf("\tCounted %lld instructions\n",count);

      close(fd);

      memset(&pe,0,sizeof(struct perf_event_attr));

      pe.type=PERF_TYPE_HARDWARE;
      pe.size=sizeof(struct perf_event_attr);
      pe.config=PERF_COUNT_HW_INSTRUCTIONS;
      pe.disabled=1;
      pe.exclude_kernel=1;
      pe.enable_on_exec=1;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening first: %s\n",
			strerror(errno));
		test_fail(test_string);
		exit(1);
	}

      if (count!=0) test_fail(test_string);

      sprintf(fd_string,"%d",fd);

      /* exec ourselves, but with two arguments to change behavior */
      execl(argv[0],argv[0],fd_string,fd_string,NULL);

   }


   /* Second exec, with enable_on_exec */
   if (argc==3) {

      fd=atoi(argv[1]);

      if (!quiet) {
	 printf("\tWe've been exec'd with enable_on_exec, fd=%d!\n",fd);
      }

      result=instructions_million();
      if (result==CODE_UNIMPLEMENTED) printf("Warning, no million\n");

      read_result=read(fd,&count,sizeof(long long));
      if (read_result!=sizeof(long long)) printf("Unexpected read size\n");

      if (!quiet) printf("\tCounted %lld instructions\n",count);

      close(fd);

      if (count==0) test_fail(test_string);

      test_pass(test_string);

      return 0;

   }


   if (!quiet) {
      printf("Testing if enable_on_exec works as expected.\n");
   }

	/* setup perf-event */

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.size=sizeof(struct perf_event_attr);
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.enable_on_exec=0;

	arch_adjust_domain(&pe,quiet);

	fd=perf_event_open(&pe,0,-1,-1,0);
	if (fd<0) {
		fprintf(stderr,"Error opening second: %s\n",strerror(errno));
		test_fail(test_string);
		exit(1);
	}

	sprintf(fd_string,"%d",fd);


	/* exec ourselves, but with an argument to change behavior */
	execl(argv[0],argv[0],fd_string,NULL);

	(void) ret;

	return 0;
}
