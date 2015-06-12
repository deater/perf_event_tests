#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <signal.h>

#include <sys/ioctl.h>

#include "../include/perf_event.h"

#include "perf_fuzzer.h"
#include "fuzzer_determinism.h"
#include "fuzzer_logging.h"
#include "fuzzer_random.h"
#include "fuzzer_stats.h"

#include "trace_filters.h"


static char filter[8192];

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

static void print_value(int type) {


	char temp[BUFSIZ];

	if (type==FILTER_TYPE_STR) {
		strcat(filter,"\"blah\"");
	}
	else {
		sprintf(temp,"%d",rand()%1024);
		strcat(filter,temp);

	}
}


int print_filter(struct trace_event_t *event,
		int max_levels, int max_whitespace) {


	int levels,l,field;

	if (event->num_filters==0) return -1;

	levels=rand()%max_levels;

	print_whitespace(max_whitespace);

	for(l=0;l<levels;l++) {
		strcat(filter,"(");
		print_whitespace(max_whitespace);
	}

	l=0;
	do {
		field=rand()%event->num_filters;
		strcat(filter,event->filter[field].name);

		print_whitespace(max_whitespace);

		print_comparison(event->filter[field].type);

		print_whitespace(max_whitespace);

		print_value(event->filter[field].type);

		print_whitespace(max_whitespace);

		if (levels==0) break;
		strcat(filter,")");

		print_whitespace(max_whitespace);

		if (l!=levels-1) strcat(filter,"&&");

		print_whitespace(max_whitespace);

		l++;
	} while(l<levels);

	return 0;

}


static int fill_filter(int which) {

	struct trace_event_t *event;

	filter[0]=0;

	event=find_event(which);
	if (event==NULL) return -1;

	print_filter(event, 4 /* MAX LEVELS */,
				1 /* MAX WHITESPACE */);

	return 0;
}



void ioctl_random_event(void) {

	long long arg,arg2;
	int result;
	unsigned int type;
	long long id;
	int any_event,sampling_event;
	int custom;

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

			fill_filter(event_data[any_event].attr.config);

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
			//	printf("FILTER FAILED %s\n",filter);
			}
			else {
//				printf("FILTER SUCCEDED %d %s\n",result,filter);
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
		/* FIXME: argument is bpf file descriptor */
		case 8:
			arg=rand();
			if (ignore_but_dont_skip.ioctl) return;
			result=ioctl(event_data[any_event].fd,
					PERF_EVENT_IOC_SET_BPF,&id);
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

