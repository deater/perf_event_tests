#include <stdlib.h>

#include "../include/perf_event.h"

/* Pick an intelligently random refresh value */
int rand_refresh(void) {

	int refresh;

	switch(rand()%6) {
		case 0:	refresh=0;	break;
		case 1: refresh=1;	break;
		case 2: refresh=-1;	break;
		case 3:	refresh=rand()%100; break;
		case 4: refresh=rand();	break;
		default: refresh=1;
	}

	return refresh;

}

/* pick an intelligently random period */
int rand_period(void) {

	int period;

	switch(rand()%6) {
		case 0:	period=0;	break;
		case 1: period=1;	break;
		case 2: period=-1;	break;
		case 3:	period=rand()%100000; break;
		case 4: period=rand();	break;
		default: period=1;
	}

	return period;
}

/* pick an intelligently random ioctl argument */
int rand_ioctl_arg(void) {

	int value=0;

	switch(rand()%3) {
		case 0:	value=0;
			break;
		case 1: value|=PERF_IOC_FLAG_GROUP;
			break;
		case 2: value=rand();
			break;
		default:
			break;
	}

	return value;

}
