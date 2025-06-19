#define _GNU_SOURCE 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

//#include <sys/mman.h>

#include <signal.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_stats.h"

#include "fuzz_mmap.h"
#include "fuzz_overflow.h"
#include "fuzz_open.h"
#include "fuzz_ioctl.h"

/* Trinity Includes */
#include "shm.h"
#include "sanitise.h"
#include "syscall.h"
#include "tables.h"
unsigned int get_cpu(void);
extern struct syscallentry syscall_perf_event_open;

#include "perf_helpers.h"


static void perf_log_attr(int fd, struct perf_event_attr *attr) {

	sprintf(log_buffer,"%x ",attr->type);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%x ",attr->size);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%llx ",attr->config);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%llx ",attr->sample_period);	/* union */
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%llx ",attr->sample_type);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%llx ",attr->read_format);
	write(fd,log_buffer,strlen(log_buffer));

	/* 64-bit bitfield */
	sprintf(log_buffer,"%d ",attr->disabled);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->inherit);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->pinned);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclusive);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_user);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_kernel);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_hv);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_idle);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->mmap);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->comm);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->freq);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->inherit_stat);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->enable_on_exec);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->task);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->watermark);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->precise_ip);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->mmap_data);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->sample_id_all);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_host);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_guest);
	write(fd,log_buffer,strlen(log_buffer));

	/****/
	sprintf(log_buffer,"%d ",attr->exclude_callchain_kernel);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->exclude_callchain_user);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->mmap2);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->comm_exec);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->use_clockid);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->context_switch);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->write_backward);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->namespaces);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->ksymbol);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->bpf_event);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->aux_output);
	write(fd,log_buffer,strlen(log_buffer));
/// new
	sprintf(log_buffer,"%d ",attr->cgroup);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->text_poke);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->build_id);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->inherit_thread);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->remove_on_exec);
	write(fd,log_buffer,strlen(log_buffer));
	sprintf(log_buffer,"%d ",attr->sigtrap);
	write(fd,log_buffer,strlen(log_buffer));
// end new

	sprintf(log_buffer,"%d ",attr->wakeup_events);	/* union */
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%d ",attr->bp_type);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%llx ",attr->config1);	/* union */
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%llx ",attr->config2);	/* union */
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%lld ",attr->branch_sample_type);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%lld ",attr->sample_regs_user);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%d ",attr->sample_stack_user);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%d ",attr->clockid);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%lld ",attr->sample_regs_intr);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%d ",attr->aux_watermark);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%hd ",attr->sample_max_stack);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"%d ",attr->aux_sample_size);
	write(fd,log_buffer,strlen(log_buffer));

	sprintf(log_buffer,"\n");
	write(fd,log_buffer,strlen(log_buffer));
	sync();

}

static int update_read_size(int i) {

	int read_size=1;

	/* Set nominal read size */
	if (event_data[i].attr.read_format&PERF_FORMAT_GROUP) {
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_ENABLED) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_RUNNING) read_size++;
		read_size+=(1+ (
			!!(event_data[i].attr.read_format&PERF_FORMAT_ID) +
			!!(event_data[i].attr.read_format&PERF_FORMAT_LOST)
			) * event_data[i].number_in_group);
	}
	else {
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_ENABLED) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_TOTAL_TIME_RUNNING) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_ID) read_size++;
		if (event_data[i].attr.read_format&PERF_FORMAT_LOST) read_size++;
	}

	return read_size*sizeof(long long);
}

void post_perf_event_open(struct syscallrecord *rec);


static int known_errno(int e) {

	switch(e) {
		case EPERM:
		case ENOENT:
		case E2BIG:
		case EBADF:
		case EINVAL:
		case EOPNOTSUPP:
		case ENODEV:
		case ENOSPC:
		case EMFILE:
		case EACCES:
		case EBUSY:
		case EAGAIN:
		case EOVERFLOW:
		case EFAULT:
			return 1;
                default:
			return 0;
	}
}

void open_random_event(int mmap_enabled, int overflow_enabled) {

	int fd;

	int i,trinity_type;

	i=find_empty_event();

	/* return if no free events */
	if (i<0) return;

	event_data[i].overflows=0;
	event_data[i].throttles=0;


	/* repeat until we create a valid event */
	while(1) {
		/* call trinity random perf_event_open() code */
		//generic_sanitise(0);
		trinity_type=syscall_perf_event_open.sanitise(&shm->syscall[0]);

		memcpy(&event_data[i].attr,
			(struct perf_event_attr *)shm->syscall[0].a1,
			sizeof(struct perf_event_attr));
		event_data[i].pid=shm->syscall[0].a2;
		event_data[i].cpu=get_cpu();
		event_data[i].group_fd=shm->syscall[0].a4;
		event_data[i].flags=shm->syscall[0].a5;

		post_perf_event_open(&shm->syscall[0]);

		/* Randomly make part of a group 1/4 of the time */
		if (rand()%4==2) {
			int j;
			j=find_random_active_event();

			/* is it a master? */
			/* can we set a group leader that isn't itself */
			/* a leader? */
//			if (event_data[j].group_fd==-1) {
			event_data[i].group_fd=event_data[j].fd;
//			}

		}

		/* Randomly try to use a kprobe */
		if (event_data[i].attr.type==PERF_TYPE_TRACEPOINT) {
			if (rand()%10==5) {
				event_data[i].attr.config=kprobe_id;
			}
		}


		if (ignore_but_dont_skip.open) return;

		/* Debugging code */
		/* We don't usually log failed opens as there are so many */

		if (logging&TYPE_OPEN) {
#if LOG_FAILURES
			if (trigger_failure_logging) {
				/* uncomment if failing opens causing crashes */
//				static int quit_next=0;
//				if (event_data[i].attr.type==PERF_TYPE_TRACEPOINT) {
				sprintf(log_buffer,"# O -1 %d %d %d %lx ",
					event_data[i].pid,
					event_data[i].cpu,
					event_data[i].group_fd,
					event_data[i].flags);
				write(log_fd,log_buffer,strlen(log_buffer));
	                	perf_log_attr(log_fd,&event_data[i].attr);
//				fsync(log_fd);
//				}
//				if (quit_next==1) exit(1);

//				if (quit_next) quit_next--;

//				if ((event_data[i].attr.read_format==0x2d2d2d))
//				quit_next=2;

			}
#endif
	        }

		/* Actually try to open the event */
		fd=perf_event_open(
			&event_data[i].attr,
			event_data[i].pid,
			event_data[i].cpu,
			event_data[i].group_fd,
			event_data[i].flags);
		stats.open_attempts++;
		stats.total_syscalls++;

		int which_type=event_data[i].attr.type;

		if ((which_type<0) || (which_type>MAX_OPEN_TYPE-1)) {
			which_type=MAX_OPEN_TYPE-1;
		}

		/* If we succede, break out of the infinite loop */
		if (fd>2) {
			stats.open_type_success[which_type]++;
			stats.open_trinity_type_success[trinity_type]++;
			break;
		}

		/* This shouldn't be possible, but has happened */
		if ((fd==0) || (fd==1) || (fd==2)) {
			printf("PERF_EVENT_BAD_RETURN fd=%d\n",fd);
			sprintf(log_buffer,"# O -1 %d %d %d %lx ",
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
			write(fileno(stdout),log_buffer,strlen(log_buffer));
                	perf_log_attr(fileno(stdout),&event_data[i].attr);
			exit(1);
		}


		/* Otherwise, track the errors */
		if (errno<MAX_ERRNOS) {
			stats.open_errno_count[errno]++;
			stats.open_type_fail[which_type]++;
			stats.open_trinity_type_fail[trinity_type]++;
		}
		else {
			/* TODO */
			/* print message? */
		}

		/* Track source of UNKNOWN errnos */
		if (!known_errno(errno)) {
			printf("UNKNOWN ERROR %d\n",errno);
			sprintf(log_buffer,"# O -1 %d %d %d %lx ",
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
			write(fileno(stdout),log_buffer,strlen(log_buffer));
                	perf_log_attr(fileno(stdout),&event_data[i].attr);
			exit(1);
		}

		/* no more file descriptors, so give up */
		if (errno==EMFILE) return;

	}

	/* We successfully opened an event! */

	stats.open_successful++;
	stats.current_open++;

	if (logging&TYPE_OPEN) {
		sprintf(log_buffer,"O %d %d %d %d %lx ",
				fd,
				event_data[i].pid,
				event_data[i].cpu,
				event_data[i].group_fd,
				event_data[i].flags);
		write(log_fd,log_buffer,strlen(log_buffer));
		perf_log_attr(log_fd,&event_data[i].attr);
#if FSYNC_EVERY
		fsync(log_fd);
#endif
	}

	event_data[i].fd=fd;
	event_data[i].active=1;
	active_events++;

	/* if we are member of a group, update size of group */
	/* this is needed for calcuating "proper" read size  */
	/* Also I don't think we adjust this on close        */
	if (event_data[i].group_fd!=-1) {
		int j=lookup_event(event_data[i].group_fd);

		event_data[j].number_in_group++;
		event_data[j].read_size=update_read_size(j);
	}

	/* Setup mmap buffer */

	if (mmap_enabled) {
		setup_mmap(i);
	}

	/* Setup overflow 50% of the time */
	if ((overflow_enabled) && (rand()%2)) {

	if (!ignore_but_dont_skip.overflow) {

		int fcntl_result;

		if (logging&TYPE_OVERFLOW) {
			sprintf(log_buffer,"o %d\n",event_data[i].fd);
			write(log_fd,log_buffer,strlen(log_buffer));
		}

		memset(&event_data[i].sa, 0, sizeof(struct sigaction));
		event_data[i].sa.sa_sigaction = our_handler;
		event_data[i].sa.sa_flags = SA_SIGINFO;

		if (sigaction( SIGRTMIN+2, &event_data[i].sa, NULL) < 0) {
			printf("Error setting up signal handler\n");
     		}

		fcntl_result=fcntl(event_data[i].fd, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
		if (fcntl_result<0) fprintf(stderr,"F1 error!\n");
		fcntl_result=fcntl(event_data[i].fd, F_SETSIG, SIGRTMIN+2);
		if (fcntl_result<0) fprintf(stderr,"F1 error!\n");
		fcntl_result=fcntl(event_data[i].fd, F_SETOWN,getpid());
		if (fcntl_result<0) fprintf(stderr,"F1 error!\n");
	}

	}

	event_data[i].number_in_group=1;

	event_data[i].read_size=update_read_size(i);

}

