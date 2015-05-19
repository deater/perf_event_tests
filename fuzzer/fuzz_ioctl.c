#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include "perf_fuzzer.h"
#include "fuzzer_random.h"
#include "fuzzer_stats.h"

#include "../include/perf_event.h"


void ioctl_random_event(void) {

	int i,arg,arg2,result;
	long long id;

	i=find_random_active_event();

	/* Exit if no events */
	if (i<0) return;

	switch(rand()%9) {
		case 0:
			arg=rand_ioctl_arg();
#if 0
			if (ignore_but_dont_skip_ioctl) return;

			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_ENABLE,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_ENABLE,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
#endif
			break;
#if 0
		case 1:
			arg=rand_ioctl_arg();
			if (ignore_but_dont_skip_ioctl) return;
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_DISABLE,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_DISABLE,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 2:
			arg=rand_refresh();
			if (ignore_but_dont_skip_ioctl) return;
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_REFRESH,arg);
			if (result>0) {
				event_data[i].last_refresh=arg;
			}
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_REFRESH,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 3:
			arg=rand_ioctl_arg();
			if (ignore_but_dont_skip_ioctl) return;
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_RESET,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_RESET,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 4: arg=rand_period();
			if (ignore_but_dont_skip_ioctl) return;
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_PERIOD,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %d\n",
					event_data[i].fd,(long)PERF_EVENT_IOC_PERIOD,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 5: arg=event_data[find_random_active_event()].fd;
			if (ignore_but_dont_skip_ioctl) return;
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,PERF_EVENT_IOC_SET_OUTPUT,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 6: arg=rand();
			if (ignore_but_dont_skip_ioctl) return;
			/* FIXME -- read filters from file */
			/* under debugfs tracing/events/ * / * /id */
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_SET_FILTER,arg);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %d\n",
					event_data[i].fd,(long)PERF_EVENT_IOC_SET_FILTER,arg);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;
		case 7: arg=rand();
			if (ignore_but_dont_skip_ioctl) return;
			result=ioctl(event_data[i].fd,PERF_EVENT_IOC_ID,&id);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %ld %lld\n",
					event_data[i].fd,(long)PERF_EVENT_IOC_ID,id);
				write(log_fd,log_buffer,strlen(log_buffer));
			}
			break;

		default:
			arg=rand(); arg2=rand();
			if (ignore_but_dont_skip_ioctl) return;
			result=ioctl(event_data[i].fd,arg,arg2);
			if ((result>=0)&&(logging&TYPE_IOCTL)) {
				sprintf(log_buffer,"I %d %d %d\n",
					event_data[i].fd,arg,arg2);
				write(log_fd,log_buffer,strlen(log_buffer));
			}

			break;
#endif
	}

	stats.ioctl_attempts++;
	if (result>=0) stats.ioctl_successful++;

}

