#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "create_perf_data.h"

//#define PERF_LOCATION	"./perf"
#define PERF_LOCATION	"/home/vince/research/linux-kernel/linux.git/tools/perf/perf"

int page_size;

#define NUM_STATUSES	1024

int main(int argc, char **argv) {

	pid_t pid,w,i;
	int status;
	int iterations=0;
	int exit_statuses[NUM_STATUSES];

	char *newargv[] = { "perf-perf-perf", "report", NULL };

	char *newenviron[] = { NULL };

	memset(exit_statuses,0,NUM_STATUSES*sizeof(int));

	page_size=getpagesize();

	while(1) {

		create_perf_data_file();

		pid=fork();

		/* child */
		if (pid==0) {
			execve(PERF_LOCATION,newargv,newenviron);

			/* if we got here, execve failed */
			fprintf(stderr,"Exec failed!\n");
			return -1;
		}

		do {
			w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
			if (w<0) {
				fprintf(stderr,"Waitpid failed\n");
				return -1;
			}
			if (WIFEXITED(status)) {
				i=WEXITSTATUS(status);
				if (i>NUM_STATUSES) {
					printf("Exit status %d too high!\n",i);
				}
				else {
					exit_statuses[i]++;
				}
				//printf("exited, status=%d\n", WEXITSTATUS(status));
			} else if (WIFSIGNALED(status)) {
				printf("killed by signal %d\n", WTERMSIG(status));
				return -1;
			} else if (WIFSTOPPED(status)) {
				printf("stopped by signal %d\n", WSTOPSIG(status));
				return -1;
			} else if (WIFCONTINUED(status)) {
				printf("continued\n");
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		iterations++;

		if (iterations%1000==0) {
			printf("Done %d iterations\n",iterations);
			for(i=0;i<NUM_STATUSES;i++) {
				if (exit_statuses[i]) {
					printf("#%d=%d\n",
						i,exit_statuses[i]);
				}
			}
			printf("\n");
		}
	}

	return 0;

}
