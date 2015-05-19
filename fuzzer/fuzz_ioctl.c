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

void ioctl_random_event(void) {

	int arg,arg2,result;
	long long arg64;
	unsigned int type;
	long long id;
	int any_event,sampling_event;

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
				sprintf(log_buffer,"I %d %d %d\n",
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
				sprintf(log_buffer,"I %d %d %d\n",
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
				sprintf(log_buffer,"I %d %d %d\n",
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
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[any_event].fd,
						PERF_EVENT_IOC_RESET,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOC_PERIOD */
		/* fd to adjust period as first argument */
		/* argument is a pointer to a 64-bit value (new period) */
		case 4:
			arg64=rand_period();
			if (ignore_but_dont_skip.ioctl) return;

			result=ioctl(event_data[sampling_event].fd,
					PERF_EVENT_IOC_PERIOD,&arg64);

			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %lld\n",
					event_data[sampling_event].fd,
					(long)PERF_EVENT_IOC_PERIOD,
					arg64);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;

		case 5: arg=event_data[find_random_active_event()].fd;
			if (ignore_but_dont_skip.ioctl) return;
			result=ioctl(event_data[any_event].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[any_event].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 6: arg=rand();
			if (ignore_but_dont_skip.ioctl) return;
			/* FIXME -- read filters from file */
			/* under debugfs tracing/events/ * / * /id */
			result=ioctl(event_data[any_event].fd,PERF_EVENT_IOC_SET_FILTER,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %d\n",
					event_data[any_event].fd,(long)PERF_EVENT_IOC_SET_FILTER,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOC_ID */
		case 7: arg=rand();
			if (ignore_but_dont_skip.ioctl) return;
			result=ioctl(event_data[any_event].fd,PERF_EVENT_IOC_ID,&id);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %lld\n",
					event_data[any_event].fd,(long)PERF_EVENT_IOC_ID,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		/* PERF_EVENT_IOC_SET_BPF */
		case 8: arg=rand();
			if (ignore_but_dont_skip.ioctl) return;
			result=ioctl(event_data[any_event].fd,PERF_EVENT_IOC_SET_BPF,&id);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %lld\n",
					event_data[any_event].fd,(long)PERF_EVENT_IOC_ID,id);
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
				sprintf(log_buffer,"I %d %d %d\n",
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

