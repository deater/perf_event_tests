#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <linux/hw_breakpoint.h>

#include "../include/perf_event.h"
#include "../include/perf_helpers.h"

#include "perf_attr_print.h"

static int error=0;

static void mmap_event(char *line) {

	int fd,size;

	sscanf(line,"%*c %d %d",&size,&fd);

	printf("\tmmap_result[%d]=mmap(NULL, %d,"
		"PROT_READ|PROT_WRITE, MAP_SHARED,"
		"fd[%d], 0);\n",fd,size,fd);
}


static void munmap_event(char *line) {

	int fd,size;

	sscanf(line,"%*c %d %d",&fd,&size);

	printf("munmap(mmap_result[%d], %d);\n",fd,size);
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

	perf_pretty_print_attr(&pe,orig_fd);

	printf("\tfd[%d]=perf_event_open(&pe[%d],%d,%d,",
		orig_fd,orig_fd,pid,cpu);
	if (group_fd==-1) {
		printf("-1,");
	}
	else {
		printf("fd[%d],",group_fd);
	}

	perf_pretty_print_flags(flags);
	printf(" /*%lx*/ ",flags);

	printf(");\n");


	printf("\n");

}

static void close_event(char *line) {

	int fd;

	sscanf(line,"%*c %d",&fd);

	printf("\tclose(fd[%d]);\n",fd);
}


static void ioctl_event(char *line) {

	int fd,arg,arg2;

	sscanf(line,"%*c %d %d %d",&fd,&arg,&arg2);
	printf("\tioctl(fd[%d],%d,%d);\n",fd,arg,arg2);
}


static void read_event(char *line) {

	int read_size,fd;

	sscanf(line,"%*c %d %d",&fd,&read_size);

	printf("\tread(fd[%d],data,%d);\n",fd,read_size);
}

#define NUM_VALUES 1024

int main(int argc, char **argv) {

	FILE *logfile;
	char line[BUFSIZ];
	char *result;
	long long total_syscalls=0;

	if (argc<2) {
		fprintf(stderr,"\nUsage: %s logfile\n\n",
			argv[0]);
		exit(1);
	}

	logfile=fopen(argv[1],"r");
	if (logfile==NULL) {
		fprintf(stderr,"Error opening %s\n",argv[1]);
		exit(1);
	}

	printf("/* log_to_code output from %s */\n",argv[1]);
	printf("/* by Vince Weaver <vincent.weaver _at_ maine.edu */\n\n");

	printf("#include <stdio.h>\n");
	printf("#include <unistd.h>\n");
	printf("#include <string.h>\n");
	printf("#include <sys/mman.h>\n");
	printf("#include <sys/syscall.h>\n");
	printf("#include <sys/ioctl.h>\n");
	printf("#include <linux/hw_breakpoint.h>\n");
	printf("#include <linux/perf_event.h>\n");

	printf("\n");

	printf("int fd[%d];\n",NUM_VALUES);
	printf("struct perf_event_attr pe[%d];\n",NUM_VALUES);
	printf("char *mmap_result[%d];\n\n",NUM_VALUES);

	printf("int perf_event_open(struct perf_event_attr *hw_event_uptr,\n");
	printf("\tpid_t pid, int cpu, int group_fd, unsigned long flags) {\n");
	printf("\n");
	printf("\treturn syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,\n");
	printf("\t\tgroup_fd, flags);\n");
	printf("}\n\n");

	printf("int main(int argc, char **argv) {\n");

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
			case 'U':
				munmap_event(line);
				break;
			default:
				fprintf(stderr,"Unknown log type \'%c\'\n",
					line[0]);
				break;
		}
		if (error) break;
		total_syscalls++;
	}

	printf("\t/* Replayed %lld syscalls */\n",total_syscalls);

	printf("\treturn 0;\n");
	printf("}\n");

	return 0;

}
