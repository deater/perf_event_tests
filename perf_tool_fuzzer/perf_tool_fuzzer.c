#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#include "create_perf_data.h"

int main(int argc, char **argv) {

	pid_t pid,w;
	int status;

	char *newargv[] = { NULL, "report", NULL };
	char *newenviron[] = { NULL };


	while(1) {

		create_perf_data_file();

		pid=fork();

		/* child */
		if (pid==0) {
			execve("./perf",newargv,newenviron);

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
                		printf("exited, status=%d\n", WEXITSTATUS(status));
			} else if (WIFSIGNALED(status)) {
				printf("killed by signal %d\n", WTERMSIG(status));
			} else if (WIFSTOPPED(status)) {
				printf("stopped by signal %d\n", WSTOPSIG(status));
			} else if (WIFCONTINUED(status)) {
				printf("continued\n");
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}

	return 0;

}
