/* This tests CBOX Uncore Events on SNB and IVB     */
/* These weren't supported until at least Linxu 3.6 */
/* This particular test sees if we can add them to a group */

/* by Vince Weaver, vincent.weaver _at_ maine.edu        */

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

char test_string[]="Testing grouping cbox uncore events...";

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
   int group_fd;

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
           strcpy(uncore_event_name,"UNC_CBO_CACHE_LOOKUP:STATE_MESI:ANY_FILTER");
           uncore_event=0x508f34;
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

				/* uncore so can't exclude */
				pe.exclude_user=0;
				pe.exclude_kernel=0;
				pe.exclude_hv=0;

				if (num_events==0) {
					group_fd=-1;
				}
				else {
					group_fd=fd[0];
				}
				if (group_fd==-1) pe.disabled=1;

				fd[num_events]=perf_event_open(&pe,
					-1, /* pid == all pids */
					0,  /* cpu = only CPU0 */
					group_fd,0);
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

	if (!quiet) printf("Starting events\n");

	for(i=0;i<num_events;i++) {
		ioctl(fd[i], PERF_EVENT_IOC_RESET, 0);
		ret=ioctl(fd[i], PERF_EVENT_IOC_ENABLE,0);
		if (ret<0) {
			if (!quiet) fprintf(stderr,"Error enabling events\n");
			test_fail(test_string);
		}
   	}

	printf("\t\tRunning on core %d\n",sched_getcpu());
	printf("\t\t");
	naive_matrix_multiply(quiet);

	if (!quiet) printf("Stopping events\n");

	for(i=0;i<num_events;i++) {
		ret=ioctl(fd[i], PERF_EVENT_IOC_DISABLE,0);
		if (ret<0) {
			if (!quiet) fprintf(stderr,"Error disabling events\n");
			test_fail(test_string);
		}
	}

	if (!quiet) printf("Reading events\n");

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
			printf("\tcbox%d %s %lld\n",i,uncore_event_name,counts[i]);
		}

	}

	test_pass(test_string);

	return 0;
}
