#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <signal.h>

#include <errno.h>

#include <sys/ioctl.h>

#include "../include/perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_random.h"
#include "fuzzer_stats.h"

#include "trace_filters.h"

#include "bpf.h"
#include "libbpf.h"
#include <sys/utsname.h>
#include <asm/unistd.h>


long sys_bpf(int cmd, union bpf_attr *attr, unsigned long size) {

	return syscall(__NR_bpf, cmd, attr, size);
}

#define MAX_FILTER_SIZE 8192

static char filter[MAX_FILTER_SIZE];

static struct trace_event_t *find_event(int event) {

	int i;

	for(i=0;i<TRACE_EVENTS;i++) {

		if (trace_events[i].id==event) return &trace_events[i];
	}
	return NULL;

}

#if 0
        { OP_OR,        "||",           1 },
        { OP_AND,       "&&",           2 },
        { OP_GLOB,      "~",            4 },
        { OP_NE,        "!=",           4 },
        { OP_EQ,        "==",           4 },
        { OP_LT,        "<",            5 },
        { OP_LE,        "<=",           5 },
        { OP_GT,        ">",            5 },
        { OP_GE,        ">=",           5 },
        { OP_BAND,      "&",            6 },
        { OP_NOT,       "!",            6 },
        { OP_NONE,      "OP_NONE",      0 },
        { OP_OPEN_PAREN, "(",           0 },

8b3725621074040d380664964ffbc40610aef8c6

numeric fields:
    ==, !=, <, <=, >, >=
    string fields:
    ==, !=
    predicates can be combined with the logical operators:
    &&, ||
    examples:
    "common_preempt_count > 4" > filter
    "((sig >= 10 && sig < 15) || sig == 17) && comm != bash" > filter

    To clear a filter, '0' can be written to the filter file.
#endif


/*
	Create a filter

		Levels

		rand if not
		rand if paren
		rand identifier
		rand operation
		rand value
*/


static void print_whitespace(int max_amount) {

	int i,amount;

	if (max_amount==0) max_amount=1;

	amount=rand()%max_amount;

	for(i=0;i<amount;i++) {
		switch(rand()%2) {
			case 0:	strcat(filter," ");
				break;
			case 1:	strcat(filter,"\t");
				break;
		}
	}
}

static void print_comparison(int type) {

	switch(rand()%8) {
		case 0: strcat(filter,"~"); break;
		case 1: strcat(filter,"!="); break;
		case 2: strcat(filter,"=="); break;
		case 3: strcat(filter,"<"); break;
		case 4: strcat(filter,"<="); break;
		case 5: strcat(filter,">"); break;
		case 6: strcat(filter,">="); break;
		case 7: strcat(filter,"&"); break;
	}
}

static void print_bool(void) {

	switch(rand()%2) {
		case 0: strcat(filter,"&&"); break;
		case 1: strcat(filter,"||"); break;
	}
}

static void print_value(int type, int try_valid) {

	int i;

	char temp[BUFSIZ];
	char string[BUFSIZ];
	int length,offset;

	if ((type==FILTER_TYPE_STR) || (!try_valid && rand()%5==1)) {

		string[0]=0;

		switch(rand()%15)  {

			case 0: strcat(string,"\"");
				break;

			case 1:
			case 2:
			case 3:
				strcat(string,"\"");
				length=rand()%256;
				for(i=0;i<length;i++) {
					string[i+1]=(rand()%254)+1;
				}
				string[i]='\"';
				string[i+1]=0;
				break;

			default: sprintf(string,"\"blah\"");
				break;
		}

		length=strlen(string);
		switch(rand()%10) {
			case 0:	if (length!=0) string[rand()%length]='*';
				break;
			case 1:	if (length!=0) {
					offset=rand()%length;
					string[offset]='*';
					string[offset+1]='\"';
					string[offset+2]=0;
				}
				break;
		}
		strcat(filter,string);

	}
	else {
		switch(rand()%10) {
			case 0: sprintf(temp,"%d",rand());
				break;
			case 1: sprintf(temp,"%d%d",rand(),rand());
				break;
			case 2: sprintf(temp,"0x%x",rand());
				break;
			case 3: sprintf(temp,"-%d",rand());
				break;
			default: sprintf(temp,"%d",rand()%1024);
				break;
		}
		strcat(filter,temp);

	}
}


static void print_expression(struct trace_event_t *event,
			int max_whitespace,int try_valid) {

	int field;

	if (event->num_filters==0) return;

	field=rand()%event->num_filters;

	if (try_valid) {
		strcat(filter,event->filter[field].name);
	}
	else {
		switch(rand()%3) {
			case 0: strcat(filter,event->filter[field].name);
				break;
			case 1: print_value(rand()%2,try_valid);
				break;
			case 2:
				break;
		}

	}


	print_whitespace(max_whitespace);

	if (try_valid) {
		print_comparison(event->filter[field].type);
	}
	else {
		if (rand()%3==1) print_comparison(event->filter[field].type);
	}

	print_whitespace(max_whitespace);

	if (try_valid) {
		print_value(event->filter[field].type,try_valid);
	}
	else {
		switch(rand()%3) {
			case 0: strcat(filter,event->filter[field].name);
				break;
			case 1: print_value(rand()%2,try_valid);
				break;
			case 2:
				break;
		}
	}

	print_whitespace(max_whitespace);

}

int make_filter(struct trace_event_t *event,
		int max_levels, int max_whitespace,
		int try_valid) {


	int levels,l;

	if (event->num_filters==0) return -1;

	levels=rand()%(max_levels+1);

	print_whitespace(max_whitespace);

	for(l=0;l<levels;l++) {
		if (try_valid) {
			strcat(filter,"(");
		}
		else {
			if (rand()%3==1) strcat(filter,"(");
		}
		print_whitespace(max_whitespace);
		if (rand()%5==1) strcat(filter,"!");
	}

	l=0;
	do {

		print_expression(event,max_whitespace,try_valid);

		if (levels==0) break;

		if (try_valid) {
			strcat(filter,")");
		}
		else {
			if (rand()%3==1) strcat(filter,")");
		}
		print_whitespace(max_whitespace);

		if (l!=levels-1) {
			if (try_valid) {
				print_bool();
			}
			else {
				if (rand()%3==1) {
					print_bool();
				}
			}
		}

		print_whitespace(max_whitespace);

		l++;
	} while(l<levels);

	return 0;

}

int make_crazy_filter(void) {

	int length=rand()%(MAX_FILTER_SIZE-1);
	int i=0;

	filter[0]=0;

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

	struct trace_event_t *event;

	filter[0]=0;

	event=find_event(which);
	if (event==NULL) return -1;

	switch(rand()%30) {
		case 0:	make_crazy_filter();
			break;

		case 1: 
		case 2: make_filter(event, rand()%20 /* MAX LEVELS */,
				rand()%20 /* MAX WHITESPACE */,
				0 /* TRY_VALID */);
			break;

		default: make_filter(event, 4 /* MAX LEVELS */,
				1 /* MAX WHITESPACE */,
				1 /* TRY_VALID */);
			break;
	}
	return 0;
}

static char *tracepoint_name(int which) {

	struct trace_event_t *event;

	event=find_event(which);
	if (event==NULL) return NULL;

	return event->name;
}


static int kprobe_initialized=0;

int kprobe_id=0;



static int setup_bpf_fd(void) {

	FILE *fff;
	static int bpf_fd=0;
	union bpf_attr battr;

	if (!kprobe_initialized) {

		fff=fopen("/sys/kernel/tracing/kprobe_events", "w");
		if (fff==NULL) {
			printf("Cannot create kprobe!\n");
			return -1;
		}

		fprintf(fff,"p:probe/VMW _text+1664816");
		fclose(fff);

		fff=fopen("/sys/kernel/tracing/events/probe/VMW/id","r");
		if (fff==NULL) {
			return -1;
		}

		fscanf(fff,"%d",&kprobe_id);
		fclose(fff);

		kprobe_initialized=1;

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
	int any_event,sampling_event;
	int custom;
	int which_tracepoint;
	int bpf_fd;

	any_event=find_random_active_event();
	sampling_event=find_random_active_sampling_event();

	/* If no sampling event, use any event instead */
	if (sampling_event<0) sampling_event=any_event;

	/* Somtimes use any event instead of a sampling event */
	if (rand()%4==0) sampling_event=any_event;

	/* Exit if no events */
	if (any_event<0) return;

	type=rand()%10;

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
				sprintf(log_buffer,"I %d %d %lld\n",
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
				sprintf(log_buffer,"I %d %d %lld\n",
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
				sprintf(log_buffer,"I %d %d %lld\n",
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
				sprintf(log_buffer,"I %d %d %lld\n",
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
				sprintf(log_buffer,"I %d %ld %lld\n",
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
				sprintf(log_buffer,"I %d %d %lld\n",
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
				sprintf(log_buffer,"I %d %ld %d %lld %s\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_SET_FILTER,
					custom,arg,filter);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

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


			break;

		/* PERF_EVENT_IOC_ID */
		/* Read the event id into a 64-bit value */
		case 7: arg=rand();
			if (ignore_but_dont_skip.ioctl) return;
			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_ID,&id);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %lld\n",
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
				sprintf(log_buffer,"I %d %ld %lld\n",
					event_data[any_event].fd,
					(long)PERF_EVENT_IOC_ID,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		/* Random */
		default:
			arg=rand(); arg2=rand();
			type=arg;
			if (ignore_but_dont_skip.ioctl) return;
			result=ioctl(event_data[any_event].fd,arg,arg2);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %lld %lld\n",
					event_data[any_event].fd,arg,arg2);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;
	}

	stats.ioctl_attempts++;

	if (result>=0) {
		stats.ioctl_successful++;
		stats.ioctl_type_success[ (type>MAX_IOCTL_TYPE-1)?
			MAX_IOCTL_TYPE-1:type]++;
	} else {
		stats.ioctl_type_fail[ (type>MAX_IOCTL_TYPE-1)?
			MAX_IOCTL_TYPE-1:type]++;
	}
}

