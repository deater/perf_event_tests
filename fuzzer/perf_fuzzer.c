/* perf_fuzzer */
/* by Vince Weaver */
/* fuzzes the perf_event system call */
/* Some code shared with the trinity fuzzer */

#define VERSION "0.5"

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
#include <sys/prctl.h>
#include <time.h>
#include <sys/wait.h>

#include <poll.h>

#include <fcntl.h>

#include "shm.h"
#include "syscall.h"
#include "../include/perf_event.h"
#include "../include/perf_helpers.h"
#include "../include/instructions_testcode.h"

#include "fuzz_compat.h"

#include "perf_attr_print.h"

/* Globals from Trinity */
int page_size;
struct shm_s *shm;
char *page_rand;


#define MAX_THROTTLES		5

#define TYPE_ALL		0xffffffff
#define TYPE_MMAP		0x0001
#define TYPE_OVERFLOW		0x0002
#define TYPE_OPEN		0x0004
#define TYPE_CLOSE		0x0008
#define TYPE_READ		0x0010
#define TYPE_WRITE		0x0020
#define TYPE_IOCTL		0x0040
#define TYPE_FORK		0x0080
#define TYPE_MMAP_SUCCESS	0x0100
#define TYPE_PRCTL		0x0200
#define TYPE_POLL		0x0400
#define TYPE_MILLION		0x0800
#define TYPE_ACCESS		0x1000
#define TYPE_TRASH_MMAP	0x2000

static int debug=0;
static int logging=0;
static int stop_after=0;

static int type=TYPE_ALL;
//static int type=TYPE_OPEN|TYPE_CLOSE|TYPE_IOCTL|TYPE_OVERFLOW|TYPE_MMAP|TYPE_TRASH_MMAP;

static int log_fd;
static char log_buffer[BUFSIZ];

#define MAX_ERRNOS 1023
static int errno_count[MAX_ERRNOS];

static long long total_iterations=0;
static long long overflows=0;
static long long sigios=0;
static long long open_attempts=0,open_successful=0;
static long long close_attempts=0,close_successful=0;
static long long mmap_attempts=0,mmap_successful=0;
static long long read_attempts=0,read_successful=0;
static long long ioctl_attempts=0,ioctl_successful=0;
static long long prctl_attempts=0,prctl_successful=0;
static long long fork_attempts=0,fork_successful=0;
static long long poll_attempts=0,poll_successful=0;
static long long trash_mmap_attempts=0,trash_mmap_successful=0;

static int throttle_close_event=0;

static int already_forked=0;
static pid_t forked_pid;


#define NUM_EVENTS 1024

struct event_data_t {
	int active;
	int fd;
	struct perf_event_attr attr;
	pid_t pid;
	int cpu;
	int group_fd;
	unsigned long flags;
	int read_size;
	int number_in_group;
	struct sigaction sa;
	char *mmap;
	int mmap_size;
	int last_refresh;
	int overflows;
	int throttles;
} event_data[NUM_EVENTS];

static struct sigaction sigio;
static struct sigaction sigquit;

extern struct syscallentry syscall_perf_event_open;

static int active_events=0;

static int find_random_active_event(void) {

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

#if 0
static int find_first_active_event(void) {

	int i;

	if (active_events<1) return -1;

	for(i=0;i<NUM_EVENTS;i++) {
		if (event_data[i].active) {
			return i;
		}
	}
	return -1;
}
#endif

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

	/* here to avoid race in overflow where we munmap or close */
	/* but event still marked active */

	event_data[i].active=0;

	if (debug&TYPE_CLOSE) {
		printf("CLOSE, Active=%d, if %p munmap(%p,%x); close(%d);\n",
			active_events,
			event_data[i].mmap,
			event_data[i].mmap,event_data[i].mmap_size,
			event_data[i].fd);
	}

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

	close_attempts++;
	result=close(event_data[i].fd);
	if (result==0) close_successful++;

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


/* The perf tool uses poll() and never sets signals */
/* Thus they never have most of these problems      */
static void our_handler(int signum, siginfo_t *info, void *uc) {

	int fd = info->si_fd;
	int i;
	int ret;
	char string[BUFSIZ];

	overflows++;

	/* disable the event for the time being */
	/* we were having trouble with signal storms */
	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);
	/* Do not log, logging only make sense if */
	/* we have deterministic counts which we don't */


	if (debug&TYPE_OVERFLOW) {
		sprintf(string,"OVERFLOW: fd=%d\n",fd);
		write(1,string,strlen(string));
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
				printf("Throttle close %d!\n",i);
				if (already_forked) {
					kill(forked_pid,SIGKILL);
				}

				throttle_close_event=i;
			}
		}

		else {

			/* read the event */
			prev_head=perf_mmap_read(event_data[i].mmap,
					event_data[i].mmap_size,
					prev_head);

			/* cannot call rand() from signal handler! */
			/* we re-enter and get stuck in a futex :( */

			if (debug&TYPE_OVERFLOW) {
				sprintf(string,"OVERFLOW REFRESH: %d\n",next_refresh);
				write(1,string,strlen(string));
			}

			ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,next_refresh);
			if (ret==0) {
				event_data[i].last_refresh=next_refresh;
			}
			/* Do not log, makes no sense */
		}
	}

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

	sigios++;
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


/* Pick an intelligently random refresh value */
static int rand_refresh(void) {

	int refresh;

	switch(rand()%6) {
		case 0:	refresh=0;	break;
		case 1: refresh=1;	break;
		case 2: refresh=-1;	break;
		case 3:	refresh=rand()%100; break;
		case 4: refresh=rand();	break;
		default: refresh=1;
	}

	return refresh;

}

/* pick an intelligently random period */
static int rand_period(void) {

	int period;

	switch(rand()%6) {
		case 0:	period=0;	break;
		case 1: period=1;	break;
		case 2: period=-1;	break;
		case 3:	period=rand()%100000; break;
		case 4: period=rand();	break;
		default: period=1;
	}

	return period;
}

/* pick an intelligently random ioctl argument */
static int rand_ioctl_arg(void) {

	int value=0;

	switch(rand()%3) {
		case 0:	value=0;
			break;
		case 1: value|=PERF_IOC_FLAG_GROUP;
			break;
		case 2: value=rand();
			break;
		default:
			break;
	}

	return value;

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

	sprintf(log_buffer,"\n");
	write(log_fd,log_buffer,strlen(log_buffer));
	sync();

}

static void print_errno_name(FILE *fff, int e) {

	switch(e) {
		case EPERM:	fprintf(fff,"EPERM");
				break;
		case ENOENT:	fprintf(fff,"ENOENT");
				break;
		case E2BIG:	fprintf(fff,"E2BIG");
				break;
		case EBADF:	fprintf(fff,"EBADF");
				break;
		case EINVAL:	fprintf(fff,"EINVAL");
				break;
		case EOPNOTSUPP:	fprintf(fff,"EOPNOTSUPP");
				break;
		case ENOSPC:	fprintf(fff,"ENOSPC");
				break;
		case EMFILE:	fprintf(fff,"EMFILE");
				break;
		case EACCES:	fprintf(fff,"EACCES");
				break;
		default:	fprintf(fff,"UNKNOWN %d",e);
				break;
	}
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

	/* repeat until we create a valie event */
	while(1) {
		/* call trinity random perf_event_open() code */
		syscall_perf_event_open.sanitise(0);

		memcpy(&event_data[i].attr,
			(struct perf_event_attr *)shm->a1[0],
			sizeof(struct perf_event_attr));
		event_data[i].pid=shm->a2[0];
		event_data[i].cpu=shm->a3[0];
		event_data[i].group_fd=shm->a4[0];
		event_data[i].flags=shm->a5[0];

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

		if (debug&TYPE_OPEN) {
			printf("PERF_EVENT_OPEN: "
				"perf_event_open(%p,%d,%d,%d,%lx);\n",
				&event_data[i].attr,
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
			perf_pretty_print_attr(stdout,
						&event_data[i].attr,0);
		}

		/* Debugging code */
		/* We don't usually log failed opens as there are so many */


	        if (logging&TYPE_OPEN) {
#if 0
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
#endif
	        }

#if 0
		if (event_data[i].attr.type==PERF_TYPE_TRACEPOINT) {
			if ((event_data[i].attr.config&0xffffffff)==0x18) {
//				printf("Tracepoint 24 thwarted!\n");
				event_data[i].attr.config=0;
			}
		}
#endif

		/* Actually try to open the event */
		fd=perf_event_open(
			&event_data[i].attr,
			event_data[i].pid,
			event_data[i].cpu,
			event_data[i].group_fd,
			event_data[i].flags);
		open_attempts++;

		/* If we succede, break out of the infinite loop */
		if (fd>0) break;

		/* Otherwise, track the errors */
		if (errno<MAX_ERRNOS) errno_count[errno]++;

		if (debug&TYPE_OPEN) {
			printf("PERF_EVENT_OPEN: FAIL %s\n",
				strerror(errno));
		}

		/* no more file descriptors, so give up */
		if (errno==EMFILE) return;

	}

	/* We successfully opened an event! */

	open_successful++;
	if (debug&TYPE_OPEN) {
		printf("PERF_EVENT_OPEN: SUCCESS fd=%d Active=%d\n",
			fd,active_events);
	}

	if (logging&TYPE_OPEN) {
		sprintf(log_buffer,"O %d %d %d %d %lx ",
				fd,
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
		write(log_fd,log_buffer,strlen(log_buffer));
		perf_log_attr(&event_data[i].attr);
		fsync(log_fd);
	}

	event_data[i].fd=fd;
	event_data[i].active=1;
	active_events++;

	/* if we are member of a group, update size of group */
	/* this is needed for calcuating "proper" read size  */
	/* Although I don't think we update that properly    */
	/* Also I don't think we adjust this on close */
	if (event_data[i].group_fd!=-1) {
		int j=lookup_event(event_data[i].group_fd);

		event_data[j].number_in_group++;
		event_data[j].read_size=update_read_size(j);
		if (debug&TYPE_OPEN) {
			printf("ADDING %d to GROUP %d\n",fd,
				event_data[i].group_fd);
		}
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
		if (debug&TYPE_MMAP) {
			printf("MMAP: mmap(NULL, %d, PROT_READ|PROT_WRITE, "
				"MAP_SHARED, %d, 0);\n",
				event_data[i].mmap_size,event_data[i].fd);
		}

		mmap_attempts++;
		event_data[i].mmap=mmap(NULL, event_data[i].mmap_size,
			PROT_READ|PROT_WRITE, MAP_SHARED, event_data[i].fd, 0);
		if (debug&TYPE_MMAP) {
			printf("MMAP: RESULT %p\n",event_data[i].mmap);
		}

		if (event_data[i].mmap==MAP_FAILED) {
			event_data[i].mmap=NULL;
		}
		else {
			if (logging&TYPE_MMAP_SUCCESS) {
 				sprintf(log_buffer,"M %d %d %p\n",
					event_data[i].mmap_size,event_data[i].fd,
					event_data[i].mmap);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			mmap_successful++;
		}
	}

	/* Setup overflow 50% of the time */
	if ((type&TYPE_OVERFLOW) && (rand()%2)) {

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

		if (debug&TYPE_OPEN) {
			printf("SIGNAL: sigaction(SIGRTMIN+2,%p,NULL); "
			"fcntl(%d,F_SETFL,O_RDWR|O_NONBLOCK|O_ASYNC); "
			"fcntl(%d,F_SETSIG, SIGRTMIN+2);"
			"fcntl(%d,G_SETOWN, %d);\n",
			&event_data[i].sa,
			event_data[i].fd,
			event_data[i].fd,
			event_data[i].fd,getpid());
		}
		fcntl(event_data[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		fcntl(event_data[i].fd, F_SETSIG, SIGRTMIN+2);
		fcntl(event_data[i].fd, F_SETOWN,getpid());

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

		memset(event_data[i].mmap,value,getpagesize());

		/* can't write high pages? */
		//event_data[i].mmap_size);

		if (logging&TYPE_TRASH_MMAP) {
			sprintf(log_buffer,"Q %d %d %d\n",
				value,
				getpagesize(),
				event_data[i].fd);
			write(log_fd,log_buffer,strlen(log_buffer));
		}
	}

	trash_mmap_attempts++;
	trash_mmap_successful++;

}

static void ioctl_random_event(void) {

	int i,arg,arg2,result;
	long long id;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	switch(rand()%9) {
		case 0:
			arg=rand_ioctl_arg();
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: "
					"ioctl(%d,PERF_EVENT_IOC_ENABLE,%d);\n",
					event_data[i].fd,arg);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_ENABLE,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_ENABLE,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 1:
			arg=rand_ioctl_arg();
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_DISABLE,%d);\n",
					event_data[i].fd,arg);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_DISABLE,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_DISABLE,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 2:
			arg=rand_refresh();
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_REFRESH,%d);\n",
				event_data[i].fd,arg);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_REFRESH,arg);
			if (result>0) {
				event_data[i].last_refresh=arg;
			}
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_REFRESH,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 3:
			arg=rand_ioctl_arg();
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_RESET,%d);\n",
				event_data[i].fd,arg);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_RESET,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_RESET,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 4: arg=rand_period();
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_PERIOD,%d);\n",
					event_data[i].fd,arg);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_PERIOD,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %d\n",
					event_data[i].fd,(long)PERF_EVENT_IOC_PERIOD,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 5: arg=event_data[find_random_active_event()].fd;
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_SET_OUTPUT,%d);\n",
					event_data[i].fd,arg);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 6: arg=rand();
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_SET_FILTER,%d);\n",
					event_data[i].fd,arg);
			}
			/* FIXME -- read filters from file */
			/* under debugfs tracing/events/ * / * /id */
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_FILTER,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %d\n",
					event_data[i].fd,(long)PERF_EVENT_IOC_SET_FILTER,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 7: arg=rand();
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_ID,%p);\n",
					event_data[i].fd,&id);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_ID,&id);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %lld\n",
					event_data[i].fd,(long)PERF_EVENT_IOC_ID,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		default:
			arg=rand(); arg2=rand();
			result=ioctl(event_data[i].fd,arg,arg2);
			if (debug&TYPE_IOCTL) {
				printf("IOCTL: RANDOM ioctl(%d,%x,%x)\n",
					event_data[i].fd,arg,arg2);
			}
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,arg,arg2);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;
	}
	if (debug&TYPE_IOCTL) printf("IOCTL RESULT %d %s\n",result,result<0?strerror(errno):"OK");
	ioctl_attempts++;
	if (result>=0) ioctl_successful++;

}

static void prctl_random_event(void) {

	int ret;

	prctl_attempts++;

	if (rand()%2) {
		ret=prctl(PR_TASK_PERF_EVENTS_ENABLE);
		if (debug&TYPE_PRCTL) {
			printf("PRCTL: PR_TASK_PERF_EVENTS_ENABLE\n");
		}
		if ((ret==0)&&(logging&TYPE_PRCTL)) {
			sprintf(log_buffer,"P 1\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}
	}
	else {
		ret=prctl(PR_TASK_PERF_EVENTS_DISABLE);
		if (debug&TYPE_PRCTL) {
			printf("PRCTL: PR_TASK_PERF_EVENTS_DISABLE\n");
		}
		if ((ret==0)&&(logging&TYPE_PRCTL)) {
			sprintf(log_buffer,"P 0\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

	}
	if (ret==0) prctl_successful++;
}

#define MAX_READ_SIZE 65536

static long long data[MAX_READ_SIZE];


static void read_random_event(void) {

	int i,result,read_size;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	/* Exit if event has fd of 0, not want to read stdin */
	if (event_data[i].fd==0) return;

	switch (rand()%4) {
		case 0:	read_size=event_data[i].read_size;
			break;
		case 1: read_size=(rand()%8)*sizeof(long long);
			break;
		case 2: read_size=(rand()%MAX_READ_SIZE);
			break;
		default: read_size=(rand()%MAX_READ_SIZE)*sizeof(long long);
	}

	if (debug&TYPE_READ) printf("READ: read(%d,%p,%d);\n",
		event_data[i].fd,&data,read_size);

	read_attempts++;
	result=read(event_data[i].fd,data,read_size);

	if (debug&TYPE_READ) printf("READ RESULT: result %d %s\n",result,
		result<0?strerror(errno):"OK");
	if (result>0) {
	        read_successful++;
		if (logging&TYPE_READ) {
			sprintf(log_buffer,"R %d %d\n",event_data[i].fd,read_size);
			write(log_fd,log_buffer,strlen(log_buffer));
		}
		if (debug&TYPE_READ) {
			printf("READ VALUES: ");
			for(i=0;i<result/8;i++) printf("%lld ",data[i]);
			printf("\n");
		}
	}

}

static void write_random_event(void) {

	int i,result,write_size;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	/* Exit if event has fd of 0, not want to read stdin */
	if (event_data[i].fd==0) return;

	switch (rand()%4) {
		case 0:	write_size=event_data[i].read_size;
			break;
		case 1: write_size=(rand()%8)*sizeof(long long);
			break;
		case 2: write_size=(rand()%MAX_READ_SIZE);
			break;
		default: write_size=(rand()%MAX_READ_SIZE)*sizeof(long long);
	}

	if (debug&TYPE_WRITE) printf("WRITE: write(%d,%p,%d);\n",
		event_data[i].fd,&data,write_size);

	result=write(event_data[i].fd,data,write_size);

	if (debug&TYPE_WRITE) printf("WRITE RESULT: result %d %s\n",result,
		result<0?strerror(errno):"OK");


}

static void poll_random_event(void) {

#define MAX_POLL_FDS 128

	int i,result,num_fds;

	struct pollfd pollfds[MAX_POLL_FDS];
	int timeout;
	char log_buffer[BUFSIZ];

	num_fds=rand()%MAX_POLL_FDS;

	for(i=0;i<num_fds;i++) {
		pollfds[i].fd=event_data[find_random_active_event()].fd;
		pollfds[i].events=POLLIN;
	}

	/* Want short timeout (ms) */
	timeout=rand()%10;


	if (debug&TYPE_POLL) {
		printf("POLL: poll(");
		for(i=0;i<num_fds;i++) {
			printf("%d,%d, ",pollfds[i].fd,pollfds[i].events);
		}
		printf("%d, %d);\n",num_fds,timeout);
	}

	poll_attempts++;
	result=poll(pollfds,num_fds,timeout);

	if (debug&TYPE_POLL) printf("POLL RESULT: result %d %s\n",result,
		result<0?strerror(errno):"OK");
	if (result>0) {
	        poll_successful++;
		if (logging&TYPE_POLL) {
			sprintf(log_buffer,"p %d ",num_fds);
			write(log_fd,log_buffer,strlen(log_buffer));
			for(i=0;i<num_fds;i++) {
				sprintf(log_buffer,"%d %x ",pollfds[i].fd,
							pollfds[i].events);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			sprintf(log_buffer,"%d\n",timeout);
			write(log_fd,log_buffer,strlen(log_buffer));
		}
	}

}


static void access_random_file(void) {

	int which;
	FILE *fff;
	char buffer[2048];

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

	which=rand()%4;

	if (rand()%2) {
		/* read */
		fff=fopen(filenames[which],"r");
		if (fff!=NULL) {
			fread(buffer,sizeof(char),2048,fff);
			fclose(fff);
		}
	}
	else {
		/* write */
		fff=fopen(filenames[which],"w");
		if (fff!=NULL) {
			fprintf(fff,"%lld\n",
				((unsigned long long)rand()<<32)|rand());
			fclose(fff);
		}
	}


	/****************************************/
	/* TODO -- try some files under 	*/
	/*	/sys/bus/event_source/devices/	*/
	/****************************************/

}

static void run_a_million_instructions(void) {

	instructions_million();

}



static void fork_random_event(void) {

	int status;

	if (already_forked) {
		if (debug&TYPE_FORK) {
			printf("FORK: KILLING pid %d\n",forked_pid);
		}
		if (logging&TYPE_FORK) {
			sprintf(log_buffer,"F 0\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		kill(forked_pid,SIGKILL);

		/* not sure if this will cause us to miss bugs */
		/* but it does make the logs more deterministic */
		waitpid(forked_pid, &status, 0);

		already_forked=0;
	}
	else {
		if (debug&TYPE_FORK) {
			printf("FORKING\n");
		}
		if (logging&TYPE_FORK) {
			sprintf(log_buffer,"F 1\n");
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		forked_pid=fork();

		/* we're the child */
		if (forked_pid==0) {
			while(1) instructions_million();
		}

		fork_attempts++;

		/* We do see failures sometimes */
		/* And when we do, if we foolishly kill process "-1" */
		/* It will kill *all* processes beloning to the user */
		/* Logging you out on all windows.                   */
		if (forked_pid==-1) {
			printf("Fork failed! %s\n",strerror(errno));
			already_forked=0;
		}
		else {
			fork_successful++;
			already_forked=1;
		}
	}
}


static void dump_summary(FILE *fff) {

	int i;

	fprintf(fff,"Iteration %lld\n",total_iterations);
	fprintf(fff,"\tOpen attempts: %lld  Successful: %lld\n",
	       open_attempts,open_successful);
	for(i=0;i<MAX_ERRNOS;i++) {
		if (errno_count[i]!=0) {
			fprintf(fff,"\t\t");
			print_errno_name(fff,i);
			fprintf(fff," : %d\n",errno_count[i]);
		}
	}

	fprintf(fff,"\tClose attempts: %lld  Successful: %lld\n",
	       close_attempts,close_successful);
	fprintf(fff,"\tRead attempts: %lld  Successful: %lld\n",
	       read_attempts,read_successful);
	fprintf(fff,"\tIoctl attempts: %lld  Successful: %lld\n",
	       ioctl_attempts,ioctl_successful);
	fprintf(fff,"\tMmap attempts: %lld  Successful: %lld\n",
	       mmap_attempts,mmap_successful);
	fprintf(fff,"\tPrctl attempts: %lld  Successful: %lld\n",
	       prctl_attempts,prctl_successful);
	fprintf(fff,"\tFork attempts: %lld  Successful: %lld\n",
	       fork_attempts,fork_successful);
	fprintf(fff,"\tPoll attempts: %lld  Successful: %lld\n",
	       poll_attempts,poll_successful);
	fprintf(fff,"\tTrash mmap attempts: %lld  Successful: %lld\n",
		trash_mmap_attempts,trash_mmap_successful);
	fprintf(fff,"\tOverflows: %lld\n", overflows);
	fprintf(fff,"\tSIGIOs due to RT signal queue full: %lld\n",sigios);

	/* Reset counts back to zero */
	open_attempts=0; open_successful=0;
	close_attempts=0; close_successful=0;
	read_attempts=0; read_successful=0;
	ioctl_attempts=0; ioctl_successful=0;
	mmap_attempts=0; mmap_successful=0;
	prctl_attempts=0; prctl_successful=0;
	fork_attempts=0; fork_successful=0;
	poll_attempts=0; poll_successful=0;
	trash_mmap_attempts=0; trash_mmap_successful=0;
	overflows=0;
	sigios=0;
	for(i=0;i<MAX_ERRNOS;i++) {
		errno_count[i]=0;
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

static void usage(char *name,int help) {

	printf("\nPerf Fuzzer version %s\n\n",VERSION);

	if (help) {
		printf("%s [-h] [-v] [-d] [-f] [-l filename] [-s num] [-r num] [-t OCIRMUPFpm]\n\n",name);
		printf("\t-h\tdisplay help\n");
		printf("\t-v\tdisplay version\n");
		printf("\t-d\tenable debugging\n");
		printf("\t-l logfile\tlog to file (- for stdout)\n");
		printf("\t-f only log fork syscalls; do not run\n");
		printf("\t-r num\tseed random number generator with num\n");
		printf("\t-s num\tstop after num system calls\n");
		printf("\t-t OCIRMUPFpm type of calls to execute (default is all)\n");
		printf("\n");
	}
}

int main(int argc, char **argv) {

	int i;
	char *logfile_name=NULL;
	unsigned int seed=0;
	int sample_rate;
	FILE *fff;

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
				/* debug */
				case 'd':	debug=TYPE_ALL;
						i++;
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
				case 't':	break;
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
		debug=0;
	}

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
		}
	}

	/* Poor Seeding */
	/* should read /dev/urandom instead */
	if (!seed) {
		seed=time(NULL);
	}
	srand(seed);
	printf("Seeding random number generator with %d\n",seed);


	/* Clear errnos count */
	for(i=0;i<MAX_ERRNOS;i++) {
		errno_count[i]=0;
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
	printf("Kernel max sample rate currently: %d/s\n",sample_rate);
	if (logging) {
		sprintf(log_buffer,"r %d\n",sample_rate);
		write(log_fd,log_buffer,strlen(log_buffer));
	}

	/* Set up to match trinity setup, vaguely */
	page_size=getpagesize();

	syscall_perf_event_open.init();

	shm=calloc(1,sizeof(struct shm_s));

	page_rand = memalign(page_size, page_size * 2);
	if (!page_rand) {
		exit(EXIT_FAILURE);
	}
	memset(page_rand, 0x55, page_size);



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

		if (throttle_close_event) {
			printf("Closing stuck event %d\n",
				throttle_close_event);
			close_event(throttle_close_event,1);
			throttle_close_event=0;
		}

		next_overflow_refresh=rand()%2;
		next_refresh=rand_refresh();
		total_iterations++;

		if ((stop_after) && (total_iterations>=stop_after)) {
			dump_summary(stderr);
			return 0;
		}

		/* Print status update every 10000 iterations      */
		/* Don't print if logging to stdout as it clutters */
		/* up the trace file.				   */
		if (total_iterations%10000==0) {
			if (log_fd!=1) {
				dump_summary(stderr);
			}
		}
//		fsync(log_fd);
	}

	return 0;

}
