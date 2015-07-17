#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>

#include "perf_event.h"
#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"
#include "fuzz_access.h"

void access_random_file(void) {

	int which_file;
	FILE *fff;
	char buffer[2048];
	int result;
	int size;
	long long write_value;

#define MAX_FILENAMES	4

	char *filenames[MAX_FILENAMES]={
		"/proc/sys/kernel/perf_cpu_time_max_percent",
		"/proc/sys/kernel/perf_event_paranoid",
		"/proc/sys/kernel/perf_event_max_sample_rate",
		"/proc/sys/kernel/perf_event_mlock_kb",
	};

	/* These files typically are owned by root */
	/* So this should never trigger any bugs   */
	/* unless running as root (a bad idea)	   */

	stats.access_attempts++;

	which_file=rand()%MAX_FILENAMES;

	/* FIXME: move write_value code out here if possible */

	if (rand()%2) {

		if (ignore_but_dont_skip.access) return;

		/* read */
		fff=fopen(filenames[which_file],"r");
		if (fff!=NULL) {
			size=2048;
			result=fread(buffer,sizeof(char),size,fff);

			if (logging&TYPE_ACCESS) {
				sprintf(log_buffer,"A 0 %d %d %d\n",
					which_file,size,result);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			if (result>0) stats.access_successful++;

			fclose(fff);
		}
	}
	else {
		/* write */
		fff=fopen(filenames[which_file],"w");
		if (fff!=NULL) {
			write_value=((unsigned long long)rand()<<32)|rand();


		if (!ignore_but_dont_skip.access) {
			result=fprintf(fff,"%lld\n",write_value);

			if (logging&TYPE_ACCESS) {
				sprintf(log_buffer,"A 1 %d %lld %d\n",
					which_file,write_value,result);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			if (result>0) stats.access_successful++;
		}

			fclose(fff);
		}
	}


	/****************************************/
	/* TODO -- try some files under 	*/
	/*	/sys/bus/event_source/devices/	*/
	/****************************************/

}

