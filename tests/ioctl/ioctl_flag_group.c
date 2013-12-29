/* ioctl_flag_group.c                               */
/* Test the PERF_IOC_FLAG_GROUP ioctl parameter     */

/* by Vince Weaver   vincent.weaver _at_ maine.edu  */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <asm/unistd.h>
#include <sys/ioctl.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "instructions_testcode.h"

char test_string[]="Testing PERF_IOC_FLAG_GROUP...";

int main(int argc, char** argv) {

   int fd[2];
   int quiet;
   int read_result;
   long long insn_count,cycles_count;
	int error=0;

   struct perf_event_attr pe;

   quiet=test_quiet();

   if (!quiet) {
      printf("This tests the PERF_IOC_FLAG_GROUP ioctl parameter.\n\n");
   }

   /**********************************************/
   /* With ENABLE/DISABLE on GROUP LEADER with 0 */
   /**********************************************/

   	if (!quiet) {
		printf("Testing on GROUP LEADER with param 0\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;
	arch_adjust_domain(&pe, quiet);



	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[1]=perf_event_open(&pe,0,-1,fd[0],0);
	if (fd[1]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	ioctl(fd[0], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,0);

	instructions_million();

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,0);

	read_result=read(fd[0],&insn_count,sizeof(long long));

	if (read_result!=sizeof(long long)) {
		fprintf(stderr,"\tImproper return from read: %d\n",read_result);
		test_fail(test_string);
	}

	read_result=read(fd[1],&cycles_count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string);
   }

	close(fd[0]);
	close(fd[1]);

	if (!quiet) {
		printf("\t%lld instructions\n",insn_count);
		printf("\t%lld cycles\n",cycles_count);
	}

	if ((insn_count<100) || (cycles_count<100)) {
		fprintf(stderr,"Error! Should have values for insn and cycles\n");
		error++;
	}

   /*****************************************************************/
   /* With ENABLE/DISABLE on GROUP LEADER with PERF_IOC_FLAG_GROUP */
   /*****************************************************************/

   	if (!quiet) {
		printf("Testing on GROUP LEADER with param PERF_IOC_FLAG_GROUP\n");
	}

   memset(&pe,0,sizeof(struct perf_event_attr));

   pe.type=PERF_TYPE_HARDWARE;
   pe.config=PERF_COUNT_HW_INSTRUCTIONS;
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[1]=perf_event_open(&pe,0,-1,fd[0],0);
	if (fd[1]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	ioctl(fd[0], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
	ioctl(fd[0], PERF_EVENT_IOC_ENABLE,PERF_IOC_FLAG_GROUP);

	instructions_million();

	ioctl(fd[0], PERF_EVENT_IOC_DISABLE,PERF_IOC_FLAG_GROUP);

   read_result=read(fd[0],&insn_count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string);
   }

   read_result=read(fd[1],&cycles_count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string);
   }

	close(fd[0]);
	close(fd[1]);


	if (!quiet) {
		printf("\t%lld instructions\n",insn_count);
		printf("\t%lld cycles\n",cycles_count);
	}

	if ((insn_count<100) || (cycles_count<100)) {
		fprintf(stderr,"Error! Should have values for insn and cycles\n");
		error++;
	}


   /**********************************************/
   /* With ENABLE/DISABLE on non-group-leader with 0 */
   /**********************************************/

   	if (!quiet) {
		printf("Testing non-leader with param 0\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[1]=perf_event_open(&pe,0,-1,fd[0],0);
	if (fd[1]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	ioctl(fd[1], PERF_EVENT_IOC_RESET, 0);
	ioctl(fd[1], PERF_EVENT_IOC_ENABLE,0);

	instructions_million();

	ioctl(fd[1], PERF_EVENT_IOC_DISABLE,0);

   read_result=read(fd[0],&insn_count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string);
   }

   read_result=read(fd[1],&cycles_count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string);
   }

   close(fd[0]);
   close(fd[1]);

   if (!quiet) {
     printf("\t%lld instructions\n",insn_count);
     printf("\t%lld cycles\n",cycles_count);
   }

	if ((insn_count>100) || (cycles_count>100)) {
		fprintf(stderr,"Error! Should have 0 for insn and cycles\n");
		error++;
	}


	/**************************************************************/
	/* With ENABLE/DISABLE on non-leader with PERF_IOC_FLAG_GROUP */
	/**************************************************************/

   	if (!quiet) {
		printf("Testing on non-leader with param PERF_IOC_FLAG_GROUP\n");
	}

	memset(&pe,0,sizeof(struct perf_event_attr));

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_INSTRUCTIONS;
	pe.disabled=1;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[0]=perf_event_open(&pe,0,-1,-1,0);
	if (fd[0]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	pe.type=PERF_TYPE_HARDWARE;
	pe.config=PERF_COUNT_HW_CPU_CYCLES;
	pe.disabled=0;
	pe.exclude_kernel=1;
	pe.exclude_hv=1;

	arch_adjust_domain(&pe, quiet);

	fd[1]=perf_event_open(&pe,0,-1,fd[0],0);
	if (fd[1]<0) {
		if (!quiet) fprintf(stderr,"Error opening\n");
		test_fail(test_string);
	}

	ioctl(fd[1], PERF_EVENT_IOC_RESET, PERF_IOC_FLAG_GROUP);
	ioctl(fd[1], PERF_EVENT_IOC_ENABLE,PERF_IOC_FLAG_GROUP);

	instructions_million();

	ioctl(fd[1], PERF_EVENT_IOC_DISABLE,PERF_IOC_FLAG_GROUP);

   read_result=read(fd[0],&insn_count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string);
   }

   read_result=read(fd[1],&cycles_count,sizeof(long long));

   if (read_result!=sizeof(long long)) {
     fprintf(stderr,"\tImproper return from read: %d\n",read_result);
     test_fail(test_string);
   }

   close(fd[0]);
   close(fd[1]);


   if (!quiet) {
     printf("\t%lld instructions\n",insn_count);
     printf("\t%lld cycles\n",cycles_count);
   }

	if ((insn_count<100) || (cycles_count<100)) {
		fprintf(stderr,"Error! Should have values for insn and cycles\n");
		error++;
	}

	if (error) {
		test_fail(test_string);
	}

	test_pass(test_string);

	return 0;
}
