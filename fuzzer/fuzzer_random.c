#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>

#include "perf_event.h"

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

/* Pick an intelligently random refresh value */
uint64_t rand_open_flags(void) {

	uint64_t flags=0;
	int which=0;

	which=rand()%3;

	/* Normal */
	if (which==0) {
		switch(rand()%3) {
			case 0:	flags=O_RDONLY;
				break;
			case 1: flags=O_WRONLY;
				break;
			case 2: flags=O_RDWR;
				break;
		}
	}

	/* Obscure */
	if (which==1) {
		if (rand()%2) flags|=O_RDONLY;
		if (rand()%2) flags|=O_WRONLY;
		if (rand()%2) flags|=O_RDWR;
		if (rand()%2) flags|=O_APPEND;
		if (rand()%2) flags|=O_ASYNC;
		if (rand()%2) flags|=O_CLOEXEC;
		if (rand()%2) flags|=O_CREAT;
#ifdef O_DIRECT
		if (rand()%2) flags|=O_DIRECT;
#endif
		if (rand()%2) flags|=O_DIRECTORY;
		if (rand()%2) flags|=O_DSYNC;
		if (rand()%2) flags|=O_EXCL;
#ifdef O_LARGEFILE
		if (rand()%2) flags|=O_LARGEFILE;
#endif
#ifdef O_NOATIME
		if (rand()%2) flags|=O_NOATIME;
#endif
		if (rand()%2) flags|=O_NOCTTY;
		if (rand()%2) flags|=O_NOFOLLOW;
		if (rand()%2) flags|=O_NONBLOCK;
#ifdef O_PATH
		if (rand()%2) flags|=O_PATH;
#endif
		if (rand()%2) flags|=O_SYNC;
#ifdef O_TMPFILE
		if (rand()%2) flags|=O_TMPFILE;
#endif
		if (rand()%2) flags|=O_TRUNC;
	}

	if (which==2) {
		flags=((uint64_t)rand()<<32)|rand();
	}
	return flags;
}
