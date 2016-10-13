#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#include <signal.h>

#include "perf_event.h"
#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"
#include "fuzz_access.h"
#include "fuzzer_random.h"

#define BUFFER_SIZE 9000

static char buffer[BUFFER_SIZE];

#define MAX_FILES 5

static int open_fds[MAX_FILES];
static int open_files=0;

#define MAX_FILENAMES	6

static char *filenames[MAX_FILENAMES]={
	"/proc/sys/kernel/perf_cpu_time_max_percent",
	"/proc/sys/kernel/perf_event_paranoid",
	"/proc/sys/kernel/perf_event_max_sample_rate",
	"/proc/sys/kernel/perf_event_max_stack",
	"/proc/sys/kernel/perf_event_mlock_kb",
	"/proc/sys/kernel/nmi_watchdog"
};

static int setup_random_write_value(void) {

	int type=rand()%3;
	int i,size;
	char string[BUFSIZ];

	/* single small integer */
	if (type==0) {
		switch(rand()%5) {
			case 0: sprintf(buffer,"%d",-1); break;
			case 1: sprintf(buffer,"%d",0); break;
			case 2: sprintf(buffer,"%d",1); break;
			case 3: sprintf(buffer,"%d",2); break;
			case 4: sprintf(buffer,"%d",3); break;
		}
		size=strlen(buffer);
	}

	/* random series of integers */
	if (type==1) {
		buffer[0]=0;
		for(i=0;i<rand()%4;i++) {
			sprintf(string,"%d",rand());
			strcat(buffer,string);
		}
		size=strlen(string);
	}

	/* completely random */
	if (type==2) {
		size=rand()%BUFFER_SIZE;
		for(i=0;i<size;i++) buffer[i]=rand();
	}

	return size;
}

void access_random_file(void) {

	int which_file;
	int fd,new_fd;
	int result;
	int read_size,write_size;
	int which;

	/* These files typically are owned by root */
	/* So this should never trigger any bugs   */
	/* unless running as root (a bad idea)	   */

	stats.access_attempts++;

	which_file=rand()%MAX_FILENAMES;
	read_size=rand()%BUFFER_SIZE;

	/* FIXME: move write_value code out here if possible */

	/* Randomly pick read/write/just-open/just-close */
	which=rand()%4;

	if (which==0) {

		if (ignore_but_dont_skip.access) return;

		/* read.  Make flags random too? */
		fd=open(filenames[which_file],rand_open_flags());
		if (fd>0) {
			result=read(fd,buffer,read_size);

			if (logging&TYPE_ACCESS) {
				sprintf(log_buffer,"A 0 %d %d %d\n",
					which_file,read_size,result);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			if (result>0) stats.access_successful++;

			close(fd);
		}
	}
	else if (which==1) {
		/* write */
		fd=open(filenames[which_file],rand_open_flags());
		if (fd>0) {
			if (!ignore_but_dont_skip.access) {

			write_size=setup_random_write_value();

			result=write(fd,buffer,write_size);

			if (logging&TYPE_ACCESS) {
				sprintf(log_buffer,"A 1 %d %d %d\n",
					which_file,write_size,result);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			if (result>0) stats.access_successful++;
			}

			close(fd);
		}
	} else if (which==2) {
		/* Leave open a certain number */
		if (open_files<MAX_FILES) {
			new_fd=open(filenames[which_file],rand_open_flags());
			if (new_fd>=0) {
				for(which_file=0;which_file<TYPE_ACCESS;
					which_file++) {
					if (open_fds[which_file]==0) break;
				}
				if (which_file==TYPE_ACCESS) {
					fprintf(stderr,"WHICH FILE TOO BIG!\n");
				}
				open_fds[which_file]=new_fd;
				open_files++;
				stats.access_successful++;
				if (logging&TYPE_ACCESS) {
					sprintf(log_buffer,"A 2 %d\n",
						which_file);
					write(log_fd,log_buffer,strlen(log_buffer));
				}
			}
		}

	} else {
		/* close a random open */
		which_file=rand()%MAX_FILES;
		if (open_fds[which_file]) {
			close(open_fds[which_file]);
			open_fds[which_file]=0;
			open_files--;
			stats.access_successful++;
			if (logging&TYPE_ACCESS) {
				sprintf(log_buffer,"A 3 %d\n",
					which_file);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

		}
	}


	/****************************************/
	/* TODO -- try some files under 	*/
	/*	/sys/bus/event_source/devices/	*/
	/****************************************/

}

