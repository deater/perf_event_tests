#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>


#include "tracefs_helpers.h"



char *find_tracefs_location(char *buffer, int quiet) {

	int result;

	/* First try /sys/kernel/tracing */
	result=access("/sys/kernel/tracing", R_OK|W_OK);
	if (result==0) {
		strcpy(buffer,"/sys/kernel/tracing");
		return buffer;
	}
	else {
		if (errno==EPERM) {
			fprintf(stderr,"Insufficient permissions!\n");
			return NULL;
		}
	}

	/* Next try /sys/kernel/debug/tracing */
	result=access("/sys/kernel/debug/tracing", R_OK|W_OK);
	if (result==0) {
		strcpy(buffer,"/sys/kernel/debug/tracing");
		return buffer;
	}
	else {
		if (errno==EPERM) {
			fprintf(stderr,"Insufficient permissions!\n");
			return NULL;
		}
	}

	return NULL;
}
