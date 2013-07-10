/* This tests CBOX Uncore Events on SNB and IVB     */
/* These weren't supported until at least Linxu 3.6 */

/* by Vince Weaver, vincent.weaver _at_ maine.edu   */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <dirent.h>

#include <errno.h>

#include <sched.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "matrix_multiply.h"

#define MAX_PMUS 1024

char test_string[]="Testing cbox uncore events...";

struct {
	char *name;
	int type;
} pmu_info[MAX_PMUS];

int main(int argc, char **argv) {

   int ret,quiet,i,j;
   struct perf_event_attr pe;
   int fd[MAX_PMUS];
   long long counts[MAX_PMUS];
   int pmu_type;

   int num_pmus=0,num_events=0;

   DIR *sys_devices;
   struct dirent *dir;
   FILE *fff;

   char filename[BUFSIZ];
   char cbox_name[BUFSIZ];

   char uncore_event_name[BUFSIZ];
   unsigned int uncore_event=0x0;

   int processor_type;

   quiet=test_quiet();

   processor_type=detect_processor();

   switch(processor_type) {
      case PROCESSOR_SANDYBRIDGE_EP:
           strcpy(uncore_event_name,"UNC_C_TOR_OCCUPANCY:ALL");
           uncore_event=0x836;
           break;
      case PROCESSOR_SANDYBRIDGE:
           strcpy(uncore_event_name,"snb_unc_cbo0::UNC_CLOCKTICKS");
           uncore_event=0xff;
      case PROCESSOR_IVYBRIDGE:
           strcpy(uncore_event_name,"UNC_CBO_CACHE_LOOKUP:STATE_I:ANY_FILTER");
		// uncore_event=0x508f34; /* STATE_MESI */
		uncore_event=0x508834; /* STATE_I */
		//uncore_event=0x508134; /* STATE_M */
		// uncore_event=0x508434; /* STATE_S */
		//uncore_event=0x508234; /* STATE_E */
           break;
      default:
	   if (!quiet) fprintf(stderr,"Unsupported processor\n");
	   test_skip(test_string);
	   break;
   }

	if (!quiet) printf("Looking for PMUs:\n");

	sys_devices=opendir("/sys/bus/event_source/devices/");
	if (sys_devices==NULL) {
		if (!quiet) {
			fprintf(stderr,
				"Couldn't open /sys/bus_event/source/devices/\n");
		}
		test_skip(test_string);
	}


	while (1) {
		dir=readdir(sys_devices);
		if (dir==NULL) break;

		if (!strncmp(dir->d_name,".",1)) continue;
		if (!strncmp(dir->d_name,"..",2)) continue;

		sprintf(filename,"/sys/devices/%s/type",dir->d_name);
		fff=fopen(filename,"r");
		if (fff==NULL) continue;

		fscanf(fff,"%d",&pmu_type);

		if (!quiet) {
			printf("\tFound %s = %d\n",dir->d_name, pmu_type);
		}
		pmu_info[num_pmus].name=strdup(dir->d_name);
		pmu_info[num_pmus].type=pmu_type;

		num_pmus++;

		if (num_pmus>=MAX_PMUS) {
			fprintf(stderr,"Too many PMUS!\n");
			return -1;
		}

		fclose(fff);
   	}
	closedir(sys_devices);


	if (!quiet) printf("Looking for cboxes\n");
	for(i=0;i<MAX_PMUS;i++) {

		sprintf(cbox_name,"uncore_cbox_%d",i);

		for(j=0;j<num_pmus;j++) {

			if (!strcmp(pmu_info[j].name,cbox_name)) {
				printf("\tFound %s\n",cbox_name);
				printf("\tAdding %s\n",uncore_event_name);

				memset(&pe,0,sizeof(struct perf_event_attr));
				pe.type=pmu_info[j].type;
				pe.size=sizeof(struct perf_event_attr);
				pe.config=uncore_event;
				pe.disabled=1;

				/* can't filter like this on uncore */
				pe.exclude_user=0;
				pe.exclude_kernel=0;
				pe.exclude_hv=0;

				fd[num_events]=perf_event_open(&pe,
					-1,/* pid, measure all */
					0, /* cpu, use cpu 0 */
					-1,/* group leader */
					0  /* flags */);
				if (fd[num_events]<0) {
					if (!quiet) {
						fprintf(stderr,"Error opening %llx, %d : %s\n",
							pe.config,errno,strerror(errno));
					}
					if (errno==EACCES) {
						if (!quiet) {
							fprintf(stderr,"Test skipped due to lack of permissions\n");
						}
						test_skip(test_string);
					}
					test_fail(test_string);
				}
				num_events++;

			}

		}


	}

	if (num_events==0) {
		printf("No CBOXes found\n");
		test_fail(test_string);
	}

	if (!quiet) printf("Running on core %d\n",sched_getcpu());

	/**************/
	/* TEST SLEEP */
	/**************/

	if (!quiet) printf("Testing sleep:\n");

	if (!quiet) printf("\tStarting events\n");

	for(i=0;i<num_events;i++) {
		ioctl(fd[i], PERF_EVENT_IOC_RESET, 0);
		ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
		if (ret<0) {
			if (!quiet) fprintf(stderr,"Error enabling events\n");
			test_fail(test_string);
		}
   	}

	printf("\t\tsleep(1);\n");
	sleep(1);

	if (!quiet) printf("\tStopping events\n");

	for(i=0;i<num_events;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) fprintf(stderr,"Error disabling events\n");
			test_fail(test_string);
		}
	}

	if (!quiet) printf("\tReading events\n");

	for(i=0;i<num_events;i++) {
		ret=read(fd[i],&counts[i],sizeof(long long));
		if (ret!=sizeof(long long)) {
			if (!quiet) {
				fprintf(stderr,
					"Unepxected result from read fd[%d]: %d\n",i,ret);
			}
			test_fail(test_string);
		}
		if (!quiet) {
			printf("\t\tcbox%d %s %lld\n",i,uncore_event_name,counts[i]);
		}

	}

	/***************************/
	/* Testing Matrix Multiply */
	/***************************/

	if (!quiet) printf("Testing Matrix Multiply\n");

	if (!quiet) printf("\tStarting events\n");

	for(i=0;i<num_events;i++) {
		ioctl(fd[i], PERF_EVENT_IOC_RESET, 0);
		ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
		if (ret<0) {
			if (!quiet) fprintf(stderr,"Error enabling events\n");
			test_fail(test_string);
		}
   	}

	printf("\t\t");
	naive_matrix_multiply(quiet);

	if (!quiet) printf("\tStopping events\n");

	for(i=0;i<num_events;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) fprintf(stderr,"Error disabling events\n");
			test_fail(test_string);
		}
	}

	if (!quiet) printf("\tReading events\n");

	for(i=0;i<num_events;i++) {
		ret=read(fd[i],&counts[i],sizeof(long long));
		if (ret!=sizeof(long long)) {
			if (!quiet) {
				fprintf(stderr,
					"Unepxected result from read fd[%d]: %d\n",i,ret);
			}
			test_fail(test_string);
		}
		if (!quiet) {
			printf("\t\tcbox%d %s %lld\n",i,uncore_event_name,counts[i]);
		}

	}

	test_pass(test_string);

	return 0;
}
