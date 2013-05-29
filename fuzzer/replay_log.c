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

static int error=0;

int page_size=4096;

int debug=0;

char *page_rand;

#if 0
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


}

#endif

static void mmap_event(char *line) {

#if 0
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
#endif


}

#define FD_REMAP_SIZE 2048

int fd_remap[FD_REMAP_SIZE];

static void open_event(char *line) {


	struct perf_event_attr pe;
	int fd,orig_fd,remapped_group_fd;
	pid_t pid;
	int cpu,group_fd;
	long int flags;

	/* I hate bitfields */
	int disabled,inherit,pinned,exclusive;
	int exclude_user,exclude_kernel,exclude_hv,exclude_idle;
	int mmap,comm,freq,inherit_stat;
	int enable_on_exec,task,watermark,precise_ip;
	int mmap_data,sample_id_all,exclude_host,exclude_guest;

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
		"%llx %llx %lld ",
		&orig_fd,&pid,&cpu,&group_fd,&flags,
		&pe.type,&pe.size,
		&pe.config,&pe.sample_period,&pe.sample_type,&pe.read_format,
		&disabled,&inherit,&pinned,&exclusive,
		&exclude_user,&exclude_kernel,&exclude_hv,&exclude_idle,
		&mmap,&comm,&freq,&inherit_stat,
		&enable_on_exec,&task,&watermark,&precise_ip,
		&mmap_data,&sample_id_all,&exclude_host,&exclude_guest,
		&pe.wakeup_events,&pe.bp_type,
		&pe.config1,&pe.config2,&pe.branch_sample_type);

	/* re-populate bitfields */
	/* can't sscanf into them */
	pe.disabled=disabled;
	pe.inherit=inherit;
	pe.pinned=pinned;
	pe.exclusive=exclusive;
	pe.exclude_user=exclude_user;
	pe.exclude_kernel=exclude_kernel;
	pe.exclude_hv=exclude_hv;
	pe.exclude_idle=exclude_idle;
	pe.mmap=mmap;
	pe.comm=comm;
	pe.freq=freq;
	pe.inherit_stat=inherit_stat;
	pe.enable_on_exec=enable_on_exec;
	pe.task=task;
	pe.watermark=watermark;
	pe.precise_ip=precise_ip;
	pe.mmap_data=mmap_data;
	pe.sample_id_all=sample_id_all;
	pe.exclude_host=exclude_host;
	pe.exclude_guest=exclude_guest;


	if (group_fd==-1) {
		remapped_group_fd=-1;
	} else {
		remapped_group_fd=fd_remap[group_fd];
	}

	fd=perf_event_open(&pe,pid,cpu,remapped_group_fd,flags);
	if (fd<0) {
		fprintf(stderr,"Error opening %s",line);
		error=1;
		return;
	}

	if (fd>FD_REMAP_SIZE) {
		fprintf(stderr,"fd out of range\n");
		error=1;
		return;
	}
	fd_remap[orig_fd]=fd;



}

static void close_event(char *line) {

#if 0
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
#endif
}


static void ioctl_event(char *line) {

#if 0
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
#endif
}


#define MAX_READ_SIZE 65536

static long long data[MAX_READ_SIZE];


static void read_event(char *line) {
#if 0
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
#endif
}


int main(int argc, char **argv) {

	FILE *logfile;
	char line[BUFSIZ];
	char *result;
	long long total_syscalls=0;

	if (argc<2) {
		fprintf(stderr,"\nUsage: %s logfile <replay_type>\n\n",
			argv[0]);
		exit(1);
	}

	logfile=fopen(argv[1],"r");
	if (logfile==NULL) {
		fprintf(stderr,"Error opening %s\n",argv[1]);
		exit(1);
	}

	while(1) {
		result=fgets(line,BUFSIZ,logfile);
		if (result==NULL) break;

		switch(line[0]) {
			case 'O':
				open_event(line);
				break;
			case 'C':
				close_event(line);
				break;
			case 'I':
				ioctl_event(line);
				break;
			case 'R':
				read_event(line);
				break;
			case 'M':
				mmap_event(line);
				break;
			default:
				fprintf(stderr,"Unknown log type \'%c\'\n",
					line[0]);
				break;
		}
		if (error) break;
		total_syscalls++;
	}

	printf("Replayed %lld syscalls\n",total_syscalls);

	return 0;

}
