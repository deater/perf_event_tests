/* perf_fuzzer */
/* by Vince Weaver */
/* fuzzes the perf_event system call */
/* Some code shared with the trinity fuzzer */

#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <fcntl.h>

/* Trinity Includes */
#include "shm.h"

/* perf_event_test infrastructure */
#include "../include/perf_event.h"

#include "perf_fuzzer.h"

#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_random.h"
#include "fuzzer_stats.h"

#include "fuzz_access.h"
#include "fuzz_close.h"
#include "fuzz_fork.h"
#include "fuzz_ioctl.h"
#include "fuzz_million.h"
#include "fuzz_mmap.h"
#include "fuzz_open.h"
#include "fuzz_overflow.h"
#include "fuzz_poll.h"
#include "fuzz_prctl.h"
#include "fuzz_read.h"
#include "fuzz_write.h"

#include "get_cpuinfo.h"
#include "perf_attr_print.h"

#include "pmus.h"



/* Globals from Trinity */
int page_size;
struct shm_s *shm;
char *page_rand;
unsigned int num_online_cpus;
unsigned int max_children=1;

/* Stats globals */
struct fuzzer_stats_t stats;

/* Logging globals */
int logging=0;
int trigger_failure_logging=0;
int stop_after=0;
int attempt_determinism=0;
int log_fd;
char log_buffer[BUFSIZ];

/* Determinism Globals */
struct skip_t ignore_but_dont_skip;

/* Our Globals */
struct event_data_t event_data[NUM_EVENTS];
int active_events=0;

/* Type selection */
static int type=TYPE_MMAP|
		TYPE_OVERFLOW|
		TYPE_OPEN|
		TYPE_CLOSE|
		TYPE_READ|
		TYPE_WRITE|
		TYPE_IOCTL|
		TYPE_FORK|
		TYPE_PRCTL|
		TYPE_POLL|
		TYPE_MILLION|
		TYPE_ACCESS|
		TYPE_TRASH_MMAP;


static int throttle_close_event=0;

static struct sigaction sigio;
static struct sigaction sigquit;



int find_random_active_event(void) {

	int i,x,j=0;

	if (active_events<1) return -1;

	x=rand()%active_events;

	for(i=0;i<NUM_EVENTS;i++) {
		if (event_data[i].active) {
			if (j==x) return i;
			j++;
		}
	}
	return -1;
}

int find_random_active_sampling_event(void) {

	int i,x,j=0;

	if (active_events<1) return -1;

	x=rand()%active_events;

	for(i=0;i<NUM_EVENTS;i++) {
		if ((event_data[i].active) && 
			(event_data[i].attr.sample_period)) {
			if (j==x) return i;
			j++;
		}
	}
	return -1;
}

int lookup_event(int fd) {

	int i;

	if (active_events<1) return -1;

	for(i=0;i<NUM_EVENTS;i++) {
		if (event_data[i].active) {
			if (event_data[i].fd==fd) return i;
		}
	}
	return -1;
}


int find_empty_event(void) {

	int i;

	for(i=0;i<NUM_EVENTS;i++) {
		if (!event_data[i].active) {
			return i;
		}
	}
	return -1;

}

static int next_overflow_refresh=0;
static int next_refresh=0;


#define WATCHDOG_TIMEOUT 60

static int watchdog_counter=0;

/* Set a watchdog, kill if it's been stuck too long */
static void alarm_handler(int signum, siginfo_t *info, void *uc) {

        if (watchdog_counter==0) {
		printf("Watchdog triggered; failed to progress for %d seconds;  killing\n",
			WATCHDOG_TIMEOUT);
		orderly_shutdown();
	}

        watchdog_counter=0;

        alarm(WATCHDOG_TIMEOUT);
}

/* Print status when ^\ pressed */
static void sigquit_handler(int signum, siginfo_t *info, void *uc) {

	int i;

	for(i=0;i<NUM_EVENTS;i++) {
		if (event_data[i].active) {
			perf_pretty_print_event(stdout,
				event_data[i].fd,
				getpid(),
				&event_data[i].attr,
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags
				);
		}
	}

}

static int get_sample_rate(void) {

	FILE *fff;
	int sample_rate;

	fff=fopen("/proc/sys/kernel/perf_event_max_sample_rate","r");
	if (fff==NULL) {
		return -1;
	}

	fscanf(fff,"%d",&sample_rate);

	fclose(fff);

	return sample_rate;
}

static int get_paranoid_value(void) {

	FILE *fff;
	int paranoid;

	fff=fopen("/proc/sys/kernel/perf_event_paranoid","r");
	if (fff==NULL) {
		return -1;
	}

	fscanf(fff,"%d",&paranoid);

	fclose(fff);

	return paranoid;

}


static void usage(char *name,int help) {

	printf("\nPerf Fuzzer version %s\n\n",VERSION);

	if (help) {
		printf("%s [-h] [-v] [-l filename] [-s num] [-r num] [-t OCIRMFQPpAoi]\n\n",name);
		printf("\t-h\tdisplay help\n");
		printf("\t-v\tdisplay version\n");
		printf("\t-l logfile\tlog to file filename (- for stdout)\n");
		printf("\t-r num\tseed random number generator with num\n");
		printf("\t-s num\tstop after num system calls\n");
		printf("\t-t type of calls to execute (default is all)\n");
		printf("\t\tO perf_event_open\tC close\n");
		printf("\t\tI ioctl\t\t\tR read\n");
		printf("\t\tM mmap\t\t\tF fork\n");
		printf("\t\tQ trash-mmap\t\tW write\n");
		printf("\t\tP prctl\t\t\tp poll\n");
		printf("\t\tA file access\t\to overflow\n");
		printf("\t\ti instruction loop\n");
		printf("\n");
	}
}

int main(int argc, char **argv) {

	int i;
	char *logfile_name=NULL;
	unsigned int seed=0;
	int sample_rate,paranoid;
	FILE *fff;
	struct utsname uname_info;
	char cpuinfo[BUFSIZ];
	int seed_specified=0;

	/* Parse command line parameters */

	if (argc>1) {
		i=1;
		while(1) {
			if (i>=argc) break;

			if(argv[i][0]=='-') {
				switch(argv[i][1]) {
				/* help */
				case 'h':	usage(argv[0],1);
						exit(0);
						break;
				/* version */
				case 'v':	usage(argv[0],0);
						exit(0);
						break;
				/* log */
                                case 'l':       logging=TYPE_ALL;
						i++;
						if (i<argc) {
							logfile_name=strdup(argv[i]);
						}
						else {
							logfile_name=strdup("out.log");
						}
						i++;
						break;
				/* seed */
				case 'r':	if (i+1<argc) {
							seed=atoi(argv[i+1]);
						}
						seed_specified=1;
						printf("Using user-specified random seed of %d\n",seed);
						i+=2;
						break;
				/* stop */
				case 's':	if (i+1<argc) {
							stop_after=atoi(argv[i+1]);
						}
						printf("Stopping after %d\n",stop_after);
						i+=2;
						break;
				/* type */
				case 't':	{
						int j;

						type=0;

						for(j=0;j<strlen(argv[i+1]);j++) {
						switch(argv[i+1][j]) {
						case 'O': type|=TYPE_OPEN; break;
						case 'C': type|=TYPE_CLOSE; break;
						case 'I': type|=TYPE_IOCTL; break;
						case 'R': type|=TYPE_READ; break;
						case 'M': type|=TYPE_MMAP; break;
						case 'F': type|=TYPE_FORK; break;
						case 'Q': type|=TYPE_TRASH_MMAP; break;
						case 'W': type|=TYPE_WRITE; break;
						case 'P': type|=TYPE_PRCTL; break;
						case 'p': type|=TYPE_POLL; break;
						case 'A': type|=TYPE_ACCESS; break;
						case 'o': type|=TYPE_OVERFLOW; break;
						case 'i': type|=TYPE_MILLION; break;
						default: printf("Unknown type %c\n",
							argv[i+1][j]);
						}
						}
						}
						i+=2;
						break;
				default:	fprintf(stderr,"Unknown parameter %s\n",argv[1]);
						usage(argv[0],1);
						exit(1);
						break;
				}

			}
			else {
				fprintf(stderr,"Unknown parameter %s\n",argv[1]);
				usage(argv[0],1);
				exit(1);
			}

		}

	} else {
		/* Use defaults */
		logging=0;
	}

	/* TODO: Make these configurable */
	printf("Watchdog enabled with timeout %ds\n",WATCHDOG_TIMEOUT);
	printf("Will auto-exit if signal storm detected\n");

	if (logging) {

		if (!strcmp(logfile_name,"-")) {
			log_fd=1;		/* stdout */
		}
		else {
			log_fd=open(logfile_name,O_WRONLY|O_CREAT,0660);
			if (log_fd<0) {
				fprintf(stderr,"Error opening %s: %s\n",
					logfile_name,strerror(errno));
				exit(1);
			}
			fprintf(stderr,"Warning! Using a named log file might disrupt determinism due to the extra file descriptor created.  Consider logging to stdout instead\n\n");
		}
	}

	printf("\n*** perf_fuzzer %s *** by Vince Weaver\n\n",VERSION);

	uname(&uname_info);

	printf("\t%s version %s %s\n",
		uname_info.sysname,uname_info.release,uname_info.machine);

	get_cpuinfo(cpuinfo);
	printf("\tProcessor: %s\n",cpuinfo);

	/* Poor Seeding */
	/* should read /dev/urandom instead */
	if (!seed) {
		seed=time(NULL);
	}
	srand(seed);
	printf("\tSeeding random number generator with %d\n",seed);

	/* setup watchdog timer */
	/* FIXME: make optional */
        struct sigaction watchdog;

        memset(&watchdog, 0, sizeof(struct sigaction));
        watchdog.sa_sigaction = alarm_handler;
        watchdog.sa_flags = SA_SIGINFO | SA_RESTART;

        if (sigaction( SIGALRM, &watchdog, NULL) < 0) {
                printf("Error setting up alarm handler\n");
        }

        alarm(WATCHDOG_TIMEOUT);


	/* Clear errnos count */
	for(i=0;i<MAX_ERRNOS;i++) {
		stats.open_errno_count[i]=0;
		stats.fork_errno_count[i]=0;
	}

	/* Clear type counts */
	for(i=0;i<MAX_OPEN_TYPE;i++) {
		stats.open_type_success[i]=0;
		stats.open_type_fail[i]=0;
	}

	/* Write seed to disk so we can find it later */
	fff=fopen("last.seed","w");
	if (fff!=NULL) {
		fprintf(fff,"%d\n",seed);
		fclose(fff);
	}

	if (logging) {
		sprintf(log_buffer,"S %d\n",seed);
		write(log_fd,log_buffer,strlen(log_buffer));
	}

	/* Save our pid so we can re-map on replay */
	if (logging) {
		sprintf(log_buffer,"G %d\n",getpid());
		write(log_fd,log_buffer,strlen(log_buffer));
	}

	/* Save the content of /proc/sys/kernel/perf_event_max_sample_rate */
	/* If it has been changed, a replay might not be perfect */
	sample_rate=get_sample_rate();
	printf("\t/proc/sys/kernel/perf_event_max_sample_rate currently: %d/s\n",
		sample_rate);
	if (logging) {
		sprintf(log_buffer,"r %d\n",sample_rate);
		write(log_fd,log_buffer,strlen(log_buffer));
	}

	/* Check paranoid setting */
	paranoid=get_paranoid_value();
	printf("\t/proc/sys/kernel/perf_event_paranoid currently: %d\n",
		paranoid);

	printf("\tLogging perf_event_open() failures: %s\n",
		LOG_FAILURES?"yes":"no");
	printf("\tRunning fsync after every syscall: %s\n",
		FSYNC_EVERY?"yes":"no");

	/* Print command line */
	printf("\tTo reproduce, try: ");
	for(i=0;i<argc;i++) {
		printf("%s ",argv[i]);
	}

	if (!seed_specified) printf("-r %d",seed);

	printf("\n\n");

	if (attempt_determinism) {
		type&=~TYPE_OVERFLOW;
	}

	printf("Pid=%d, sleeping 1s\n",getpid());
	sleep(1);


	/* Print what we are actually fuzzing */
	/* Sometimes I comment out code and forget */

	printf("==================================================\n");

	printf("Fuzzing the following syscalls:\n\t");
	if (type&TYPE_MMAP) printf("mmap ");
	if (type&TYPE_OPEN) printf("perf_event_open ");
	if (type&TYPE_CLOSE) printf("close ");
	if (type&TYPE_READ) printf("read ");
	if (type&TYPE_WRITE) printf("write ");
	if (type&TYPE_IOCTL) printf("ioctl ");
	if (type&TYPE_FORK) printf("fork ");
	if (type&TYPE_PRCTL) printf("prctl ");
	if (type&TYPE_POLL) printf("poll ");
	printf("\n");

	printf("*NOT* Fuzzing the following syscalls:\n\t");
	if (!(type&TYPE_MMAP)) printf("mmap ");
	if (!(type&TYPE_OPEN)) printf("perf_event_open ");
	if (!(type&TYPE_CLOSE)) printf("close ");
	if (!(type&TYPE_READ)) printf("read ");
	if (!(type&TYPE_WRITE)) printf("write ");
	if (!(type&TYPE_IOCTL)) printf("ioctl ");
	if (!(type&TYPE_FORK)) printf("fork ");
	if (!(type&TYPE_PRCTL)) printf("prctl ");
	if (!(type&TYPE_POLL)) printf("poll ");
	printf("\n");

	printf("Also attempting the following:\n\t");
	if (type&TYPE_OVERFLOW) printf("signal-handler-on-overflow ");
	if (type&TYPE_MILLION) printf("busy-instruction-loop ");
	if (type&TYPE_ACCESS) printf("accessing-perf-proc-and-sys-files ");
	if (type&TYPE_TRASH_MMAP) printf("trashing-the-mmap-page ");
	printf("\n");

	printf("*NOT* attempting the following:\n\t");
	if (!(type&TYPE_OVERFLOW)) printf("signal-handler-on-overflow ");
	if (!(type&TYPE_MILLION)) printf("busy-instruction-loop ");
	if (!(type&TYPE_ACCESS)) printf("accessing-perf-proc-and-sys-files ");
	if (!(type&TYPE_TRASH_MMAP)) printf("trashing-the-mmap-page ");
	printf("\n");

	if (attempt_determinism) {
		printf("\nAttempting more deterministic results by:\n\t");
		printf("waitpid-after-killing-child ");
		printf("disabling-overflow-signal-handler ");
		printf("\n");
	}


	printf("==================================================\n");



	/* Set up to match trinity setup, vaguely */
	page_size=getpagesize();
	num_online_cpus = sysconf(_SC_NPROCESSORS_ONLN);

	create_shm();
	create_shm_arrays();
	init_shm();

//	init_shared_pages();

	syscall_perf_event_open.init();

	/* Initialize PMU names if possible */
	/* This depends on trinity exporting the values */
	if (pmus!=NULL) {
		for(i=0;i<num_pmus;i++) {
			if (pmus[i].type<MAX_OPEN_TYPE) {
				stats_set_pmu_name(pmus[i].type,pmus[i].name);
			}
		}
	}


	/* Set up SIGIO handler */
	/* In theory we shouldn't get SIGIO as we set up SIGRT for overflow */
	/* But if the RT queue overflows we will get a SIGIO */
	memset(&sigio, 0, sizeof(struct sigaction));
	sigio.sa_sigaction = sigio_handler;
	sigio.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sigio, NULL) < 0) {
		printf("Error setting up SIGIO signal handler\n");
     	}


	/* Set up SIGQUIT handler */
	memset(&sigquit, 0, sizeof(struct sigaction));
	sigquit.sa_sigaction = sigquit_handler;
	sigquit.sa_flags = SA_SIGINFO;

	if (sigaction( SIGQUIT, &sigquit, NULL) < 0) {
		printf("Error setting up SIGQUIT signal handler\n");
     	}


	/* Initialize */
	for(i=0;i<NUM_EVENTS;i++) {
		event_data[i].active=0;
		event_data[i].fd=0;
		event_data[i].read_size=rand();
	}

	while(1) {

		switch(rand()%11) {
			case 0:	if (type&TYPE_OPEN) {
					open_random_event(type&TYPE_MMAP,
							type&TYPE_OVERFLOW);
				}
				break;
			case 1: if (type&TYPE_CLOSE) {
//					if (rand()%3==0)
						close_random_event();
				}
				break;
			case 2: if (type&TYPE_IOCTL) {
					ioctl_random_event();
				}
				break;
			case 3: if (type&TYPE_PRCTL) {
					prctl_random_event();
				}
				break;
			case 4: if (type&TYPE_READ) {
					read_random_event();
				}
				break;
			case 5: if (type&TYPE_WRITE) {
					write_random_event();
				}
				break;
			case 6: if (type&TYPE_ACCESS) {
					access_random_file();
				}
				break;
			case 7: if (type&TYPE_FORK) {
					fork_random_event();
				}
				break;
			case 8: if (type&TYPE_POLL) {
					poll_random_event();
				}
				break;
			case 9:if (type&TYPE_MMAP) {
					mmap_random_event(type);
				}
				break;
			default:
				if (type&TYPE_MILLION) {
					run_a_million_instructions();
				}
				break;
		}

#if FSYNC_EVERY
		if (logging) fsync(log_fd);
#endif

		if (throttle_close_event) {
			printf("Closing stuck event %d\n",
				throttle_close_event);
			close_event(throttle_close_event,1);
			throttle_close_event=0;
		}

		next_overflow_refresh=rand()%2;
		next_refresh=rand_refresh();
		stats.total_iterations++;
		watchdog_counter++;

		if ((stop_after) && (stats.total_iterations>=stop_after)) {
			dump_summary(stderr,1);

			/* Kill child, doesn't happen automatically? */
			if (already_forked) {
				int status;
				kill(forked_pid,SIGKILL);
				waitpid(forked_pid, &status, 0);
			}
			return 0;
		}

		/* Print status update every 10000 iterations      */
		/* Don't print if logging to stdout as it clutters */
		/* up the trace file.				   */
		if (stats.total_iterations%10000==0) {
			if (log_fd!=1) {
				dump_summary(stderr,1);
			}
			else {
				dump_summary(stderr,0);
			}
		}
//		fsync(log_fd);
	}

	return 0;

}
