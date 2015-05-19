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

	fprintf(fff,"\tClose attempts: %lld  Successful: %lld\n",
	       stats.close_attempts,stats.close_successful);
	fprintf(fff,"\tRead attempts: %lld  Successful: %lld\n",
	       stats.read_attempts,stats.read_successful);
	fprintf(fff,"\tWrite attempts: %lld  Successful: %lld\n",
	       stats.write_attempts,stats.writes_successful);
	fprintf(fff,"\tIoctl attempts: %lld  Successful: %lld\n",
	       stats.ioctl_attempts,stats.ioctl_successful);
	fprintf(fff,"\tMmap attempts: %lld  Successful: %lld\n",
	       stats.mmap_attempts,stats.mmap_successful);
	fprintf(fff,"\tPrctl attempts: %lld  Successful: %lld\n",
	       stats.prctl_attempts,stats.prctl_successful);
	fprintf(fff,"\tFork attempts: %lld  Successful: %lld\n",
	       stats.fork_attempts,stats.fork_successful);
	fprintf(fff,"\tPoll attempts: %lld  Successful: %lld\n",
	       stats.poll_attempts,stats.poll_successful);
	fprintf(fff,"\tAccess attempts: %lld  Successful: %lld\n",
	       stats.access_attempts,stats.access_successful);
	fprintf(fff,"\tTrash mmap attempts: %lld  Successful: %lld\n",
		stats.trash_mmap_attempts,stats.trash_mmap_successful);
	fprintf(fff,"\tOverflows: %lld\n", stats.overflows);
	fprintf(fff,"\tSIGIOs due to RT signal queue full: %lld\n",stats.sigios);

	}

	/* Reset counts back to zero */
	stats.open_attempts=0; stats.open_successful=0;
	stats.close_attempts=0; stats.close_successful=0;
	stats.read_attempts=0; stats.read_successful=0;
	stats.write_attempts=0; stats.writes_successful=0;
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
