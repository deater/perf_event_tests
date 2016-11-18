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

#include "trace_filters.h"

#include "filter_tracepoint.h"

#include "bpf.h"
#include "libbpf.h"
#include <sys/utsname.h>
#include <asm/unistd.h>

#include "bpf_helpers.h"

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

int make_tracepoint_filter(
		int which,
		char *filter_out,
		int filter_size,
		int max_levels,
		int max_whitespace,
		int try_valid) {


	int levels,l;
	struct trace_event_t *event;

	event=find_event(which);
	if (event==NULL) return -1;

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

	memcpy(filter_out,filter,filter_size);

	return 0;

}

#if DEBUG_TRACEPOINT
char *tracepoint_name(int which) {

        struct trace_event_t *event;

        event=find_event(which);
        if (event==NULL) return NULL;

        return event->name;
}
#endif

