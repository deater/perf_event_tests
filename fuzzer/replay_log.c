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
#include <sys/wait.h>
#include <poll.h>

#include <fcntl.h>

#include <shm.h>
#include <syscall.h>
#include "perf_event.h"
#include "perf_helpers.h"

#include "fuzz_compat.h"

#include "perf_attr_print.h"

static int error=0;
static unsigned long long line_num=0;

#define FD_REMAP_SIZE 2048


static int fd_overflows[FD_REMAP_SIZE];
static int fd_throttles[FD_REMAP_SIZE];

struct event_type {
	int fd_remap;
	char *mmap_remap;
	int size;
} event[FD_REMAP_SIZE];

static int debug=0;

static int original_pid=-1;

	int page_size;
	struct shm_s *shm;
	char *page_rand;
	extern struct syscallentry syscall_perf_event_open;


static void mmap_event(char *line) {

	int fd,size,remapped_fd;
	char *data;
	unsigned long long address;

	sscanf(line,"%*c %d %d %llx",&size,&fd,&address);

	remapped_fd=event[fd].fd_remap;

	if (remapped_fd==-1) {
		fprintf(stderr,"Line %lld Skipping mmap as fd %d not valid\n",
			line_num,fd);
		return;
	}

	/* Clear this out, in case we fail */
	event[fd].mmap_remap=MAP_FAILED;
	event[fd].size=0;


#ifdef FORCE_SAME_MMAP_ADDR
	data=mmap((void *)(long)address, size,
		PROT_READ|PROT_WRITE, MAP_SHARED,
		remapped_fd, 0);

#else
	data=mmap(NULL, size,
		PROT_READ|PROT_WRITE, MAP_SHARED,
		remapped_fd, 0);
#endif
	if (data==MAP_FAILED) {
		fprintf(stderr,"Line %lld: Error with mmap of size %d of %d/%d\n",
			line_num,size,fd,remapped_fd);
		error=1;
	}
	else {
		fprintf(stderr,"VMW MAP fd=%d addr=%p size=%d\n",fd,data,size);

		event[fd].mmap_remap=data;
		event[fd].size=size;
	}

//		fcntl(event_data[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
//		fcntl(event_data[i].fd, F_SETSIG, SIGRTMIN+2);
//		fcntl(event_data[i].fd, F_SETOWN,getpid());

}

static void trash_mmap_event(char *line) {

	int fd,value,size;

	sscanf(line,"%*c %d %d %d",&value,&size,&fd);

	if (event[fd].fd_remap==-1) {
		fprintf(stderr,"Line %lld Skipping trash mmap as fd %d not valid\n",
			line_num,fd);
		return;
	}

	if (event[fd].mmap_remap==MAP_FAILED) {
		fprintf(stderr,"Line %lld Skipping trash mmap as fd %d not valid\n",
			line_num,fd);
		return;
	}

	if (event[fd].mmap_remap==NULL) {
		fprintf(stderr,"Line %lld Skipping trash mmap as fd %d not valid\n",
			line_num,fd);
		return;
	}

//	printf("Trashing %p with %x\n",mmap_remap[fd_remap[fd]],value);

	memset(event[fd].mmap_remap,value,size);

}

/* Urgh, we allow unmapping after closing now */

static void munmap_event(char *line) {

	int fd,size,result,remapped_fd;
	void *mmap_address;

	sscanf(line,"%*c %d %d",&fd,&size);

	remapped_fd=event[fd].fd_remap;

	/* Might have already been closed */
	/* This should be OK as long as mmap value still good*/
	if (remapped_fd==-1) {
		if (event[fd].mmap_remap==MAP_FAILED) {
			fprintf(stderr,"Line: %lld Skipping munmap as fd %d fd and mmap not valid\n",
				line_num,fd);
		}
		return;
	}

	mmap_address=event[fd].mmap_remap;
	/* Mark it as unmapped */
	event[fd].mmap_remap=MAP_FAILED;

	if (event[fd].size!=size) {
		fprintf(stderr,"Line %lld size didn't match (%d!=%d)\n",
			line_num,size,event[fd].size);
		return;
	}

	if (mmap_address==MAP_FAILED) {
		fprintf(stderr,"Line: %lld Skipping munmap as data is invalid\n",
			line_num);
		return;

	}

	result=munmap(mmap_address, size);

	if (result<0) {
		fprintf(stderr,"Line: %lld Error with munmap of %p size %d of %d\n",
			line_num,mmap_address,size,fd);
		error=1;
		return;
	}

	fprintf(stderr,"VMW UNMAP %d %p %d\n",fd,mmap_address,size);

}


static void open_event(char *line) {


	struct perf_event_attr *pe;
	int fd,orig_fd,remapped_group_fd;
	pid_t pid;
	int cpu,group_fd,orig_size;
	long int flags;

	unsigned char zeros[4096];

	/* urgh, necessary.  Took forever to track this down  */
	/* data from the poll structure was leaking in if not */
	/* entirely zero, and also getting E2BIG errors if we */
	/* set the size to a "too big" value and there were   */
	/* non-zero values in that space.                     */
	/* The max size of a struct is the pagesize, so make  */
	/* all events live in a sea of zeros to avoid problems*/

	memset(&zeros,0,4096);
	pe=(struct perf_event_attr *)&zeros;

	/* I hate bitfields */
	int disabled,inherit,pinned,exclusive;
	int exclude_user,exclude_kernel,exclude_hv,exclude_idle;
	int mmap,comm,freq,inherit_stat;
	int enable_on_exec,task,watermark,precise_ip;
	int mmap_data,sample_id_all,exclude_host,exclude_guest;
	int exclude_callchain_user,exclude_callchain_kernel;
	int mmap2,comm_exec;
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
		"%d %d %lld %d %d %d",
		&orig_fd,&pid,&cpu,&group_fd,&flags,
		&pe->type,&pe->size,
		&pe->config,&pe->sample_period,&pe->sample_type,&pe->read_format,
		&disabled,&inherit,&pinned,&exclusive,
		&exclude_user,&exclude_kernel,&exclude_hv,&exclude_idle,
		&mmap,&comm,&freq,&inherit_stat,
		&enable_on_exec,&task,&watermark,&precise_ip,
		&mmap_data,&sample_id_all,&exclude_host,&exclude_guest,
		&pe->wakeup_events,&pe->bp_type,
		&pe->config1,&pe->config2,&pe->branch_sample_type,
		&exclude_callchain_kernel,&exclude_callchain_user,
		&pe->sample_regs_user,&pe->sample_stack_user,&mmap2,&comm_exec);

	errno=0;

	/* use recorded value for pid not our actual pid */
	if (pid==original_pid) {
		pid=getpid();
	}

	/* re-populate bitfields */
	/* can't sscanf into them */
	pe->disabled=disabled;
	pe->inherit=inherit;
	pe->pinned=pinned;
	pe->exclusive=exclusive;
	pe->exclude_user=exclude_user;
	pe->exclude_kernel=exclude_kernel;
	pe->exclude_hv=exclude_hv;
	pe->exclude_idle=exclude_idle;
	pe->mmap=mmap;
	pe->comm=comm;
	pe->freq=freq;
	pe->inherit_stat=inherit_stat;
	pe->enable_on_exec=enable_on_exec;
	pe->task=task;
	pe->watermark=watermark;
	pe->precise_ip=precise_ip;
	pe->mmap_data=mmap_data;
	pe->sample_id_all=sample_id_all;
	pe->exclude_host=exclude_host;
	pe->exclude_guest=exclude_guest;
	pe->exclude_callchain_user=exclude_callchain_user;
	pe->exclude_callchain_kernel=exclude_callchain_kernel;
	pe->mmap2=mmap2;
	pe->comm_exec=comm_exec;

	/* kernel over-writes this sometimes :( */
	orig_size=pe->size;

	if (group_fd==-1) {
		remapped_group_fd=-1;
	} else {
		remapped_group_fd=event[group_fd].fd_remap;
	}


	fd=perf_event_open(pe,pid,cpu,remapped_group_fd,flags);
	if (fd<0) {

		fprintf(stderr,"Line %lld Error opening %s : %s\n",
			line_num,line,strerror(errno));

                perf_pretty_print_event(stderr,orig_fd,original_pid,
				pe, pid, cpu,
				remapped_group_fd,flags);

		if (errno==E2BIG) {
			printf("Too big!  Kernel returns %d we were %d\n",
				pe->size,orig_size);
		}
#if 0
	{
		int i;
		char *blah;

		blah=(char *)&pe;

		printf("BEFORE\n");
		for(i=0;i<100;i++) {
			printf("%d:%2x ",i,blah[i]);
		}
		printf("AFTER\n");
	}
#endif

		error=1;
		return;
	}

	if (orig_fd>FD_REMAP_SIZE) {
		fprintf(stderr,"fd out of range\n");
		error=1;
		return;
	}

	fprintf(stderr,"VMW OPEN %d (remapped %d)\n",orig_fd,fd);
	event[orig_fd].fd_remap=fd;
	event[orig_fd].mmap_remap=MAP_FAILED;

	if (fd>FD_REMAP_SIZE) {
		fprintf(stderr,"overflow fd out of range\n");
		error=1;
		return;
	}
	fd_overflows[fd]=0;
	fd_throttles[fd]=0;
}



static void close_event(char *line) {

	int fd;
	int result;

	sscanf(line,"%*c %d",&fd);

	if (event[fd].fd_remap==-1) {
		fprintf(stderr,"Line %lld Skipping close as fd %d not valid\n",
			line_num,fd);
		return;
	}

	result=close(event[fd].fd_remap);
	if (result<0) {
		fprintf(stderr,"Line %lld Error closing %d/%d\n",
			line_num,fd,event[fd].fd_remap);

	}
	fprintf(stderr,"VMW CLOSE %d (%d)\n",fd,event[fd].fd_remap);
	event[fd].fd_remap=-1;

}


static void ioctl_event(char *line) {

	int fd,arg,result;
	long long id;
	long long period;
	long long arg2;

	sscanf(line,"%*c %d %d %lld",&fd,&arg,&arg2);

	errno=0;

	switch(arg) {
		case PERF_EVENT_IOC_SET_OUTPUT:
			if (arg2==-1) {
				result=ioctl(event[fd].fd_remap,arg,arg2);
			}
			else {
				result=ioctl(event[fd].fd_remap,arg,event[arg2].fd_remap);
			}
			break;
		case PERF_EVENT_IOC_ID:
			result=ioctl(event[fd].fd_remap,arg,&id);
			break;
		case PERF_EVENT_IOC_PERIOD:
			period=arg2;
			result=ioctl(event[fd].fd_remap,arg,&period);
			break;
		default:
			result=ioctl(event[fd].fd_remap,arg,arg2);
			break;
	}

	if (result<0) {
		fprintf(stderr,"Line %lld Error with ioctl %d %lld on %d/%d : %s\n",
			line_num,arg,arg2,fd,event[fd].fd_remap,strerror(errno));
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
		fprintf(stderr,"Line %lld Read size of %d exceeds max of %zd\n",
			line_num,read_size,MAX_READ_SIZE*sizeof(long long));
		error=1;
		return;
	}

	result=read(event[fd].fd_remap,data,read_size);

	if (result<0) {
		fprintf(stderr,"Line %lld Error reading %d bytes from %d/%d\n",
			line_num,read_size,fd,event[fd].fd_remap);
		error=1;
		return;
	}
}

#define MAX_FILENAMES 6
char *filenames[MAX_FILENAMES]={
        "/proc/sys/kernel/perf_cpu_time_max_percent",
        "/proc/sys/kernel/perf_event_paranoid",
        "/proc/sys/kernel/perf_event_max_sample_rate",
        "/proc/sys/kernel/perf_event_max_stack",
        "/proc/sys/kernel/perf_event_mlock_kb",
        "/proc/sys/kernel/nmi_watchdog"
};

static void access_event(char *line) {

	long long size;
	int which,result,type;
	char buffer[2048];
	FILE *fff;

	sscanf(line,"%*c %d %d %lld %d",&type,&which,&size,&result);

	if (which>MAX_FILENAMES) {
		fprintf(stderr,"Error access_event too big\n");
		exit(0);
	}

	/* read */
	if (type==0) {
		fff=fopen(filenames[which],"r");
		if (fff!=NULL) {
                        result=fread(buffer,sizeof(char),size,fff);
			fclose(fff);
		}
	}
	else {
		fff=fopen(filenames[which],"w");
		if (fff!=NULL) {
			result=fprintf(fff,"%lld\n",size);
			fclose(fff);
		}

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
        int timeout=0;
	char *next;

        sscanf(line,"%*c %d",&num_fds);

	strtok(line," ");	/* Point to num_fds */
	strtok(NULL," ");	/* Point to first value */

//	printf("p %d \n",num_fds);

        for(i=0;i<num_fds;i++) {
		next=strtok(NULL," ");
		if (next==NULL) {
			printf("Unexpected end! %d < %d\n",i,num_fds);
			break;
		}
                pollfds[i].fd=atoi(next);
		next=strtok(NULL," ");
		if (next==NULL) {
			printf("Unexpected end! %d < %d\n",i,num_fds);
			break;
		}
                pollfds[i].events=atoi(next);
//		printf("%d %d/%d %d\n",
//			i,pollfds[i].fd,
//			fd_remap[pollfds[i].fd],
//			pollfds[i].events);
//		printf("Next: %p\n",next);
        }

	next=strtok(NULL," ");
	if (next==NULL) {
		printf("Unexpected no timeout \n");
	}
	else {
	        timeout=atoi(next);
	}

//	printf("%d\n",timeout);

	/* quiet a warning */
	(void)result;

        result=poll(pollfds,num_fds,timeout);

}


static int forked_pid=0;

static void fork_event(char *line) {

        int enable=0;
	int status;

        sscanf(line,"%*c %d",&enable);

        if (enable) {
                forked_pid=fork();
                if (forked_pid==0) while(1);
        }
        else {
                kill(forked_pid,SIGKILL);
		/* We had some race conditions with breakpoints   */
		/* If we didn't let the child finish dying before */
		/* continuing.  Sometimes a new breakpoint would be */
		/* attempted before the child's resources had freed */
		waitpid(forked_pid, &status, 0);
        }

}

static int overflows=0;

static void our_handler(int signum, siginfo_t *info, void *uc) {

	int fd = info->si_fd;
	int ret;

	char string[BUFSIZ];

	overflows++;
	/* disable the event for the time being */
	/* we were having trouble with signal storms */
	ioctl(fd,PERF_EVENT_IOC_DISABLE,0);

	fd_overflows[fd]++;
	if (fd_overflows[fd]>10000) {
		sprintf(string,"Throttling event %d\n",fd);
		write(1,string,strlen(string));
		fd_throttles[fd]++;
		fd_overflows[fd]=0;

#define MAX_THROTTLES 20

		if (fd_throttles[fd]>MAX_THROTTLES) {
			sprintf(string,"Hit max throttles\n");
			write(1,string,strlen(string));

			close(fd);
			event[fd].fd_remap=-1;
		}
	}

	else {
		ret=ioctl(fd, PERF_EVENT_IOC_REFRESH,1);
	}

        (void) ret;

}

static void sigio_handler(int signum, siginfo_t *info, void *uc) {


}


static void setup_overflow(char *line) {

        int overflow_fd;
	struct sigaction sa;

        sscanf(line,"%*c %d",&overflow_fd);

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = our_handler;
	sa.sa_flags = SA_SIGINFO;

	if (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {
		printf("Error setting up signal handler\n");
	}

	fcntl(event[overflow_fd].fd_remap, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(event[overflow_fd].fd_remap, F_SETSIG, SIGRTMIN+2);
	fcntl(event[overflow_fd].fd_remap, F_SETOWN,getpid());
}


#define REPLAY_OPEN		0x0001
#define REPLAY_CLOSE		0x0002
#define REPLAY_IOCTL		0x0004
#define REPLAY_READ		0x0008
#define REPLAY_MMAP		0x0010
#define REPLAY_MUNMAP		0x0020
#define REPLAY_PRCTL		0x0040
#define REPLAY_FORK		0x0080
#define REPLAY_POLL		0x0100
#define REPLAY_SEED		0x0200
#define REPLAY_OVERFLOW		0x0400
#define REPLAY_TRASH_MMAP	0x0800
#define REPLAY_ACCESS		0x1000
#define REPLAY_ALL		0xffff

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



void print_usage(char *exec_name) {

	fprintf(stderr,"\nUsage: %s [-h] [-s lines] [-p lines] [-r OCIRMUPFp] filename\n\n",
		exec_name);
}

static char line[BUFSIZ];


int main(int argc, char **argv) {

	FILE *logfile,*fff;
	char *logfile_name=NULL;
	char *result;
	long long total_syscalls=0,replay_syscalls=0;

	long long skip_lines=0;
	long long stop_lines=0;

	struct sigaction sigio;

	int i,j,seed=0xdeadbeef;

	int replay_which=REPLAY_ALL;

	int sample_rate,old_sample_rate;

	/* init */

	for(i=0;i<FD_REMAP_SIZE;i++) {
		event[i].fd_remap=-1;
		event[i].mmap_remap=MAP_FAILED;
		event[i].size=0;
	}

//	if (argc<2) {
//		print_usage(argv[0]);
//		exit(1);
//	}

	i=1;
	while(1) {
		if (i>=argc) break;

		if (argv[i][0]=='-') {
			switch(argv[i][1]) {
			case 'h':	print_usage(argv[0]);
					exit(1);
					break;
			case 'p':	i++;
					if (i<argc) {
						stop_lines=atoll(argv[i]);
						printf("stopping after %lld lines\n",stop_lines);
						i++;
					}
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
						case 'o':	replay_which|=REPLAY_OVERFLOW;
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
						case 'p':	replay_which|=REPLAY_POLL;
								break;
						default:	fprintf(stderr,"Unknown replay %c\n",
									argv[i][j]);
						}
					}
					i++;
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
//		fprintf(stderr,"Must specify logfile name\n");
//		exit(1);
		logfile=stdin;
	} else {

		logfile=fopen(logfile_name,"r");
		if (logfile==NULL) {
			fprintf(stderr,"Error opening %s\n",logfile_name);
			exit(1);
		}
	}

	printf("Replaying...\n");

	/* Write fake seed to disk */
	/* trying to make the syscall trace more similar to the actual fuzzer */
        fff=fopen("fake.seed","w");
        if (fff!=NULL) {
                fprintf(fff,"%d\n",seed);
                fclose(fff);
        }



        /* Save the content of /proc/sys/kernel/perf_event_max_sample_rate */
        /* If it has been changed, a replay might not be perfect */
        sample_rate=get_sample_rate();

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


	while(1) {
		result=fgets(line,BUFSIZ,logfile);
		if (result==NULL) break;

		line_num++;

		if (debug) printf("%lld %s",line_num,line);

		/* don't want to skip the random seed, etc */
		if ((line_num>2) && (line_num<skip_lines)) continue;

		switch(line[0]) {
			case 'A':
				if (replay_which & REPLAY_ACCESS) {
					access_event(line);
					replay_syscalls++;
				}
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
			case 'r':
				sscanf(line,"r %d",&old_sample_rate);
				//sample_rate=get_sample_rate();

				if (sample_rate!=old_sample_rate) {
					printf("Warning! The current max sample rate is %d\n",sample_rate);
					printf("\tThis log was recorded when it was %d\n",old_sample_rate);
					printf("\tFor proper replay you might want to (as root):\n\techo \"%d\" > /proc/sys/kernel/perf_event_max_sample_rate\n",old_sample_rate);
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

			case '#':
				/* skip */
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

		if (stop_lines && (total_syscalls > stop_lines)) break;
	}

	/* Kill any lingering children */
	if (forked_pid) {
                kill(forked_pid,SIGKILL);
	}

	printf("Replayed %lld of %lld syscalls\n",
		replay_syscalls,total_syscalls);

	return 0;

}
