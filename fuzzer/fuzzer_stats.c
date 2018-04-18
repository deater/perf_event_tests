#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "fuzzer_stats.h"

/* Global Variables */
struct fuzzer_stats_t stats;

/* Local Variables */
static char open_type_names[MAX_OPEN_TYPE][20]={
	"Hardware","Software","Tracepoint","Cache",
	"Raw","Breakpoint","#6","#7",
	"#8","#9","#10","#11",
	"#12","#13","#14","#15",
	"#16","#17","#18",">19",
};

static char open_trinity_type_names[MAX_OPEN_TRINITY_TYPE][20]={
	"Normal","Sampling","Global","Random",
};

static char ioctl_type_names[MAX_IOCTL_TYPE][20]={
	"ENABLE","DISABLE","REFRESH","RESET",
	"PERIOD","SET_OUTPUT","SET_FILTER","ID",
	"SET_BPF","PAUSE_OUTPUT","QUERY_BPF","MOD_ATTR",
	"#12","#13","#14",">14"
};

void stats_set_pmu_name(int which, char *name) {
	strncpy(open_type_names[which],name,20);
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
		case EBUSY:	fprintf(fff,"EBUSY");
				break;
		case EAGAIN:	fprintf(fff,"EAGAIN");
				break;
		case EOVERFLOW:	fprintf(fff,"EOVERFLOW");
				break;

		default:	fprintf(fff,"UNKNOWN %d",e);
				break;
	}
}

void dump_summary(FILE *fff, int print_values, double elapsed) {

	int i;

	if (print_values) {

	fprintf(fff,"Iteration %lld, %lld syscalls in %.2lf s (%.3lf k syscalls/s)\n",
		stats.total_iterations,
		stats.total_syscalls,
		elapsed,
		(stats.total_syscalls/elapsed)/1000.0);
	fprintf(fff,"\tOpen attempts: %lld  Successful: %lld  Currently open: %lld\n",
	       stats.open_attempts,stats.open_successful,stats.current_open);
	for(i=0;i<MAX_ERRNOS;i++) {
		if (stats.open_errno_count[i]!=0) {
			fprintf(fff,"\t\t");
			print_errno_name(fff,i);
			fprintf(fff," : %d\n",stats.open_errno_count[i]);
		}
	}

	fprintf(fff,"\t\tTrinity Type ");
	for(i=0;i<MAX_OPEN_TRINITY_TYPE;i++) {
		fprintf(fff,"(%s %d/%d)",open_trinity_type_names[i],
				stats.open_trinity_type_success[i],
				stats.open_trinity_type_success[i]+
					stats.open_trinity_type_fail[i]);
	}
	fprintf(fff,"\n");

	fprintf(fff,"\t\tType ");
	for(i=0;i<MAX_OPEN_TYPE;i++) {
		fprintf(fff,"(%s %d/%d)",open_type_names[i],
				stats.open_type_success[i],
				stats.open_type_success[i]+
					stats.open_type_fail[i]);
	}
	fprintf(fff,"\n");

	fprintf(fff,"\tClose:\t%lld/%lld Successful\n",
	       stats.close_successful,stats.close_attempts);
	fprintf(fff,"\tRead:\t%lld/%lld Successful\n",
	       stats.read_successful,stats.read_attempts);
	fprintf(fff,"\tWrite:\t%lld/%lld Successful\n",
	       stats.write_successful,stats.write_attempts);
	fprintf(fff,"\tIoctl:\t%lld/%lld Successful: ",
	       stats.ioctl_successful,stats.ioctl_attempts);

	for(i=0;i<MAX_IOCTL_TYPE;i++) {
		fprintf(fff,"(%s %d/%d)",ioctl_type_names[i],
				stats.ioctl_type_success[i],
				stats.ioctl_type_success[i]+
					stats.ioctl_type_fail[i]);
	}
	fprintf(fff,"\n");

	fprintf(fff,"\tMmap:\t%lld/%lld Successful: ",
		stats.mmap_successful,stats.mmap_attempts);
		fprintf(fff,"(MMAP %lld/%lld)",
		stats.mmap_successful,stats.mmap_attempts);
		fprintf(fff,"(TRASH %lld/%lld)",
		stats.mmap_trash_successful,stats.mmap_trash_attempts);
		fprintf(fff,"(READ %lld/%lld)",
		stats.mmap_read_successful,stats.mmap_read_attempts);
		fprintf(fff,"(UNMAP %lld/%lld)",
		stats.mmap_unmap_successful,stats.mmap_unmap_attempts);
		fprintf(fff,"(AUX %lld/%lld)",
		stats.mmap_aux_successful,stats.mmap_aux_attempts);
		fprintf(fff,"(AUX_READ %lld/%lld)",
		stats.mmap_aux_read_successful,stats.mmap_aux_read_attempts);
		fprintf(fff,"\n");

	fprintf(fff,"\tPrctl:\t%lld/%lld Successful\n",
	       stats.prctl_successful,stats.prctl_attempts);
	fprintf(fff,"\tFork:\t%lld/%lld Successful\n",
	       stats.fork_successful,stats.fork_attempts);
	if (stats.fork_successful!=stats.fork_attempts) {
		fprintf(fff,"\t\tFORK FAILURES: ");
		for(i=0;i<MAX_ERRNOS;i++) {
			if (stats.fork_errno_count[i]!=0) {
				print_errno_name(fff,i);
				fprintf(fff," : %d ",stats.fork_errno_count[i]);
			}
		}
		fprintf(fff,"\n");
	}
	fprintf(fff,"\tPoll:\t%lld/%lld Successful\n",
	       stats.poll_successful,stats.poll_attempts);
	fprintf(fff,"\tAccess:\t%lld/%lld Successful\n",
	       stats.access_successful,stats.access_attempts);
	fprintf(fff,"\tOverflows: %lld  Recursive: %lld\n",
		stats.overflows, stats.already_overflows);
	fprintf(fff,"\tSIGIOs due to RT signal queue full: %lld\n",stats.sigios);

	}

	/* Reset counts back to zero */
	stats.total_syscalls=0;
	stats.open_attempts=0; stats.open_successful=0;
	stats.close_attempts=0; stats.close_successful=0;
	stats.read_attempts=0; stats.read_successful=0;
	stats.write_attempts=0; stats.write_successful=0;
	stats.ioctl_attempts=0; stats.ioctl_successful=0;
	stats.mmap_attempts=0; stats.mmap_successful=0;
	stats.mmap_trash_attempts=0; stats.mmap_trash_successful=0;
	stats.mmap_read_attempts=0; stats.mmap_read_successful=0;
	stats.mmap_unmap_attempts=0; stats.mmap_unmap_successful=0;
	stats.mmap_aux_attempts=0; stats.mmap_aux_successful=0;
	stats.mmap_aux_read_attempts=0; stats.mmap_aux_read_successful=0;
	stats.prctl_attempts=0; stats.prctl_successful=0;
	stats.fork_attempts=0; stats.fork_successful=0;
	stats.poll_attempts=0; stats.poll_successful=0;
	stats.access_attempts=0; stats.access_successful=0;
	stats.overflows=0;
	stats.already_overflows=0;
	stats.sigios=0;
	for(i=0;i<MAX_ERRNOS;i++) {
		stats.open_errno_count[i]=0;
		stats.fork_errno_count[i]=0;
	}
	for(i=0;i<MAX_OPEN_TYPE;i++) {
		stats.open_type_success[i]=0;
		stats.open_type_fail[i]=0;
	}
	for(i=0;i<MAX_OPEN_TRINITY_TYPE;i++) {
		stats.open_trinity_type_success[i]=0;
		stats.open_trinity_type_fail[i]=0;
	}
	for(i=0;i<MAX_IOCTL_TYPE;i++) {
		stats.ioctl_type_success[i]=0;
		stats.ioctl_type_fail[i]=0;
	}
}
