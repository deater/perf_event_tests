#include <stdio.h>
#include <stdlib.h>

#include "trace_filters.h"

struct trace_event_t *find_event(int event) {

	int i;

	for(i=0;i<NUM_EVENTS;i++) {

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
			case 0:	printf(" ");
				break;
			case 1:	printf("\t");
				break;
		}
	}
}

static void print_comparison(int type) {

	switch(rand()%8) {
		case 0: printf("~"); break;
		case 1: printf("!="); break;
		case 2: printf("=="); break;
		case 3: printf("<"); break;
		case 4: printf("<="); break;
		case 5: printf(">"); break;
		case 6: printf(">="); break;
		case 7: printf("&"); break;
	}
}

static void print_value(int type) {

	if (type==FILTER_TYPE_STR) {
		printf("\"blah\"");
	}
	else {
		printf("%d",rand()%1024);

	}
}


int print_filter(struct trace_event_t *event,
		int max_levels, int max_whitespace) {


	int levels,l,field;

	levels=rand()%max_levels;

	print_whitespace(max_whitespace);

	for(l=0;l<levels;l++) {
		printf("(");
		print_whitespace(max_whitespace);
	}

	l=0;
	do {
		field=rand()%event->num_filters;
		printf("%s",event->filter[field].name);

		print_whitespace(max_whitespace);

		print_comparison(event->filter[field].type);

		print_whitespace(max_whitespace);

		print_value(event->filter[field].type);

		print_whitespace(max_whitespace);

		if (levels==0) break;
		printf(")");

		print_whitespace(max_whitespace);

		if (l!=levels-1) printf("&&");

		print_whitespace(max_whitespace);

		l++;
	} while(l<levels);

	printf("\n");

	return 0;

}


int main(int argc, char **argv) {

	int which,i;
	struct trace_event_t *event;

	for(i=0;i<100;i++) {

		which=rand()%1024;

//		printf("Which: %d\n",which);
		event=find_event(which);
		if (event==NULL) continue;
		printf("Found event %s with %d filters\n",
			event->name,
			event->num_filters);


		print_filter(event, 4 /* MAX LEVELS */,
					1 /* MAX WHITESPACE */);


	}

	return 0;
}
