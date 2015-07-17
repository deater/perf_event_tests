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
#include <poll.h>

#include <fcntl.h>

#include <shm.h>
#include <syscall.h>
#include "perf_event.h"
#include "perf_helpers.h"

#include "fuzz_compat.h"

#include "perf_attr_print.h"

static unsigned long long line_num=0;

static int original_pid=-1;

#define MAX_EVENTS 8192

struct event_info_t {
	int opened;
	int enabled;
	int mmap;
	int signal_handler;
        pid_t pid;
	int cpu;
	int group_fd;
	unsigned long flags;
	struct perf_event_attr attr;
} event_info[MAX_EVENTS];

struct mmap_info_t {
	int valid;
	int size;
	unsigned long addr;
} mmap_info[MAX_EVENTS];


static void mmap_event(char *line) {

	int fd,size;

	sscanf(line,"%*c %d %d",&size,&fd);

	if (event_info[fd].opened==0) {
		fprintf(stderr,"Line %lld Skipping mmap as fd %d not valid\n",
			line_num,fd);
		return;
	}

	event_info[fd].mmap=1;
	mmap_info[fd].size=size;
	mmap_info[fd].valid=1;

}

static void trash_mmap_event(char *line) {

}


static void munmap_event(char *line) {

	int fd,size;

	sscanf(line,"%*c %d %d",&fd,&size);

	if (mmap_info[fd].valid==0) {
		fprintf(stderr,"Line: %lld Skipping munmap as fd %d is invalid\n",
			line_num,fd);
		return;
	}

	mmap_info[fd].valid=0;
	mmap_info[fd].size=0;

}


static void open_event(char *line) {


	struct perf_event_attr pe;
	int orig_fd;
	pid_t pid;
	int cpu,group_fd;
	long int flags;

	/* I hate bitfields */
	int disabled,inherit,pinned,exclusive;
	int exclude_user,exclude_kernel,exclude_hv,exclude_idle;
	int mmap,comm,freq,inherit_stat;
	int enable_on_exec,task,watermark,precise_ip;
	int mmap_data,sample_id_all,exclude_host,exclude_guest;
	int exclude_callchain_user,exclude_callchain_kernel;
	int mmap2;


	sscanf(line,
		"%*c %d %d %d %d %lx "
		"%x %x "
		"%llx %llx %llx %llx "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d "
		"%llx %llx %lld "
		"%d %d %lld %d %d",
		&orig_fd,&pid,&cpu,&group_fd,&flags,
		&pe.type,&pe.size,
		&pe.config,&pe.sample_period,&pe.sample_type,&pe.read_format,
		&disabled,&inherit,&pinned,&exclusive,
		&exclude_user,&exclude_kernel,&exclude_hv,&exclude_idle,
		&mmap,&comm,&freq,&inherit_stat,
		&enable_on_exec,&task,&watermark,&precise_ip,
		&mmap_data,&sample_id_all,&exclude_host,&exclude_guest,
		&pe.wakeup_events,&pe.bp_type,
		&pe.config1,&pe.config2,&pe.branch_sample_type,
		&exclude_callchain_kernel,&exclude_callchain_user,
		&pe.sample_regs_user,&pe.sample_stack_user,&mmap2);

	if ((orig_fd<0) || (orig_fd>=MAX_EVENTS)) {
		fprintf(stderr,"Error! fd %d out of range\n",orig_fd);
		return;
	}

	event_info[orig_fd].pid=pid;
	event_info[orig_fd].cpu=cpu;
	event_info[orig_fd].group_fd=group_fd;
	event_info[orig_fd].flags=flags;
	event_info[orig_fd].attr.type=pe.type;
	event_info[orig_fd].attr.size=pe.size;
	event_info[orig_fd].attr.config=pe.config;
	event_info[orig_fd].attr.sample_period=pe.sample_period;
	event_info[orig_fd].attr.sample_type=pe.sample_type;
	event_info[orig_fd].attr.read_format=pe.read_format;
	event_info[orig_fd].attr.wakeup_events=pe.wakeup_events;
	event_info[orig_fd].attr.bp_type=pe.bp_type;
	event_info[orig_fd].attr.config1=pe.config1;
	event_info[orig_fd].attr.config2=pe.config2;
	event_info[orig_fd].attr.branch_sample_type=pe.branch_sample_type;
	event_info[orig_fd].attr.sample_regs_user=pe.sample_regs_user;
	event_info[orig_fd].attr.sample_stack_user=pe.sample_stack_user;

	event_info[orig_fd].attr.disabled=disabled;
	event_info[orig_fd].attr.inherit=inherit;
	event_info[orig_fd].attr.pinned=pinned;
	event_info[orig_fd].attr.exclusive=exclusive;
	event_info[orig_fd].attr.exclude_user=exclude_user;
	event_info[orig_fd].attr.exclude_kernel=exclude_kernel;
	event_info[orig_fd].attr.exclude_hv=exclude_hv;
	event_info[orig_fd].attr.exclude_idle=exclude_idle;
	event_info[orig_fd].attr.mmap=mmap;
	event_info[orig_fd].attr.comm=comm;
	event_info[orig_fd].attr.freq=freq;
	event_info[orig_fd].attr.inherit_stat=inherit_stat;
	event_info[orig_fd].attr.enable_on_exec=enable_on_exec;
	event_info[orig_fd].attr.task=task;
	event_info[orig_fd].attr.watermark=watermark;
	event_info[orig_fd].attr.precise_ip=precise_ip;
	event_info[orig_fd].attr.mmap_data=mmap_data;
	event_info[orig_fd].attr.sample_id_all=sample_id_all;
	event_info[orig_fd].attr.exclude_host=exclude_host;
	event_info[orig_fd].attr.exclude_guest=exclude_guest;
	event_info[orig_fd].attr.exclude_callchain_user=exclude_callchain_user;
	event_info[orig_fd].attr.exclude_callchain_kernel=exclude_callchain_kernel;
	event_info[orig_fd].attr.mmap2=mmap2;

	event_info[orig_fd].opened=1;
	event_info[orig_fd].enabled=enable_on_exec;
	event_info[orig_fd].mmap=0;
	event_info[orig_fd].signal_handler=0;

}

static void close_event(char *line) {

	int fd;

	sscanf(line,"%*c %d",&fd);

	if (event_info[fd].opened==0) {
		fprintf(stderr,"Line %lld Skipping close as fd %d not valid\n",
			line_num,fd);
		return;
	}

	event_info[fd].opened=0;
	event_info[fd].enabled=0;
	event_info[fd].mmap=0;
	event_info[fd].signal_handler=0;
}


static void ioctl_event(char *line) {

	int fd,arg,arg2;

	sscanf(line,"%*c %d %d %d",&fd,&arg,&arg2);

	errno=0;

	switch(arg) {
		case PERF_EVENT_IOC_ENABLE:
			event_info[fd].enabled=1;
			break;
		case PERF_EVENT_IOC_DISABLE:
			event_info[fd].enabled=0;
			break;
		default:
			break;
	}


}



static void read_event(char *line) {

}

static void prctl_event(char *line) {

        int enable=0,i;

        sscanf(line,"%*c %d",&enable);

        if (enable) {
		for(i=0;i<MAX_EVENTS;i++) {
			if (event_info[i].opened) event_info[i].enabled=1;
		}
        }
        else {
		for(i=0;i<MAX_EVENTS;i++) {
			if (event_info[i].opened) event_info[i].enabled=0;
		}
        }
}


static void poll_event(char *line) {

}

static void fork_event(char *line) {

}


static void setup_overflow(char *line) {

	int overflow_fd;

	sscanf(line,"%*c %d",&overflow_fd);

	event_info[overflow_fd].signal_handler=1;

}


#define REPLAY_OPEN		0x001
#define REPLAY_CLOSE		0x002
#define REPLAY_IOCTL		0x004
#define REPLAY_READ		0x008
#define REPLAY_MMAP		0x010
#define REPLAY_MUNMAP		0x020
#define REPLAY_PRCTL		0x040
#define REPLAY_FORK		0x080
#define REPLAY_POLL		0x100
#define REPLAY_SEED		0x200
#define REPLAY_OVERFLOW		0x400
#define REPLAY_TRASH_MMAP	0x800
#define REPLAY_ALL		0xfff


void print_usage(char *exec_name) {

	fprintf(stderr,"\nUsage: %s [-h] [-s lines] [-r OCIRMUPFp] filename\n\n",
		exec_name);
}

int main(int argc, char **argv) {

	FILE *logfile;
	char *logfile_name=NULL;
	char line[BUFSIZ];
	char *result;
	long long total_syscalls=0,replay_syscalls=0;
	long long skip_lines=0;

	int i;

	int replay_which=REPLAY_ALL;

	if (argc<2) {
		print_usage(argv[0]);
		exit(1);
	}

	i=1;
	while(1) {
		if (i>=argc) break;

		if (argv[i][0]=='-') {
			switch(argv[i][1]) {
			case 'h':	print_usage(argv[0]);
					exit(1);
					break;
			default:	fprintf(stderr,"Unknown option -%c\n",argv[i][1]);
					exit(1);
					break;
			}
		}
		else {
			logfile_name=strdup(argv[i]);
			i++;
		}
	}

	if (logfile_name==NULL) {
		fprintf(stderr,"Must specify logfile name\n");
		exit(1);
	}

	logfile=fopen(logfile_name,"r");
	if (logfile==NULL) {
		fprintf(stderr,"Error opening %s\n",logfile_name);
		exit(1);
	}

	/* Init structs */
	for(i=0;i<MAX_EVENTS;i++) {
		event_info[i].opened=0;
		event_info[i].enabled=0;
		mmap_info[i].valid=0;
	}

	/* Main loop */

	while(1) {
		result=fgets(line,BUFSIZ,logfile);
		if (result==NULL) break;

		line_num++;

		if (line_num<skip_lines) continue;

		switch(line[0]) {
			case 'A':
				break;
			case 'C':
				if (replay_which & REPLAY_CLOSE) {
					close_event(line);
					replay_syscalls++;
				}
				break;
			case 'F':
				if (replay_which & REPLAY_FORK) {
					fork_event(line);
					replay_syscalls++;
				}
				break;
			case 'G':
				sscanf(line,"%*c %d",&original_pid);
				printf("Original pid was %d\n",original_pid);
				break;
			case 'I':
				if (replay_which & REPLAY_IOCTL) {
					ioctl_event(line);
					replay_syscalls++;
				}
				break;
			case 'M':
				if (replay_which & REPLAY_MMAP) {
					mmap_event(line);
					replay_syscalls++;
				}
				break;
			case 'O':
				if (replay_which & REPLAY_OPEN) {
					open_event(line);
					replay_syscalls++;
				}
				break;
			case 'o':
				if (replay_which & REPLAY_OVERFLOW) {
					setup_overflow(line);
					replay_syscalls++;
				}
				break;
			case 'P':
				if (replay_which & REPLAY_PRCTL) {
					prctl_event(line);
					replay_syscalls++;
				}
				break;
			case 'p':
				if (replay_which & REPLAY_POLL) {
					poll_event(line);
					replay_syscalls++;
				}
				break;
			case 'Q':
				if (replay_which & REPLAY_TRASH_MMAP) {
					trash_mmap_event(line);
					replay_syscalls++;
				}
				break;
			case 'q':
				fprintf(stderr,"Quitting early\n");
				exit(1);
			case 'R':
				if (replay_which & REPLAY_READ) {
					read_event(line);
					replay_syscalls++;
				}
				break;
			case 'S':
				if (replay_which & REPLAY_SEED) {
					/* don't need to do anything */
					/* as we don't use rand      */
				}
				break;
			case 'U':
				if (replay_which & REPLAY_MUNMAP) {
					munmap_event(line);
					replay_syscalls++;
				}
				break;
			default:
				fprintf(stderr,"Line %lld Unknown log type \'%c\'\n",
					line_num,line[0]);
				break;
		}
		//if (error) break;
		total_syscalls++;
		if (total_syscalls%1000==0) {
			printf("%lld\n",total_syscalls);
		}
	}
	printf("VMW: %s",line);

	printf("ACTIVE EVENT REPORT\n");
	printf("~~~~~~~~~~~~~~~~~~~\n");
	printf("\tReplayed %lld of %lld syscalls\n",
		replay_syscalls,total_syscalls);

	int total_active=0,total_opened=0;

	for(i=0;i<MAX_EVENTS;i++) {
		if (event_info[i].opened) total_opened++;
		if (event_info[i].enabled) total_active++;
	}
	printf("\t%d events open, %d events active\n",
		total_opened,total_active);

	printf("ENABLED EVENTS\n\n");
	for(i=0;i<MAX_EVENTS;i++) {
		if (event_info[i].enabled) {
			perf_pretty_print_event(stdout,
				i,original_pid,
				&event_info[i].attr,
				event_info[i].pid,
				event_info[i].cpu,
				event_info[i].group_fd,
				event_info[i].flags);
			printf("\n\n");
		}
	}

	printf("SHORT ENABLED EVENT SUMMARY\n\n");
	for(i=0;i<MAX_EVENTS;i++) {
		if (event_info[i].enabled) {
			perf_pretty_print_event_short(stdout,
				i,original_pid,
				&event_info[i].attr,
				event_info[i].pid,
				event_info[i].cpu,
				event_info[i].group_fd,
				event_info[i].flags);
		}
	}


	printf("SHORT OPENED EVENT SUMMARY\n\n");
	for(i=0;i<MAX_EVENTS;i++) {
		if (event_info[i].opened) {
			perf_pretty_print_event_short(stdout,
				i,original_pid,
				&event_info[i].attr,
				event_info[i].pid,
				event_info[i].cpu,
				event_info[i].group_fd,
				event_info[i].flags);
		}
	}
	return 0;

}
