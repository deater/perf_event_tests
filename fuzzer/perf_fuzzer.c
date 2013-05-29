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

#include <fcntl.h>

#include "shm.h"
#include "syscall.h"
#include "../include/perf_event.h"
#include "../include/perf_helpers.h"
#include "../include/instructions_testcode.h"

int user_set_seed;
int page_size=4096;

#define DEBUG_ALL	0xffffffff
#define DEBUG_MMAP	0x01
#define DEBUG_OVERFLOW	0x02
#define DEBUG_OPEN	0x04
#define DEBUG_CLOSE	0x08
#define DEBUG_READ	0x10
#define DEBUG_WRITE	0x20
#define DEBUG_IOCTL	0x40
#define DEBUG_FORK	0x80
#define DEBUG_MMAP_SUCCESS	0x100

int debug=0;
int logging=0;

FILE *logfile;

struct shm_s *shm;

char *page_rand;


static long long total_iterations=0;
static long long overflows=0;
static long long open_attempts=0,open_successful=0;
static long long close_attempts=0,close_successful=0;
static long long mmap_attempts=0,mmap_successful=0;
static long long read_attempts=0,read_successful=0;
static long long ioctl_attempts=0,ioctl_successful=0;

#define NUM_EVENTS 1024

struct event_data_t{
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
} event_data[NUM_EVENTS];

static struct sigaction sigio;

extern struct syscall syscall_perf_event_open;

static int active_events=0;

static int find_random_active_event(void) {

	int i,x,j=0;

	if (active_events<1) return -1;

	x=rand()%active_events;

	for(i=0;i<NUM_EVENTS;i++) {
		if (event_data[i].active) {
			if (j==x) break;
			j++;
		}
	}
	return i;
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
			active_events++;
			return i;
		}
	}
	return -1;

}

#if 0
static int find_random_event(void) {

	return rand()%NUM_EVENTS;

}
#endif

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


static void our_handler(int signum, siginfo_t *info, void *uc) {

	int fd = info->si_fd;
	int i;
	int ret;
	char string[BUFSIZ];

	overflows++;

	if (debug&DEBUG_OVERFLOW) {
		sprintf(string,"OVERFLOW: fd=%d\n",fd);
		write(1,string,strlen(string));
	}

	i=lookup_event(fd);

	if (i>=0) {

	   prev_head=perf_mmap_read(event_data[i].mmap,
					event_data[i].mmap_size,
					prev_head);
        }

	/* cannot call rand() from signal handler! */
	/* we re-enter and get stuck in a futex :( */
	if (next_overflow_refresh) {
		if (debug&DEBUG_OVERFLOW) {
			sprintf(string,"OVERFLOW REFRESH: %d\n",next_refresh);
			write(1,string,strlen(string));
		}
		ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,next_refresh);
	}

	(void) ret;

}

#if 0
static void sigio_handler(int signum, siginfo_t *info, void *uc) {

	int fd = info->si_fd;
	int i;
	int ret;
	char string[BUFSIZ];


	if (debug&DEBUG_OVERFLOW) {
		sprintf(string,"SIGIO: fd=%d\n",fd);
		write(1,string,strlen(string));
	}

	i=lookup_event(fd);

	if (i>=0) {

	   prev_head=perf_mmap_read(event_data[i].mmap,
					event_data[i].mmap_size,
					prev_head);
        }

	(void) ret;

}
#endif

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


void perf_dump_attr(struct perf_event_attr *attr) {

	printf("PERF_EVENT_ATTR: ");
	printf("type=%x ",attr->type);
	printf("size=%x ",attr->size);
	printf("config=%llx ",attr->config);
	printf("sample_period=%llx ",attr->sample_period);
	printf("sample_type=%llx ",attr->sample_type);
	printf("read_format=%llx ",attr->read_format);
	printf("disabled=%d ",attr->disabled);
	printf("inherit=%d ",attr->inherit);
	printf("pinned=%d ",attr->pinned);
	printf("exclusive=%d ",attr->exclusive);
	printf("exclude_user=%d ",attr->exclude_user);
	printf("exclude_kernel=%d ",attr->exclude_kernel);
	printf("exclude_hv=%d ",attr->exclude_hv);
	printf("exclude_idle=%d ",attr->exclude_idle);
	printf("mmap=%d ",attr->mmap);
	printf("comm=%d ",attr->comm);
	printf("freq=%d ",attr->freq);
	printf("inherit_stat=%d ",attr->inherit_stat);
	printf("enable_on_exec=%d ",attr->enable_on_exec);
	printf("task=%d ",attr->task);
	printf("watermark=%d ",attr->watermark);
	printf("precise_ip=%d ",attr->precise_ip);
	printf("mmap_data=%d ",attr->mmap_data);
	printf("sample_id_all=%d ",attr->sample_id_all);
	printf("exclude_host=%d ",attr->exclude_host);
	printf("exclude_guest=%d ",attr->exclude_guest);
//	printf("exclude_callchain_kernel=%d ",attr->exclude_callchain_kernel);
//	printf("exclude_callchain_user=%d ",attr->exclude_callchain_user);
	printf("wakeup_events=%d ",attr->wakeup_events);
	printf("bp_type=%d ",attr->bp_type);
	printf("config1=%llx ",attr->config1);
	printf("config2=%llx ",attr->config2);
	printf("branch_sample_type=%lld ",attr->branch_sample_type);
//	printf("sample_regs_user=%d ",attr->sample_regs_user);
//	printf("sample_stack_user=%d ",attr->sample_stack_user);
	printf("\n");
}


void perf_log_attr(struct perf_event_attr *attr) {

	fprintf(logfile,"%x ",attr->type);
	fprintf(logfile,"%x ",attr->size);
	fprintf(logfile,"%llx ",attr->config);
	fprintf(logfile,"%llx ",attr->sample_period);
	fprintf(logfile,"%llx ",attr->sample_type);
	fprintf(logfile,"%llx ",attr->read_format);
	fprintf(logfile,"%d ",attr->disabled);
	fprintf(logfile,"%d ",attr->inherit);
	fprintf(logfile,"%d ",attr->pinned);
	fprintf(logfile,"%d ",attr->exclusive);
	fprintf(logfile,"%d ",attr->exclude_user);
	fprintf(logfile,"%d ",attr->exclude_kernel);
	fprintf(logfile,"%d ",attr->exclude_hv);
	fprintf(logfile,"%d ",attr->exclude_idle);
	fprintf(logfile,"%d ",attr->mmap);
	fprintf(logfile,"%d ",attr->comm);
	fprintf(logfile,"%d ",attr->freq);
	fprintf(logfile,"%d ",attr->inherit_stat);
	fprintf(logfile,"%d ",attr->enable_on_exec);
	fprintf(logfile,"%d ",attr->task);
	fprintf(logfile,"%d ",attr->watermark);
	fprintf(logfile,"%d ",attr->precise_ip);
	fprintf(logfile,"%d ",attr->mmap_data);
	fprintf(logfile,"%d ",attr->sample_id_all);
	fprintf(logfile,"%d ",attr->exclude_host);
	fprintf(logfile,"%d ",attr->exclude_guest);
	fprintf(logfile,"%d ",attr->wakeup_events);
	fprintf(logfile,"%d ",attr->bp_type);
	fprintf(logfile,"%llx ",attr->config1);
	fprintf(logfile,"%llx ",attr->config2);
	fprintf(logfile,"%lld ",attr->branch_sample_type);
	fprintf(logfile,"\n");
}



static void open_random_event(void) {

	int fd;

	int i,read_size=0;

	i=find_empty_event();

	/* return if no free events */
	if (i<0) return;

	while(1) {
		syscall_perf_event_open.sanitise(0);
		memcpy(&event_data[i].attr,
			(struct perf_event_attr *)shm->a1[0],
			sizeof(struct perf_event_attr));
		event_data[i].pid=shm->a2[0];
		event_data[i].cpu=shm->a3[0];
		event_data[i].group_fd=shm->a4[0];
		event_data[i].flags=shm->a5[0];


		/* Randomly make part of a group */
		if (rand()%4==2) {
			int j;
			j=find_random_active_event();
			/* is it a master? */
			if (event_data[j].group_fd==-1) {
				event_data[i].group_fd=event_data[j].fd;
			}

		}


		if (debug&DEBUG_OPEN) {
			printf("PERF_EVENT_OPEN: "
				"perf_event_open(%p,%d,%d,%d,%lx);\n",
				&event_data[i].attr,
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
			perf_dump_attr(&event_data[i].attr);
		}

		fd=perf_event_open(
			&event_data[i].attr,
			event_data[i].pid,
			event_data[i].cpu,
			event_data[i].group_fd,
			event_data[i].flags);
		open_attempts++;

		if (fd>0) break;

		if (debug&DEBUG_OPEN) {
			printf("PERF_EVENT_OPEN: FAIL %s\n",
				strerror(errno));
		}

		/* too many open files */
		if (errno==EMFILE) return;

	}
	open_successful++;
	if (debug&DEBUG_OPEN) {
		printf("PERF_EVENT_OPEN: SUCCESS fd=%d Active=%d\n",
			fd,active_events);
	}
	if (logging&DEBUG_OPEN) {
	   fprintf(logfile,"O %d %d %d %d %lx ",
				fd,
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
	   perf_log_attr(&event_data[i].attr);
	}

	event_data[i].fd=fd;
	event_data[i].active=1;

	if (event_data[i].group_fd!=-1) {
		event_data[event_data[i].group_fd].number_in_group++;
		if (debug&DEBUG_OPEN) {
			printf("ADDING %d to GROUP %d\n",fd,
				event_data[i].group_fd);
		}
	}

	/* Setup mmap buffer */

	switch(rand()%3) {
		case 0:	event_data[i].mmap_size=(rand()%64)*getpagesize();
			break;
		case 1: event_data[i].mmap_size=
				(1 + (1<<rand()%10) )*getpagesize();
			break;
		default: event_data[i].mmap_size=rand()%65535;
	}

	event_data[i].mmap=NULL;
	if (debug&DEBUG_MMAP) {
		printf("MMAP: mmap(NULL, %d, PROT_READ|PROT_WRITE, "
			"MAP_SHARED, %d, 0);\n",
			event_data[i].mmap_size,event_data[i].fd);
	}
	mmap_attempts++;
	event_data[i].mmap=mmap(NULL, event_data[i].mmap_size,
		PROT_READ|PROT_WRITE, MAP_SHARED, event_data[i].fd, 0);
	if (debug&DEBUG_MMAP) {
		printf("MMAP: RESULT %p\n",event_data[i].mmap);
	}

	if (event_data[i].mmap==MAP_FAILED) {
		event_data[i].mmap=NULL;
	}
	else {
		if (logging&DEBUG_MMAP_SUCCESS) {
 			fprintf(logfile,"M %d %d %p\n",
				event_data[i].mmap_size,event_data[i].fd,
				event_data[i].mmap);
		}

		mmap_successful++;
	}

	/* Setup overflow? */
	if (rand()%2) {

		memset(&event_data[i].sa, 0, sizeof(struct sigaction));
		event_data[i].sa.sa_sigaction = our_handler;
		event_data[i].sa.sa_flags = SA_SIGINFO;

		if (sigaction( SIGRTMIN+2, &event_data[i].sa, NULL) < 0) {
			printf("Error setting up signal handler\n");
     		}

		if (debug&DEBUG_OPEN) {
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

	/* Set nominal read size */
	if (event_data[i].attr.read_format&PERF_FORMAT_GROUP) {
		read_size=1;
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_ENABLED) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_RUNNING) read_size++;
		read_size+=(1+!!(event_data[i].attr.read_format&PERF_FORMAT_ID)) * event_data[i].number_in_group;
	}
	else {
		read_size=1; /* The value */
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_ENABLED) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_RUNNING) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_ID) read_size++;
	}
	event_data[i].read_size=read_size*sizeof(long long);
}

static void close_random_event(void) {

	int i,result;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	active_events--;

	if (debug&DEBUG_CLOSE) {
		printf("CLOSE, Active=%d, if %p munmap(%p,%x); close(%d);\n",
			active_events,
			event_data[i].mmap,
			event_data[i].mmap,event_data[i].mmap_size,
			event_data[i].fd);
	}

	if ((event_data[i].mmap)) {// && (rand()%2==1)) {
		munmap(event_data[i].mmap,event_data[i].mmap_size);
	}

	close_attempts++;
	result=close(event_data[i].fd);
	if (result==0) close_successful++;

	if (logging&DEBUG_CLOSE) {
		fprintf(logfile,"C %d\n",event_data[i].fd);
	}

	event_data[i].active=0;
}

static void ioctl_random_event(void) {

	int i,arg,arg2,result;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	switch(rand()%8) {
		case 0:
			if (debug&DEBUG_IOCTL) {
				printf("IOCTL: "
					"ioctl(%d,PERF_EVENT_IOC_ENABLE,0);\n",
				event_data[i].fd);
			}
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_ENABLE,0);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %d %d\n",
					PERF_EVENT_IOC_ENABLE,0);
			}
			break;
		case 1:
			if (debug&DEBUG_IOCTL) printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_DISABLE,0);\n",event_data[i].fd);
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_DISABLE,0);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %d %d\n",
					PERF_EVENT_IOC_DISABLE,0);
			}
			break;
		case 2: arg=rand_refresh();
			if (debug&DEBUG_IOCTL) printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_REFRESH,%d);\n",event_data[i].fd,arg);
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_REFRESH,arg);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %d %d\n",
					PERF_EVENT_IOC_REFRESH,arg);
			}
			break;
		case 3:
			if (debug&DEBUG_IOCTL) printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_RESET,0);\n",event_data[i].fd);
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_RESET,0);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %d %d\n",
					PERF_EVENT_IOC_RESET,0);
			}
			break;
		case 4: arg=rand_period();
			if (debug&DEBUG_IOCTL) printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_PERIOD,%d);\n",event_data[i].fd,arg);
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_PERIOD,arg);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %ld %d\n",
					PERF_EVENT_IOC_PERIOD,arg);
			}
			break;
		case 5: arg=event_data[find_random_active_event()].fd;
			if (debug&DEBUG_IOCTL) printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_SET_OUTPUT,%d);\n",event_data[i].fd,arg);
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %d %d\n",
					PERF_EVENT_IOC_SET_OUTPUT,arg);
			}
			break;
		case 6: arg=rand();
			if (debug&DEBUG_IOCTL) printf("IOCTL: ioctl(%d,PERF_EVENT_IOC_SET_FILTER,%d);\n",event_data[i].fd,arg);
			/* FIXME -- read filters from file */
			/* under debugfs tracing/events/ * / * /id */
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_FILTER,arg);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %ld %d\n",
					PERF_EVENT_IOC_SET_FILTER,arg);
			}
			break;
		default:
			arg=rand(); arg2=rand();
			result=ioctl(event_data[i].fd,arg,arg2);
			if (debug&DEBUG_IOCTL) printf("IOCTL: RANDOM ioctl(%d,%x,%x)\n",
				event_data[i].fd,arg,arg2);
			if ((result>=0)&&(logging&DEBUG_IOCTL)) {
				fprintf(logfile,"I %d %d\n",
					arg,arg2);
			}

			break;
	}
	if (debug&DEBUG_IOCTL) printf("IOCTL RESULT %d %s\n",result,result<0?strerror(errno):"OK");
	ioctl_attempts++;
	if (result>=0) ioctl_successful++;

}

static void prctl_random_event(void) {



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

	if (debug&DEBUG_READ) printf("READ: read(%d,%p,%d);\n",
		event_data[i].fd,&data,read_size);

	read_attempts++;
	result=read(event_data[i].fd,data,read_size);

	if (debug&DEBUG_READ) printf("READ RESULT: result %d %s\n",result,
		result<0?strerror(errno):"OK");
	if (result>0) {
	        read_successful++;
		if (logging&DEBUG_READ) {
			fprintf(logfile,"R %d %d\n",event_data[i].fd,read_size);
		}
		if (debug&DEBUG_READ) {
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

	if (debug&DEBUG_WRITE) printf("WRITE: write(%d,%p,%d);\n",
		event_data[i].fd,&data,write_size);

	result=write(event_data[i].fd,data,write_size);

	if (debug&DEBUG_WRITE) printf("WRITE RESULT: result %d %s\n",result,
		result<0?strerror(errno):"OK");


}

static void access_random_file(void) {

	/* FIXME -- access perf event files under /proc and /sys */
#if 0
	/proc/sys/kernel/perf_event_paranoid
	/proc/sys/kernel/perf_event_max_sample_rate
	/proc/sys/kernel/perf_event_mlock_kb
	/sys/bus/event_source/devices/
#endif
}

static void run_a_million_instructions(void) {

	instructions_million();

}


static int already_forked=0;
static pid_t forked_pid;

static void fork_random_event(void) {

	if (already_forked) {
		if (debug&DEBUG_FORK) printf("FORK: KILLING pid %d\n",forked_pid);
		kill(forked_pid,SIGKILL);
		already_forked=0;
	}
	else {
		if (debug&DEBUG_FORK) printf("FORKING\n");
		forked_pid=fork();

		/* we're the child */
		if (forked_pid==0) {
			while(1) instructions_million();
		}
		already_forked=1;
	}
}

#define VERSION "0.1"

static void usage(char *name,int help) {

	printf("\nPerf Fuzzer version %s\n\n",VERSION);

	if (help) {
		printf("%s [-h] [-v] [-d] [-l filename]\n\n",name);
		printf("\t-h\tdisplay help\n");
		printf("\t-v\tdisplay version\n");
		printf("\t-d\tenable debugging\n");
		printf("\t-l logfile\tlog to file\n");
		printf("\n");
	}
}

int main(int argc, char **argv) {

	int i;
	char *logfile_name=NULL;

	/* Parse command line parameters */

	if (argc>1) {

		if(argv[1][0]=='-') {
			switch(argv[1][1]) {
				case 'h':	usage(argv[0],1);
						exit(0);
						break;
				case 'v':	usage(argv[0],0);
						exit(0);
						break;
				case 'd':	debug=DEBUG_ALL;
						break;
				case 'l':	logging=DEBUG_ALL;
						if (argc>2) {
							logfile_name=strdup(argv[2]);
						}
						else {
							logfile_name=strdup("out.log");
						}
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

	} else {
		/* Use defaults */
		logging=0;
		debug=0;
	}

	if (logging) {
		logfile=fopen(logfile_name,"w");
		if (logfile==NULL) {
			fprintf(stderr,"Error opening %s\n",logfile_name);
			exit(1);
		}
	}

	/* Set up to match trinity setup, vaguely */

	shm=calloc(1,sizeof(struct shm_s));

	page_rand = memalign(page_size, page_size * 2);
	if (!page_rand) {
		exit(EXIT_FAILURE);
	}
	memset(page_rand, 0x55, page_size);
	fprintf(stderr, "page_rand @ %p\n", page_rand);

	/* Set up SIGIO handler */
	memset(&sigio, 0, sizeof(struct sigaction));
	sigio.sa_sigaction = SIG_IGN; //sigio_handler;
	sigio.sa_flags = SA_SIGINFO;

	if (sigaction( SIGIO, &sigio, NULL) < 0) {
		printf("Error setting up SIGIO signal handler\n");
     	}


	/* Initialize */
	for(i=0;i<NUM_EVENTS;i++) {
		event_data[i].active=0;
		event_data[i].fd=0;
		event_data[i].read_size=rand();
	}

	while(1) {

		switch(rand()%9) {
			case 0:	open_random_event();
				break;
			case 1: close_random_event();
				break;
			case 2: ioctl_random_event();
				break;
			case 3: prctl_random_event();
				break;
			case 4: read_random_event();
				break;
			case 5: write_random_event();
				break;
			case 6: access_random_file();
				break;
//			case 7: fork_random_event();
//				break;
			default:
				run_a_million_instructions();
				break;
		}
		next_overflow_refresh=rand()%2;
		next_refresh=rand_refresh();
		total_iterations++;
		if (total_iterations%10000==0) {
			printf("Iteration %lld\n",total_iterations);
			printf("\tOpen attempts: %lld  Successful: %lld\n",
			       open_attempts,open_successful);
			printf("\tClose attempts: %lld  Successful: %lld\n",
			       close_attempts,close_successful);
			printf("\tRead attempts: %lld  Successful: %lld\n",
			       read_attempts,read_successful);
			printf("\tIoctl attempts: %lld  Successful: %lld\n",
			       ioctl_attempts,ioctl_successful);
			printf("\tMmap attempts: %lld  Successful: %lld\n",
			       mmap_attempts,mmap_successful);
			printf("\tOverflows: %lld\n",
			       overflows);
			open_attempts=0; open_successful=0;
			close_attempts=0; close_successful=0;
			read_attempts=0; read_successful=0;
			ioctl_attempts=0; ioctl_successful=0;
			mmap_attempts=0; mmap_successful=0;
			overflows=0;
		}
		fflush(logfile);
	}

	return 0;

}
