#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

#include "filter_address.h"

#define MAX_FILTER_SIZE 8192
static char filter[MAX_FILTER_SIZE];

/* from kernel/core.c */
/*
 * Address range filtering: limiting the data to certain
 * instruction address ranges. Filters are ioctl()ed to us from
 * userspace as ascii strings.
 *
 * Filter string format:
 *
 * ACTION RANGE_SPEC
 * where ACTION is one of the
 *  * "filter": limit the trace to this region
 *  * "start": start tracing from this address
 *  * "stop": stop tracing at this address/region;
 * RANGE_SPEC is
 *  * for kernel addresses: <start address>[/<size>]
 *  * for object files:     <start address>[/<size>]@</path/to/object/file>
 *
 * if <size> is not specified, the range is treated as a single address.
 */

static void print_whitespace(int max_amount) {

	int i,amount;

	if (max_amount==0) max_amount=1;

	amount=rand()%max_amount;

	for(i=0;i<amount;i++) {
		switch(rand()%50) {
			case 0 ... 30:
				strcat(filter," ");
				break;
			case 31:
				strcat(filter,"\r");
				break;
			case 32:
				strcat(filter,"\n");
				break;

			case 33:
				strcat(filter,",");
				break;

			default:
				strcat(filter,"\t");
				break;
		}
	}
}

static unsigned long long get_kernel_address(void) {

	unsigned long long val;

	switch(rand()%6) {
		case 0:	val=((long long)rand()<<32)|rand();
			break;
		case 1:
		case 2:
		case 3:
			val=0xffffffff00000000ULL | rand();
			break;
		case 4:
			val=0xc0000000ULL | rand();

		default:
			val=0;
	}
	return val;
}

static unsigned long long get_size(void) {

	unsigned long long val;

	switch(rand()%5) {
		case 0:	val=((long long)rand()<<32)|rand();
			break;
		case 1: val=rand();
			break;
		case 2: val=rand()&~0xffff;
			break;
		case 3:
			val=0xffffffff00000000ULL | rand();
			break;

		default:
			val=0;

	}
	return val;
}


static char *get_path(void) {

	return "/vmlinuz";
}


int make_address_filter(
		int which,
		char *filter_out,
		int filter_size,
		int max_whitespace,
		int try_valid) {

	int action,range_type;
	char temp[BUFSIZ];

	filter[0]=0;

	action=rand()%5;
	switch(action) {
		case 0: strcpy(filter,"filter"); break;
		case 1: strcpy(filter,"start"); break;
		case 2: strcpy(filter,"stop"); break;
		default: print_whitespace(max_whitespace);
	}

	print_whitespace(max_whitespace);

	range_type=rand()%3;
	if (range_type==0) {
		/* nothing */
		print_whitespace(max_whitespace);
	}
	else {
		/* kernel address or range*/
		switch(rand()%10) {
			case 0: sprintf(temp,"%llu",get_kernel_address());
				break;
			case 1: sprintf(temp,"%lld",get_kernel_address());
				break;
			case 2: sprintf(temp,"%llx",get_kernel_address());
				break;
			case 3: sprintf(temp,"%llo",get_kernel_address());
				break;
			default: sprintf(temp,"0x%llx",get_kernel_address());
				break;
		}
		strcat(filter,temp);
		//print_whitespace(max_whitespace);
		if (rand()%2) {
			switch(rand()%5) {
				case 0: sprintf(temp,"/%llx",get_size());
					break;
				case 1: sprintf(temp,"/%lld",get_size());
					break;
				case 2: sprintf(temp,"/0x%llx",get_size());
					break;
				default: sprintf(temp,"/%llu",get_size());
					break;
			}
			strcat(filter,temp);
		}

		if (range_type==2) {
			sprintf(temp,"@%s",get_path());
			strcat(filter,temp);
		}
	}

	memcpy(filter_out,filter,filter_size);

	return 0;

}

