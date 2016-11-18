#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include <poll.h>

#include "hw_breakpoint.h"
#include "perf_event.h"
#include "perf_helpers.h"

#include "perf_attr_print.h"

static int error=0;
static unsigned long long line_num=0;

static int original_pid=-1;

static void mmap_event(char *line) {

	int fd,size;

	sscanf(line,"%*c %d %d",&size,&fd);

	printf("\tmmap_result[%d]=mmap(NULL, %d,"
		"PROT_READ|PROT_WRITE, MAP_SHARED,"
		"fd[%d], 0);\n",fd,size,fd);
}

static void trash_mmap_event(char *line) {

	int fd,size,value;

	sscanf(line,"%*c %d %d %d",&value,&size,&fd);

	printf("\tif (mmap_result[%d]>0) memset(mmap_result[%d],%d,%d);\n",
		fd,fd,value,size);

}


static void munmap_event(char *line) {

	int fd,size;

	sscanf(line,"%*c %d %d",&fd,&size);

	printf("\tmunmap(mmap_result[%d], %d);\n",fd,size);
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
	int exclude_callchain_kernel,exclude_callchain_user;
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
		"%d %d %lld %d %d ",
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
	pe.exclude_callchain_user=exclude_callchain_user;
	pe.exclude_callchain_kernel=exclude_callchain_kernel;
	pe.mmap2=mmap2;

	perf_pretty_print_event(stdout,orig_fd,original_pid,
				&pe,pid,cpu,group_fd,flags);

	printf("\n");

}

static void close_event(char *line) {

	int fd;

	sscanf(line,"%*c %d",&fd);

	printf("\tclose(fd[%d]);\n",fd);
}

static void ioctl_arg_print(int arg) {
	if (arg&PERF_IOC_FLAG_GROUP) {
		printf("PERF_IOC_FLAG_GROUP | ");
	}
	printf("%d",arg);
}

static void ioctl_event(char *line) {

	int fd,arg;
	long long arg2;

	sscanf(line,"%*c %d %d %lld",&fd,&arg,&arg2);

	switch(arg) {
		case PERF_EVENT_IOC_ENABLE:
			printf("\tioctl(fd[%d],PERF_EVENT_IOC_ENABLE,",fd);
			ioctl_arg_print(arg2);
			printf(");\n");
			break;
		case PERF_EVENT_IOC_DISABLE:
			printf("\tioctl(fd[%d],PERF_EVENT_IOC_DISABLE,",fd);
			ioctl_arg_print(arg2);
			printf(");\n");
			break;
		case PERF_EVENT_IOC_REFRESH:
			printf("\tioctl(fd[%d],PERF_EVENT_IOC_REFRESH,%lld);\n",
				fd,arg2);
			break;
		case PERF_EVENT_IOC_RESET:
			printf("\tioctl(fd[%d],PERF_EVENT_IOC_RESET,",fd);
			ioctl_arg_print(arg2);
			printf(");\n");
			break;
		case PERF_EVENT_IOC_PERIOD:
			printf("\tperiod=%lld;\n",arg2);
			printf("\tioctl(fd[%d],PERF_EVENT_IOC_PERIOD,&period);\n",
				fd);
			break;
		case PERF_EVENT_IOC_SET_OUTPUT:
			if (arg2==-1) {
				printf("\tioctl(fd[%d],"
					"PERF_EVENT_IOC_SET_OUTPUT,%lld);\n",
					fd,arg2);
			}
			else {
				printf("\tioctl(fd[%d],"
					"PERF_EVENT_IOC_SET_OUTPUT,fd[%lld]);\n",
					fd,arg2);
			}
			break;
		case PERF_EVENT_IOC_SET_FILTER:
			printf("\tioctl(fd[%d],PERF_EVENT_IOC_SET_FILTER,%lld);\n",
				fd,arg2);
			break;
		case PERF_EVENT_IOC_ID:
			printf("\tioctl(fd[%d],PERF_EVENT_IOC_ID,&id);\n",fd);
			break;
		default:
			printf("\tioctl(fd[%d],%d,%lld);\n",fd,arg,arg2);
			break;
	}

}


static void read_event(char *line) {

	int read_size,fd;

	sscanf(line,"%*c %d %d",&fd,&read_size);

	printf("\tread(fd[%d],data,%d);\n",fd,read_size);
}

static void prctl_event(char *line) {

	int enable=0;

	sscanf(line,"%*c %d",&enable);

	if (enable) {
		printf("\tprctl(PR_TASK_PERF_EVENTS_ENABLE);\n");
	}
	else {
		printf("\tprctl(PR_TASK_PERF_EVENTS_DISABLE);\n");
	}
}


static void poll_event(char *line) {

#define MAX_POLL_FDS 128

        int i,num_fds;

        struct pollfd pollfds[MAX_POLL_FDS];
        int timeout;
        char *next;

        sscanf(line,"%*c %d",&num_fds);

        strtok(line," ");       /* Point to num_fds */
        strtok(NULL," ");       /* Point to first value */

        for(i=0;i<num_fds;i++) {
                next=strtok(NULL," ");
                pollfds[i].fd=atoi(next);
                next=strtok(NULL," ");
                pollfds[i].events=atoi(next);
		printf("\tpollfds[%d].fd=fd[%d]; pollfds[%d].events=%d;\n",
			i,pollfds[i].fd,i,pollfds[i].events);
        }
        next=strtok(NULL," ");

        timeout=atoi(next);

	printf("\tpoll(pollfds,%d,%d);\n",num_fds,timeout);

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

	sscanf(line,"%*c %d %d %lld %d",&type,&which,&size,&result);

	if (which>=MAX_FILENAMES) {
		fprintf(stderr,"access_event out of bounds!\n");
		exit(1);
	}

	/* read */
	if (type==0) {
		printf("\tfff=fopen(\"%s\",\"r\");\n",filenames[which]);
		printf("\tif (fff!=NULL) {\n");
		printf("\t\tresult=fread(buffer,sizeof(char),%lld,fff);\n",size);
		printf("\t\tfclose(fff);\n");
		printf("\t}\n");
	}

	else {
		printf("\tfff=fopen(\"%s\",\"w\");\n",filenames[which]);
		printf("\t\tif (fff!=NULL) {\n");
		printf("\t\tresult=fprintf(fff,\"%%lld\",%lldULL);\n",size);
		printf("\t\tfclose(fff);\n");
		printf("\t}\n");
        }
}

static void fork_event(char *line) {

	int enable=0;

	sscanf(line,"%*c %d",&enable);

	if (enable) {
		printf("\tforked_pid=fork();\n");
                printf("\tif (forked_pid==0) while(1);\n");
	}
	else {
		printf("\tkill(forked_pid,SIGKILL);\n");
		/* We had some race conditions with breakpoint events */
		/* if we didn't wait for the child to finish dying    */
		printf("\twaitpid(forked_pid, &status, 0);\n");
	}

}

static void setup_overflow(char *line) {

	int overflow_fd;

	sscanf(line,"%*c %d",&overflow_fd);

	printf("\tmemset(&sa, 0, sizeof(struct sigaction));\n");
	printf("\tsa.sa_sigaction = our_handler;\n");
	printf("\tsa.sa_flags = SA_SIGINFO;\n");

	printf("\tif (sigaction( SIGRTMIN+2, &sa, NULL) < 0) {\n");
        printf("\t\tprintf(\"Error setting up signal handler\\n\");\n");
	printf("\t}\n");

        printf("\tfcntl(fd[%d], F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);\n",overflow_fd);
        printf("\tfcntl(fd[%d], F_SETSIG, SIGRTMIN+2);\n",overflow_fd);
        printf("\tfcntl(fd[%d], F_SETOWN,getpid());\n",overflow_fd);
}




#define NUM_VALUES 1024

int main(int argc, char **argv) {

	FILE *logfile;
	char line[BUFSIZ];
	char *result;
	long long total_syscalls=0;
	int random_seed;
	int max_sample_rate;

	if (argc<2) {
		logfile=stdin;
//		fprintf(stderr,"\nUsage: %s logfile\n\n",
//			argv[0]);
//		exit(1);
	}
	else {
		logfile=fopen(argv[1],"r");
		if (logfile==NULL) {
			fprintf(stderr,"Error opening %s\n",argv[1]);
			exit(1);
		}
	}

	printf("/* log_to_code output from %s */\n",argv[1]);
	printf("/* by Vince Weaver <vincent.weaver _at_ maine.edu> */\n\n");

	printf("#define _GNU_SOURCE 1\n");
	printf("#include <stdio.h>\n");
	printf("#include <unistd.h>\n");
	printf("#include <fcntl.h>\n");
	printf("#include <string.h>\n");
	printf("#include <signal.h>\n");
	printf("#include <sys/mman.h>\n");
	printf("#include <sys/syscall.h>\n");
	printf("#include <sys/ioctl.h>\n");
	printf("#include <sys/prctl.h>\n");
	printf("#include <sys/wait.h>\n");
	printf("#include <poll.h>\n");
	printf("#include <linux/hw_breakpoint.h>\n");
	printf("#include <linux/perf_event.h>\n");

	printf("\n");

	printf("static int fd[%d];\n",NUM_VALUES);
	printf("static struct perf_event_attr pe[%d];\n",NUM_VALUES);
	printf("static char *mmap_result[%d];\n",NUM_VALUES);

	printf("#define MAX_READ_SIZE 65536\n");
	printf("static long long data[MAX_READ_SIZE];\n");

	printf("\n");

	printf("#define MAX_POLL_FDS 128\n");
	printf("static struct pollfd pollfds[MAX_POLL_FDS];\n");

	printf("\n");

	/* For ioctl(PERF_EVENT_IOC_ID); */
	printf("static long long id;\n");
	/* For ioctl(PERF_EVENT_IOC_PERIOD); */
	printf("static long long period;\n");

	printf("\n");

	printf("static int status;\n");
	printf("static int forked_pid;\n\n");

	printf("static struct sigaction sa;\n");
	printf("static int overflows=0;\n");
	printf("static int sigios=0;\n\n");

	printf("FILE *fff;\n");
	printf("static int result;\n");
	printf("static long long size;\n");
	printf("static char buffer[2048];\n\n");

	printf("static void our_handler(int signum, siginfo_t *info, void *uc) {\n");
	printf("\tint fd = info->si_fd;\n");
	printf("\tint ret;\n\n");
	printf("\toverflows++;\n");
	printf("\tioctl(fd,PERF_EVENT_IOC_DISABLE,0);\n");
	printf("\tif (sigios) return;\n");
	printf("\tret=ioctl(fd, PERF_EVENT_IOC_REFRESH,1);\n");
	printf("}\n\n");

	printf("int perf_event_open(struct perf_event_attr *hw_event_uptr,\n");
	printf("\tpid_t pid, int cpu, int group_fd, unsigned long flags) {\n");
	printf("\n");
	printf("\treturn syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,\n");
	printf("\t\tgroup_fd, flags);\n");
	printf("}\n\n");



	printf("int main(int argc, char **argv) {\n");

	/* Match how replay_log implements things */
	printf("\n\tint i;\n");
	printf("\tfor(i=0;i<%d;i++) fd[i]=-1;\n\n",NUM_VALUES);

	while(1) {
		result=fgets(line,BUFSIZ,logfile);
		if (result==NULL) break;

		line_num++;

		printf("/* %lld */\n",line_num);
		//printf("printf(\"Line: %lld\\n\");\n",line_num);

		switch(line[0]) {
			case 'A':
				access_event(line);
				total_syscalls++;
				break;
			case 'C':
				close_event(line);
				total_syscalls++;
				break;
			case 'F':
				fork_event(line);
				total_syscalls++;
				break;
			case 'G':
				sscanf(line,"%*c %d",&original_pid);
				break;
			case 'I':
				ioctl_event(line);
				total_syscalls++;
				break;
			case 'M':
				mmap_event(line);
				total_syscalls++;
				break;
			case 'O':
				open_event(line);
				total_syscalls++;
				break;
			case 'o':
				setup_overflow(line);
				total_syscalls++;
				break;
			case 'P':
				prctl_event(line);
				total_syscalls++;
				break;
			case 'p':
				poll_event(line);
				total_syscalls++;
				break;
			case 'Q':
				trash_mmap_event(line);
				total_syscalls++;
				break;
			case 'R':
				read_event(line);
				total_syscalls++;
				break;
			case 'r':
				sscanf(line,"%*c %d",&max_sample_rate);
				printf("/* /proc/sys/kernel/perf_event_max_sample_rate was %d */\n",max_sample_rate);
				break;
			case 'S':
				sscanf(line,"%*c %d",&random_seed);
				printf("/* Random Seed was %d */\n",random_seed);
				break;
			case 'U':
				munmap_event(line);
				total_syscalls++;

				break;
			default:
				fprintf(stderr,"Unknown log type \'%c\'\n",
					line[0]);
				break;
		}
		if (error) break;
	}

	printf("\n\n\t/* Replayed %lld syscalls */\n",total_syscalls);

	printf("\treturn 0;\n");
	printf("}\n");

	return 0;

}
