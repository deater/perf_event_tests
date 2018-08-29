#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <signal.h>

#include <errno.h>

#include <sys/ioctl.h>

#include "perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_random.h"
#include "fuzzer_stats.h"

#include "filter_address.h"
#include "filter_tracepoint.h"

#include "bpf.h"
#include "libbpf.h"
#include <sys/utsname.h>
#include <asm/unistd.h>

#include "bpf_helpers.h"

#define MAX_FILTER_SIZE 8192
static char filter[MAX_FILTER_SIZE];

/* Use as a valid address for random calls that could use one */
static char valid_ram[4096];


/* From asm-generic/ioctl.h */
#define _IOC_NRSHIFT    0
#define _IOC_TYPESHIFT  (_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT  (_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT   (_IOC_SIZESHIFT+_IOC_SIZEBITS)

static long create_ioctl(int dir, int type, int nr, int size) {

	long number;

	number=(((dir)  << _IOC_DIRSHIFT) |
		((type) << _IOC_TYPESHIFT) |
		((nr)   << _IOC_NRSHIFT) |
		((size) << _IOC_SIZESHIFT));

	return number;
}

static int make_crazy_filter(void) {

	int length=rand()%(MAX_FILTER_SIZE-1);
	int i=0;

//	filter[0]=0;

	char random_mess[]="()&=!><~|";

	while(i<length) {

		switch(rand()%3) {
			case 0: filter[i]=random_mess[rand()%9];
				break;
			case 1: filter[i]=(rand()%254)+1;
				break;
			case 2: filter[i]=(rand()%10)+'0';
				break;

		}
		i++;
	}

	filter[i]=0;

	return 0;
}

static int fill_filter(int which) {

	filter[0]=0;

	switch(rand()%30) {
		case 0:	make_crazy_filter();
			break;

		case 1:
		case 2: make_tracepoint_filter(which,
				filter, MAX_FILTER_SIZE,
				rand()%20 /* MAX LEVELS */,
				rand()%20 /* MAX WHITESPACE */,
				0 /* TRY_VALID */);
			break;

		case 3:
		case 4:
			make_address_filter(which,filter,
				MAX_FILTER_SIZE,
				rand()%20, /* MAX WHITESPACE */
				0 /* TRY_VALID */);
			break;

		case 10 ... 20:
			make_address_filter(which,filter,
				MAX_FILTER_SIZE,
				1, /* MAX WHITESPACE */
				1  /* TRY_VALID */);
			break;

		default: make_tracepoint_filter(which,
				filter, MAX_FILTER_SIZE,
				4, /* MAX LEVELS */
				1, /* MAX WHITESPACE */
				1  /* TRY_VALID */);
			break;
	}
	return 0;
}

static int kprobe_initialized=0;
int kprobe_id=0;

static int setup_bpf_fd(void) {

	FILE *fff;
	static int bpf_fd=0;
	union bpf_attr battr;

	if (!kprobe_initialized) {

		kprobe_initialized=1;

		fff=fopen("/sys/kernel/tracing/kprobe_events", "w");
		if (fff==NULL) {
			printf("Cannot open /sys/kernel/tracing/kprobe_events\n");
			return -1;
		}

		/* perf probe -a VMW=handle_mm_fault */
		fprintf(fff,"p:probe/VMW _text+1664624");
		fclose(fff);

		fff=fopen("/sys/kernel/tracing/events/probe/VMW/id","r");
		if (fff==NULL) {
			printf("Cannot open /sys/kernel/tracing/events/probe/VMW/id\n");
			return -1;
		}

		fscanf(fff,"%d",&kprobe_id);
		fclose(fff);

		struct bpf_insn instructions[] = {
			BPF_MOV64_IMM(BPF_REG_0, 0),	/* r0 = 0 */
			BPF_EXIT_INSN(),		/* return r0 */
		};

		unsigned char license[]="GPL";

		#define LOG_BUF_SIZE 65536
		static char bpf_log_buf[LOG_BUF_SIZE];

		/* Kernel will EINVAL if unused bits aren't zero */
		memset(&battr,0,sizeof(battr));

		/* Version has to match currently running kernel */

		struct utsname version;
		int major, minor, subminor, version_code;

		uname(&version);

		sscanf(version.release,"%d.%d.%d",&major,&minor,&subminor);

		version_code = (major<<16) | (minor<<8) | subminor;

		battr.prog_type = BPF_PROG_TYPE_KPROBE;
		battr.insn_cnt= sizeof(instructions) / sizeof(struct bpf_insn);
		battr.insns = (uint64_t) (unsigned long) instructions;
		battr.license = (uint64_t) (unsigned long) license;
		battr.log_buf = (uint64_t) (unsigned long) bpf_log_buf;
		battr.log_size = LOG_BUF_SIZE;
		battr.log_level = 1;
		battr.kern_version = version_code;

		bpf_log_buf[0] = 0;

		bpf_fd = sys_bpf(BPF_PROG_LOAD, &battr, sizeof(battr));
		if (bpf_fd < 0) {
			printf("bpf: load: failed to load program, %s\n"
				"-- BEGIN DUMP LOG ---\n%s\n-- END LOG --\n",
				strerror(errno), bpf_log_buf);
		}

        }

	return bpf_fd;
}


void ioctl_random_event(void) {

	long long arg,arg2;
	int result;
	unsigned int type;
	long long id;
	int any_event,sampling_event,breakpoint_event;
	int custom;
	int which_tracepoint;
	int bpf_fd;

	any_event=find_random_active_event();
	sampling_event=find_random_active_sampling_event();
	breakpoint_event=find_random_active_breakpoint_event();


	/* If no sampling event, use any event instead */
	if (sampling_event<0) sampling_event=any_event;

	/* Somtimes use any event instead of a sampling event */
	if (rand()%4==0) sampling_event=any_event;

	/* Exit if no events */
	if (any_event<0) return;

	type=rand()%14;

	switch(type) {

		/* PERF_EVENT_IOC_ENABLE */
		/* fd to enable as first argument */
		/* if PERF_IOC_FLAG_GROUP set in second argument */
		/* then all in group are enabled */
		case 0:
			arg=rand_ioctl_arg();
			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_ENABLE,arg);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %d %lld\n",
					event_data[any_event].fd,
					PERF_EVENT_IOC_ENABLE,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOC_DISABLE */
		/* fd to disable as first argument */
		/* if PERF_IOC_FLAG_GROUP set in second argument */
		/* then all in group are disabled */
		case 1:
			arg=rand_ioctl_arg();
			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_DISABLE,arg);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %d %lld\n",
					event_data[any_event].fd,
					PERF_EVENT_IOC_DISABLE,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOC_REFRESH */
		/* fd to refresh as first argument */
		/* argument adds to the refresh count */
		case 2:
			arg=rand_refresh();
			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[sampling_event].fd,
					PERF_EVENT_IOC_REFRESH,arg);

			if (result>0) {
				event_data[sampling_event].last_refresh=arg;
			}

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %d %lld\n",
					event_data[sampling_event].fd,
					PERF_EVENT_IOC_REFRESH,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		/* PERF_EVENT_IOC_RESET */
		/* fd to reset as first argument */
		/* if PERF_IOC_FLAG_GROUP set in second argument */
		/* then all in group are reset */
		case 3:
			arg=rand_ioctl_arg();
			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_RESET,arg);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %d %lld\n",
					event_data[any_event].fd,
						PERF_EVENT_IOC_RESET,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOC_PERIOD */
		/* fd to adjust period as first argument */
		/* argument is a pointer to a 64-bit value (new period) */
		case 4:
			arg=rand_period();
			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[sampling_event].fd,
					PERF_EVENT_IOC_PERIOD,&arg);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %ld %lld\n",
					event_data[sampling_event].fd,
					(long)PERF_EVENT_IOC_PERIOD,
					arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		/* PERF_EVENT_IOC_SET_OUTPUT */
		/* specifies an alternate fd to send output notifications */
		case 5:
			arg=event_data[find_random_active_event()].fd;
			if (rand()%10==0) arg=-1;

			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[sampling_event].fd,
					PERF_EVENT_IOC_SET_OUTPUT,arg);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %d %lld\n",
					event_data[sampling_event].fd,
					PERF_EVENT_IOC_SET_OUTPUT,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOCTL_SET_FILTER */
		/* Argument is a pointer to a filter? FIXME*/
		/* FIXME: preferentially find ftrace events */
		case 6:

			custom=rand()%2;

			if (custom) arg=rand();
			else arg=(long)&filter;

			which_tracepoint=event_data[any_event].attr.config;

			fill_filter(which_tracepoint);

			if (ignore_but_dont_skip.ioctl) return;
			/* FIXME -- read filters from file */
			/* under debugfs tracing/events/ * / * /id */
			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_SET_FILTER,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %ld %d %lld %s\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_SET_FILTER,
					custom,arg,filter);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

#if DEBUG_FILTER
			if (result<0) {
				if (filter[0]!=0) {
					printf("FILTER FAILED %s %s %s\n",
						tracepoint_name(which_tracepoint),
						filter,strerror(errno));
				}
			}
			else {
				printf("FILTER SUCCEDED %s %s\n",
					tracepoint_name(which_tracepoint),
					filter);
			}

#endif
			break;

		/* PERF_EVENT_IOC_ID */
		/* Read the event id into a 64-bit value */
		case 7: arg=rand();
			if (ignore_but_dont_skip.ioctl) return;
			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_ID,&id);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %ld %lld\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_ID,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOC_SET_BPF */
		/* argument is bpf file descriptor */
		case 8:
			arg=rand();
			if (ignore_but_dont_skip.ioctl) return;
			bpf_fd=setup_bpf_fd();

			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_SET_BPF,bpf_fd);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %ld %lld\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_SET_BPF,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		/* PERF_EVENT_IOC_PAUSE_OUTPUT */
		/* argument is to pause/unpause */
		case 9:
			arg=rand()%3;
			if (arg==2) arg=rand();

			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_PAUSE_OUTPUT,arg);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %ld %lld\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_PAUSE_OUTPUT,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		/* PERF_EVENT_IOC_QUERY_BPF */
		/* argument is struct perf_event_query_bpf * */
		case 10:
			arg=rand()%3;
			if (arg==2) arg=rand();

			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_QUERY_BPF,arg);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %ld %lld\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_QUERY_BPF,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		/* PERF_EVENT_IOC_MODIFY_ATTRIBUTES */
		/* argument is struct perf_event_attr */
		/* Currently only works on BREAKPOINT events */
		case 11:

			if (ignore_but_dont_skip.ioctl) return;

			custom=rand()%3;

			/* Operate on breakpoint, try to replace with
				and type of event */
			if (custom==0) {
				result=ioctl(event_data[breakpoint_event].fd,
					PERF_EVENT_IOC_MODIFY_ATTRIBUTES,
					&event_data[any_event].attr);

			}

			/* Operate on breakpoint, try to replace with
				other breakpoint*/
			if (custom==1) {
				custom=find_random_active_breakpoint_event();
				result=ioctl(event_data[breakpoint_event].fd,
					PERF_EVENT_IOC_MODIFY_ATTRIBUTES,
					&event_data[custom].attr);

			}

			// 1 in 3, make it totally random */
			if (custom==2) {
				arg=rand();
				result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_MODIFY_ATTRIBUTES,
					arg);
			}


			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %ld %lld\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_MODIFY_ATTRIBUTES,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;


		/* Random perf ($) ioctl */
		case 12:
			/* dir = none, r, w, rw */
			/* type = $ for perf */
			/* nr = number */
			/* size = size of argument? */

			/* realisitic */
			if (rand()%2) {
				arg=create_ioctl(rand()%3,'$',
					rand()%64,rand()%16);
			}
			/* unrealisitic */
			else {
				arg=create_ioctl(rand()%3,'$',rand(),rand());
			}

			if (rand()%2) {
				arg2=rand();
				valid_ram[rand()%4096]=rand();
			}
			else {
				arg2=(long)(&valid_ram);
			}

			type=arg;

			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[any_event].fd,arg,arg2);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %lld %lld\n",
					event_data[any_event].fd,arg,arg2);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		/* Random any ioctl */
		default:
			arg=create_ioctl(rand(),rand(),rand(),rand());
			arg2=rand();
			type=arg;

			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[any_event].fd,arg,arg2);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				snprintf(log_buffer,BUFSIZ,"I %d %lld %lld\n",
					event_data[any_event].fd,arg,arg2);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;
	}

	stats.ioctl_attempts++;
	stats.total_syscalls++;

	if (result>=0) {
		stats.ioctl_successful++;
		stats.ioctl_type_success[ (type>MAX_IOCTL_TYPE-1)?
			MAX_IOCTL_TYPE-1:type]++;
	} else {
		stats.ioctl_type_fail[ (type>MAX_IOCTL_TYPE-1)?
			MAX_IOCTL_TYPE-1:type]++;
	}
}

#if 0

/* test filters */
int main(int argc, char **argv) {

	int i;

	for(i=0;i<100;i++) {
		fill_filter(rand()%1024);
		printf("%d: %s\n",i,filter);
	}

	return 0;
}
#endif


