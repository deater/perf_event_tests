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

#include "shm.h"
#include "syscall.h"
#include "../include/perf_event.h"
#include "../include/perf_helpers.h"

#include "fuzz_compat.h"

#include "perf_attr_print.h"

static int error=0;
static unsigned long long line_num=0;

#define FD_REMAP_SIZE 2048

static int fd_remap[FD_REMAP_SIZE];
static char *mmap_remap[FD_REMAP_SIZE];

static void mmap_event(char *line) {

	int fd,size;
	char *data;

	sscanf(line,"%*c %d %d",&size,&fd);

	if (fd_remap[fd]==-1) {
		fprintf(stderr,"Line %lld Skipping mmap as fd %d not valid\n",
			line_num,fd);
		return;
	}

	data=mmap(NULL, size,
		PROT_READ|PROT_WRITE, MAP_SHARED,
		fd_remap[fd], 0);

	if (data==MAP_FAILED) {
		fprintf(stderr,"Line %lld: Error with mmap of size %d of %d/%d\n",
			line_num,size,fd,fd_remap[fd]);
		error=1;
	}
	mmap_remap[fd_remap[fd]]=data;

//		fcntl(event_data[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
//		fcntl(event_data[i].fd, F_SETSIG, SIGRTMIN+2);
//		fcntl(event_data[i].fd, F_SETOWN,getpid());

}


static void munmap_event(char *line) {

	int fd,size,result;

	sscanf(line,"%*c %d %d",&fd,&size);

	if (fd_remap[fd]==-1) {
		fprintf(stderr,"Line: %lld Skipping munmap as fd %d is invalid\n",
			line_num,fd);
		return;
	}

	if (mmap_remap[fd_remap[fd]]==MAP_FAILED) {
		fprintf(stderr,"Line: %lld Skipping munmap as data is invalid\n",
			line_num);
		return;

	}

	result=munmap(mmap_remap[fd_remap[fd]], size);
	if (result<0) {
		fprintf(stderr,"Line: %lld Error with munmap of %p size %d of %d/%d\n",
			line_num,mmap_remap[fd_remap[fd]],size,fd,fd_remap[fd]);
		error=1;
		return;
	}

}


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

	errno=0;

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
		fprintf(stderr,"Line %lld Error opening %s : %s\n",
			line_num,line,strerror(errno));
                perf_pretty_print_event(stderr,orig_fd,
                                &pe, pid, cpu,
                                remapped_group_fd,flags);
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

	int fd;
	int result;

	sscanf(line,"%*c %d",&fd);

	if (fd_remap[fd]==-1) {
		fprintf(stderr,"Line %lld Skipping close as fd %d not valid\n",
			line_num,fd);
		return;
	}

	result=close(fd_remap[fd]);
	if (result<0) {
		fprintf(stderr,"Line %lld Error closing %d/%d\n",
			line_num,fd,fd_remap[fd]);

	}
	fd_remap[fd]=-1;

}


static void ioctl_event(char *line) {

	int fd,arg,arg2,result;
	long long id;

	sscanf(line,"%*c %d %d %d",&fd,&arg,&arg2);

	errno=0;

	switch(arg) {
		case PERF_EVENT_IOC_SET_OUTPUT:
			if (arg2==-1) {
				result=ioctl(fd_remap[fd],arg,arg2);
			}
			else {
				result=ioctl(fd_remap[fd],arg,fd_remap[arg2]);
			}
			break;
		case PERF_EVENT_IOC_ID:
			result=ioctl(fd_remap[fd],arg,&id);
			break;
		default:
			result=ioctl(fd_remap[fd],arg,arg2);
			break;
	}

	if (result<0) {
		fprintf(stderr,"Line %lld Error with ioctl %d %d on %d/%d : %s\n",
			line_num,arg,arg2,fd,fd_remap[fd],strerror(errno));
		error=1;
		return;
	}

}


#define MAX_READ_SIZE 65536

static long long data[MAX_READ_SIZE];


static void read_event(char *line) {

	int result,read_size,fd;

	sscanf(line,"%*c %d %d",&fd,&read_size);

	if (read_size>MAX_READ_SIZE*sizeof(long long)) {
		fprintf(stderr,"Line %lld Read size of %d exceeds max of %ld\n",
			line_num,read_size,MAX_READ_SIZE*sizeof(long long));
		error=1;
		return;
	}

	result=read(fd_remap[fd],data,read_size);

	if (result<0) {
		fprintf(stderr,"Line %lld Error reading %d bytes from %d/%d\n",
			line_num,read_size,fd,fd_remap[fd]);
		error=1;
		return;
	}
}

static void prctl_event(char *line) {

        int enable=0;

        sscanf(line,"%*c %d",&enable);

        if (enable) {
                prctl(PR_TASK_PERF_EVENTS_ENABLE);
        }
        else {
                prctl(PR_TASK_PERF_EVENTS_DISABLE);
        }
}


static void poll_event(char *line) {

#define MAX_POLL_FDS 128

        int i,result,num_fds;

        struct pollfd pollfds[MAX_POLL_FDS];
        int timeout;
	char *next;

        sscanf(line,"%*c %d",&num_fds);

	strtok(line," ");	/* Point to num_fds */
	strtok(NULL," ");	/* Point to first value */

//	printf("p %d \n",num_fds);

        for(i=0;i<num_fds;i++) {
		next=strtok(NULL," ");
                pollfds[i].fd=atoi(next);
		next=strtok(NULL," ");
                pollfds[i].events=atoi(next);
//		printf("%d %d ",pollfds[i].fd,pollfds[i].events);
        }

	next=strtok(NULL," ");

        timeout=atoi(next);
//	printf("%d\n",timeout);

	/* quiet a warning */
	(void)result;

        result=poll(pollfds,num_fds,timeout);

}


static int forked_pid=0;

static void fork_event(char *line) {

        int enable=0;

        sscanf(line,"%*c %d",&enable);

        if (enable) {
                forked_pid=fork();
                if (forked_pid==0) while(1);
        }
        else {
                kill(forked_pid,SIGKILL);
        }

}


#define REPLAY_OPEN	0x001
#define REPLAY_CLOSE	0x002
#define REPLAY_IOCTL	0x004
#define REPLAY_READ	0x008
#define REPLAY_MMAP	0x010
#define REPLAY_MUNMAP	0x020
#define REPLAY_PRCTL	0x040
#define REPLAY_FORK	0x080
#define REPLAY_POLL	0x100
#define REPLAY_SEED	0x200
#define REPLAY_ALL	0xfff


void print_usage(char *exec_name) {

	fprintf(stderr,"\nUsage: %s [-h] [-s lines] [-r OCIRMUPF] filename\n\n",
		exec_name);
}

int main(int argc, char **argv) {

	FILE *logfile;
	char *logfile_name=NULL;
	char line[BUFSIZ];
	char *result;
	long long total_syscalls=0,replay_syscalls=0;
	long long skip_lines=0;

	int i,j;

	int replay_which=REPLAY_ALL;

	/* init */

	for(i=0;i<FD_REMAP_SIZE;i++) fd_remap[i]=-1;

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
			case 's':	i++;
					if (i<argc) {
						skip_lines=atoll(argv[i]);
						printf("skipping %lld lines\n",skip_lines);
						i++;
					}
					break;

			case 'r':	replay_which=0;
					i++;
					for(j=0;j<strlen(argv[i]);j++) {
						switch(argv[i][j]) {
						case 'O':	replay_which|=REPLAY_OPEN;
								break;
						case 'C':	replay_which|=REPLAY_CLOSE;
								break;
						case 'I':	replay_which|=REPLAY_IOCTL;
								break;
						case 'R':	replay_which|=REPLAY_READ;
								break;
						case 'M':	replay_which|=REPLAY_MMAP;
								break;
						case 'U':	replay_which|=REPLAY_MUNMAP;
								break;
						case 'P':	replay_which|=REPLAY_PRCTL;
								break;
						case 'F':	replay_which|=REPLAY_FORK;
								break;
						default:	fprintf(stderr,"Unknown replay %c\n",
									argv[i][j]);
						}
					}
					i+=2;
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


	while(1) {
		result=fgets(line,BUFSIZ,logfile);
		if (result==NULL) break;

		line_num++;

		if (line_num<skip_lines) continue;

		switch(line[0]) {
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
	}

	printf("Replayed %lld of %lld syscalls\n",
		replay_syscalls,total_syscalls);

	return 0;

}
