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

	/* Most likely a random number */
	if (rand()%2) {
		period=rand();
	}
	/* Otherwise, some corner cases */
	else {

		switch(rand()%8) {
			case 0:	period=0;		break;
			case 1: period=1;		break;
			case 2: period=-1;		break;
			case 3:	period=rand()%100000;	break;
			case 4: period=rand();		break;
			case 5: period=rand()%256;	break;
			case 6: period=-(rand()%256);	break;
			default: period=rand();
		}
	}

	return period;
}

/* pick an intelligently random ioctl argument */
int rand_ioctl_arg(void) {

	int value=0;

	switch(rand()%4) {
		case 0:	value=0;
			break;
		case 1: value|=PERF_IOC_FLAG_GROUP;
			break;
		/* A small value */
		case 2: value=rand()%256;
			break;
		/* Completely random */
		case 3: value=rand();
			break;
		default:
			break;
	}

	return value;

}
