/* perf_fuzzer */
/* by Vince Weaver */
/* fuzzes the perf_event system call */
/* Some code shared with the trinity fuzzer */

#include "version.h"

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/utsname.h>

#include <fcntl.h>

/* Trinity Includes */
#include "shm.h"
#include "sanitise.h"
#include "syscall.h"
#include "tables.h"

/* perf_event_test infrastructure */
#include "../include/perf_event.h"
#include "../include/perf_helpers.h"
#include "../include/instructions_testcode.h"

#include "fuzz_compat.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_random.h"
#include "fuzzer_stats.h"

#include "fuzz_ioctl.h"
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
unsigned int get_cpu(void);

struct fuzzer_stats_t stats;


#define MAX_THROTTLES		10

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

static int already_forked=0;
static pid_t forked_pid;

static struct sigaction sigio;
static struct sigaction sigquit;

extern struct syscallentry syscall_perf_event_open;

static int active_events=0;

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

static int lookup_event(int fd) {

	int i;

	if (active_events<1) return -1;

	for(i=0;i<NUM_EVENTS;i++) {
		if (event_data[i].active) {
			if (event_data[i].fd==fd) return i;
		}
	}
	return -1;
}


static int find_empty_event(void) {

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

static long long prev_head=0;

long long perf_mmap_read( void *our_mmap,
				int mmap_size,
				long long prev_head) {

	struct perf_event_mmap_page *control_page = our_mmap;
	long long head;

	if (!our_mmap) return 0;

	if (mmap_size==0) return 0;

	if (control_page==NULL) {
		return -1;
	}
	head=control_page->data_head;
	rmb(); /* Must always follow read of data_head */

	/* Mark all as read */
	control_page->data_tail=head;

	return head;
}


static void close_event(int i, int from_sigio) {

	int result;
	int unmap_before_close;

	/* Exit if no events */
	if (i<0) return;

	unmap_before_close=rand()%2;

	if (ignore_but_dont_skip.close) return;

	/* here to avoid race in overflow where we munmap or close */
	/* but event still marked active */

	event_data[i].active=0;

	/* unmap any associated memory */
	/* we tried not doing this randomly, */
	/* but rapidly ran out of memory */
	if ((event_data[i].mmap) && unmap_before_close) {
		munmap(event_data[i].mmap,event_data[i].mmap_size);
		if ((!from_sigio) && (logging&TYPE_MMAP)) {
			sprintf(log_buffer,"U %d %d %p\n",
				event_data[i].fd,
				event_data[i].mmap_size,
				event_data[i].mmap);
			write(log_fd,log_buffer,strlen(log_buffer));
		}
		event_data[i].mmap=0;
	}

	stats.close_attempts++;
	result=close(event_data[i].fd);
	if (result==0) {
		stats.close_successful++;
		stats.current_open--;
	}

	if ((!from_sigio) && (logging&TYPE_CLOSE)) {
		sprintf(log_buffer,"C %d\n",event_data[i].fd);
		write(log_fd,log_buffer,strlen(log_buffer));
	}

	/* sometimes unmap after we've closed */
	if ((event_data[i].mmap) && !unmap_before_close) {
		munmap(event_data[i].mmap,event_data[i].mmap_size);
		if ((!from_sigio) && (logging&TYPE_MMAP)) {
			sprintf(log_buffer,"U %d %d %p\n",
				event_data[i].fd,
				event_data[i].mmap_size,
				event_data[i].mmap);
			write(log_fd,log_buffer,strlen(log_buffer));
		}
		event_data[i].mmap=0;
	}

	active_events--;


}

static void close_random_event(void) {

	int i;

	i=find_random_active_event();

	close_event(i,0);

}


/* Try to exit without leaving any children around */
static void orderly_shutdown(void) {

	int status;

	if (already_forked) {
		printf("Trying to cleanly shudtdown pid %d\n",forked_pid);
		kill(forked_pid,SIGKILL);
		waitpid(forked_pid, &status, 0);
		printf("Done...\n");
	}

	printf("Trying to shut ourselves down: %d, last child %d\n",
			getpid(),forked_pid);

	exit(1);
}


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







/* The perf tool uses poll() and never sets signals */
/* Thus they never have most of these problems      */
static void our_handler(int signum, siginfo_t *info, void *uc) {

	static int already_handling=0;

	int fd = info->si_fd;
	int i;
	int ret;


	/* In some cases (syscall tracepoint) */
	/* The act of disabling an event would trigger */
	/* Another overflow, leading to a recursive storm */
	if (already_handling) return;

	already_handling=1;

	stats.overflows++;

	/* disable the event for the time being */
	/* we were having trouble with signal storms */
	ret=ioctl(fd,PERF_EVENT_IOC_DISABLE,0);
	/* Do not log, logging only make sense if */
	/* we have deterministic counts which we don't */

	/* Somehow we got a signal from an invalid event? */
	/* How would this happen?			*/
	/* Looks like if we fork() then close an event, */
	/* It can still be alive in the child and cause */
	/* a signal to come in even though it is closed.*/
	if (ret<0) {
		printf("Signal from invalid fd %d %s\n",
			fd,strerror(errno));
		orderly_shutdown();
	}




	i=lookup_event(fd);

	if (i>=0) {

		event_data[i].overflows++;

		if (event_data[i].overflows>10000) {
			if (!logging) printf("Throttling event %d fd %d, last_refresh=%d, "
				"period=%llu, type=%d throttles %d\n",
				i,event_data[i].fd,event_data[i].last_refresh,
				event_data[i].attr.sample_period,
				event_data[i].attr.type,
				event_data[i].throttles);
			event_data[i].overflows=0;
			event_data[i].throttles++;

			/* otherwise if we re-trigger next time */
			/* with >1 refresh the throttle never   */
			/* lasts a significant amount of time.  */
			next_refresh=0;

			/* Avoid infinite throttle storms */
			if (event_data[i].throttles > MAX_THROTTLES) {

				printf("Stuck in a signal storm w/o forward progress; Max throttle count hit, giving up\n");
				orderly_shutdown();

				/* In a storm we used to try to somehow stop */
				/* it by closing all events, but this never  */
				/* really worked.			     */
#if 0
				/* Disable all events */
				printf("Trying to disable all events\n");
				for(j=0;j<NUM_EVENTS;j++) {
					if (event_data[j].active) {
						ioctl(event_data[j].fd,PERF_EVENT_IOC_DISABLE,0);
					}
				}

				throttle_close_event=i;
#endif
			}
		}

		else {

			/* read the event */
			prev_head=perf_mmap_read(event_data[i].mmap,
					event_data[i].mmap_size,
					prev_head);

			/* cannot call rand() from signal handler! */
			/* we re-enter and get stuck in a futex :( */

			ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,next_refresh);
			if (ret==0) {
				event_data[i].last_refresh=next_refresh;
			}
			/* Do not log, makes no sense */
		}
	}

	already_handling=0;

	(void) ret;

}

static void sigio_handler(int signum, siginfo_t *info, void *uc) {

	int fd = info->si_fd;
	long band = info->si_band;
	int code = info->si_code;
	int i;

	if (code & SI_KERNEL) {
		/* We overflowed the RT signal queue and the kernel	*/
		/* has rudely let us know w/o telling us which event	*/
		/* Close a random event in hopes of stopping		*/
		if (!logging) printf("SIGIO due to RT queue overflow\n");

		/* Close all events? */
		/* Extreme, but easier to re-play */
		/* Does make for non-deterministic traces :( */

		for(i=0;i<NUM_EVENTS;i++) {
			if (event_data[i].active) close_event(i,1);
		}
	}
	else {
		printf("SIGIO from fd %d band %lx code %d\n",fd,band,code);
	}

	stats.sigios++;
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

void perf_log_attr(struct perf_event_attr *attr) {

	sprintf(log_buffer,"%x ",attr->type);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%x ",attr->size);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%llx ",attr->config);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%llx ",attr->sample_period);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%llx ",attr->sample_type);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%llx ",attr->read_format);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->disabled);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->inherit);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->pinned);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclusive);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_user);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_kernel);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_hv);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_idle);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->mmap);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->comm);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->freq);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->inherit_stat);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->enable_on_exec);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->task);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->watermark);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->precise_ip);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->mmap_data);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->sample_id_all);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_host);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_guest);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->wakeup_events);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->bp_type);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%llx ",attr->config1);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%llx ",attr->config2);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%lld ",attr->branch_sample_type);
	write(log_fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%d ",attr->exclude_callchain_kernel);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_callchain_user);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%lld ",attr->sample_regs_user);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->sample_stack_user);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->mmap2);
	write(log_fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->comm_exec);
	write(log_fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"\n");
	write(log_fd,log_buffer,strlen(log_buffer));
	sync();

}

static int update_read_size(int i) {

	int read_size=1;

	/* Set nominal read size */
	if (event_data[i].attr.read_format&PERF_FORMAT_GROUP) {
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_ENABLED) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_RUNNING) read_size++;
		read_size+=(1+!!(event_data[i].attr.read_format&PERF_FORMAT_ID)) * event_data[i].number_in_group;
	}
	else {
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_ENABLED) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_RUNNING) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_ID) read_size++;
	}

	return read_size*sizeof(long long);
}

static void open_random_event(void) {

	int fd;

	int i;

	i=find_empty_event();

	/* return if no free events */
	if (i<0) return;

	event_data[i].overflows=0;
	event_data[i].throttles=0;


	/* repeat until we create a valid event */
	while(1) {
		/* call trinity random perf_event_open() code */
		//generic_sanitise(0);
		syscall_perf_event_open.sanitise(&shm->syscall[0]);

		memcpy(&event_data[i].attr,
			(struct perf_event_attr *)shm->syscall[0].a1,
			sizeof(struct perf_event_attr));
		event_data[i].pid=shm->syscall[0].a2;
		event_data[i].cpu=get_cpu();
		event_data[i].group_fd=shm->syscall[0].a4;
		event_data[i].flags=shm->syscall[0].a5;

		/* Randomly make part of a group 1/4 of the time */
		if (rand()%4==2) {
			int j;
			j=find_random_active_event();

			/* is it a master? */
			/* can we set a group leader that isn't itself */
			/* a leader? */
//			if (event_data[j].group_fd==-1) {
			event_data[i].group_fd=event_data[j].fd;
//			}

		}

//		if (event_data[i].attr.type==6) {
//			event_data[i].pid=-1;
//			event_data[i].cpu=0;
//			event_data[i].flags=0;
//			event_data[i].group_fd=-1;
//			memset(&event_data[i].attr,0,sizeof(struct perf_event_attr));
//			event_data[i].attr.type=6;
//			event_data[i].attr.config=2;
//		}

		/* Debugging code */
		/* We don't usually log failed opens as there are so many */

	if (ignore_but_dont_skip.open) return;

	        if (logging&TYPE_OPEN) {
#if LOG_FAILURES
		if (trigger_failure_logging) {
		  /* uncomment if failing opens are causing crashes */
//			static int quit_next=0;
//			if (event_data[i].attr.type==PERF_TYPE_TRACEPOINT) {
	                sprintf(log_buffer,"# O -1 %d %d %d %lx ",
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
			write(log_fd,log_buffer,strlen(log_buffer));
	                perf_log_attr(&event_data[i].attr);
//			fsync(log_fd);
//			}
//			if (quit_next==1) exit(1);

//			if (quit_next) quit_next--;

//		        if ((event_data[i].attr.read_format==0x2d2d2d))
//                      quit_next=2;

		        // if ((event_data[i].group_fd==152) &&
                        //    (event_data[i].flags==0x800e9e9)) quit_next=1;
		}
#endif
	        }

		/* Actually try to open the event */
		fd=perf_event_open(
			&event_data[i].attr,
			event_data[i].pid,
			event_data[i].cpu,
			event_data[i].group_fd,
			event_data[i].flags);
		stats.open_attempts++;

		int which_type=event_data[i].attr.type;

		if ((which_type<0) || (which_type>MAX_OPEN_TYPE-1)) {
			which_type=MAX_OPEN_TYPE-1;
		}


		/* If we succede, break out of the infinite loop */
		if (fd>0) {
			stats.open_type_success[which_type]++;
			break;
		}
#if 0
		/* Track source of UNKNOWN errnos */
		if (errno==16) {
			printf("Event t=%d c=%llx pid=%d cpu=%d %s\n",
				event_data[i].attr.type,
				event_data[i].attr.config,
				event_data[i].pid,
				event_data[i].cpu,
				strerror(errno));
		}
#endif

		/* Otherwise, track the errors */
		if (errno<MAX_ERRNOS) {
			stats.errno_count[errno]++;
			stats.open_type_fail[which_type]++;
		}

		/* no more file descriptors, so give up */
		if (errno==EMFILE) return;

	}

	/* We successfully opened an event! */

	stats.open_successful++;
	stats.current_open++;

	if (logging&TYPE_OPEN) {
		sprintf(log_buffer,"O %d %d %d %d %lx ",
				fd,
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
		write(log_fd,log_buffer,strlen(log_buffer));
		perf_log_attr(&event_data[i].attr);
#if FSYNC_EVERY
		fsync(log_fd);
#endif
	}

	event_data[i].fd=fd;
	event_data[i].active=1;
	active_events++;

	/* if we are member of a group, update size of group */
	/* this is needed for calcuating "proper" read size  */
	/* Also I don't think we adjust this on close        */
	if (event_data[i].group_fd!=-1) {
		int j=lookup_event(event_data[i].group_fd);

		event_data[j].number_in_group++;
		event_data[j].read_size=update_read_size(j);
	}

	/* Setup mmap buffer */

	if (type & TYPE_MMAP) {

		/* to be valid we really want to be 1+2^x pages */
		switch(rand()%3) {
			case 0:	event_data[i].mmap_size=(rand()%64)*getpagesize();
				break;
			case 1: event_data[i].mmap_size=
					(1 + (1<<rand()%10) )*getpagesize();
				break;
			default: event_data[i].mmap_size=rand()%65535;
		}

		event_data[i].mmap=NULL;

if (!ignore_but_dont_skip.mmap) {

		stats.mmap_attempts++;
		event_data[i].mmap=mmap(NULL, event_data[i].mmap_size,
			PROT_READ|PROT_WRITE, MAP_SHARED, event_data[i].fd, 0);

		if (event_data[i].mmap==MAP_FAILED) {
			event_data[i].mmap=NULL;
#if LOG_FAILURES
			if (logging&TYPE_MMAP) {
			if (trigger_failure_logging) {
 				sprintf(log_buffer,"# M %d %d %p\n",
					event_data[i].mmap_size,event_data[i].fd,
					event_data[i].mmap);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			}
#endif
		}
		else {

			if (logging&TYPE_MMAP) {
 				sprintf(log_buffer,"M %d %d %p\n",
					event_data[i].mmap_size,event_data[i].fd,
					event_data[i].mmap);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			stats.mmap_successful++;
		}
}
	}

	/* Setup overflow 50% of the time */
	if ((type&TYPE_OVERFLOW) && (rand()%2)) {

	if (!ignore_but_dont_skip.overflow) {

		if (logging&TYPE_OVERFLOW) {
			sprintf(log_buffer,"o %d\n",event_data[i].fd);
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		memset(&event_data[i].sa, 0, sizeof(struct sigaction));
		event_data[i].sa.sa_sigaction = our_handler;
		event_data[i].sa.sa_flags = SA_SIGINFO;

		if (sigaction( SIGRTMIN+2, &event_data[i].sa, NULL) < 0) {
			printf("Error setting up signal handler\n");
     		}

		fcntl(event_data[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(event_data[i].fd, F_SETSIG, SIGRTMIN+2);
		fcntl(event_data[i].fd, F_SETOWN,getpid());
	}

	}

	event_data[i].number_in_group=1;

	event_data[i].read_size=update_read_size(i);

}


/* The first mmap() page is writeable so you can set the tail pointer */
/* So try over-writing it to see what happens.                        */

static void trash_random_mmap(void) {

	int i,value;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	if ((event_data[i].mmap)) {// && (rand()%2==1)) {

		value=rand();

		if (ignore_but_dont_skip.trash_mmap) return;

		/* can't write high pages? */
		//event_data[i].mmap_size);

//		*(event_data[i].mmap)=0xff;

		memset(event_data[i].mmap,value, 1);//getpagesize());

		if (logging&TYPE_TRASH_MMAP) {
			sprintf(log_buffer,"Q %d %d %d\n",
				value,
				getpagesize(),
				event_data[i].fd);
			write(log_fd,log_buffer,strlen(log_buffer));
		}


	}

	stats.trash_mmap_attempts++;
	stats.trash_mmap_successful++;

}

static void access_random_file(void) {

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

static void run_a_million_instructions(void) {

	if (ignore_but_dont_skip.million) return;

	instructions_million();

}



static void fork_random_event(void) {

	int status;

	if (ignore_but_dont_skip.fork) return;

	if (already_forked) {

		if (logging&TYPE_FORK) {
			sprintf(log_buffer,"F 0\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		kill(forked_pid,SIGKILL);

		/* not sure if this will cause us to miss bugs */
		/* but it does make the logs more deterministic */
		if (attempt_determinism) {
			waitpid(forked_pid, &status, 0);
		}

		already_forked=0;
	}
	else {

		if (logging&TYPE_FORK) {
			sprintf(log_buffer,"F 1\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		forked_pid=fork();

		/* we're the child */
		if (forked_pid==0) {
			while(1) {
				instructions_million();
				/* we were orphaned, exit */
				/* Had problems with orphans clogging up */
				/* the system if the parent emergency */
				/* exited */
				if (getppid()==1) {
					exit(1);
				}
			}
		}

		stats.fork_attempts++;

		/* We do see failures sometimes */
		/* And when we do, if we foolishly kill process "-1" */
		/* It will kill *all* processes beloning to the user */
		/* Logging you out on all windows.                   */
		if (forked_pid==-1) {
			printf("Fork failed! %s\n",strerror(errno));
			already_forked=0;
		}
		else {
			stats.fork_successful++;
			already_forked=1;
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
		stats.errno_count[i]=0;
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
					open_random_event();
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
			case 9: if (type&TYPE_TRASH_MMAP) {
					trash_random_mmap();
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
