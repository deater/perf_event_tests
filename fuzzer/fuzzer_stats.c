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
		default:	fprintf(fff,"UNKNOWN %d",e);
				break;
	}
}

void dump_summary(FILE *fff, int print_values) {

	int i;

	if (print_values) {

	fprintf(fff,"Iteration %lld\n",stats.total_iterations);
	fprintf(fff,"\tOpen attempts: %lld  Successful: %lld  Currently open: %lld\n",
	       stats.open_attempts,stats.open_successful,stats.current_open);
	for(i=0;i<MAX_ERRNOS;i++) {
		if (stats.errno_count[i]!=0) {
			fprintf(fff,"\t\t");
			print_errno_name(fff,i);
			fprintf(fff," : %d\n",stats.errno_count[i]);
		}
	}

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
	fprintf(fff,"\tIoctl:\t%lld/%lld Successful\n",
	       stats.ioctl_successful,stats.ioctl_attempts);
	fprintf(fff,"\tMmap:\t%lld/%lld Successful\n",
	       stats.mmap_successful,stats.mmap_attempts);
	fprintf(fff,"\tPrctl:\t%lld/%lld Successful\n",
	       stats.prctl_successful,stats.prctl_attempts);
	fprintf(fff,"\tFork:\t%lld/%lld Successful\n",
	       stats.fork_successful,stats.fork_attempts);
	fprintf(fff,"\tPoll:\t%lld/%lld Successful\n",
	       stats.poll_successful,stats.poll_attempts);
	fprintf(fff,"\tAccess:\t%lld/%lld Successful\n",
	       stats.access_successful,stats.access_attempts);
	fprintf(fff,"\tTrash mmap:\t%lld/%lld Successful\n",
		stats.trash_mmap_successful,stats.trash_mmap_attempts);
	fprintf(fff,"\tOverflows: %lld\n", stats.overflows);
	fprintf(fff,"\tSIGIOs due to RT signal queue full: %lld\n",stats.sigios);

	}

	/* Reset counts back to zero */
	stats.open_attempts=0; stats.open_successful=0;
	stats.close_attempts=0; stats.close_successful=0;
	stats.read_attempts=0; stats.read_successful=0;
	stats.write_attempts=0; stats.write_successful=0;
	stats.ioctl_attempts=0; stats.ioctl_successful=0;
	stats.mmap_attempts=0; stats.mmap_successful=0;
	stats.prctl_attempts=0; stats.prctl_successful=0;
	stats.fork_attempts=0; stats.fork_successful=0;
	stats.poll_attempts=0; stats.poll_successful=0;
	stats.access_attempts=0; stats.access_successful=0;
	stats.trash_mmap_attempts=0; stats.trash_mmap_successful=0;
	stats.overflows=0;
	stats.sigios=0;
	for(i=0;i<MAX_ERRNOS;i++) {
		stats.errno_count[i]=0;
	}
	for(i=0;i<MAX_OPEN_TYPE;i++) {
		stats.open_type_success[i]=0;
		stats.open_type_fail[i]=0;
	}
}
