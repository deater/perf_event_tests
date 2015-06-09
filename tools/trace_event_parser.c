/* Parses the trace events exported by the kernel	*/
/* Under /sys/kernel/debug/tracing			*/

#define SYSFS "/sys/kernel/debug/tracing/events/"
//#define SYSFS "./fakesys/snb_ep/sys/bus/event_source/devices/"

#define _BSD_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

struct format_type {
	char *name;
	char *value;
	int field;
	unsigned long long mask;
};

struct subevent_type {
	char *name;
	int id;
	struct format_type *format;
};

struct event_type {
	char *name;
	int num_subevents;
	struct subevent_type *subevent;
};

static int num_events=0;

static struct event_type *events=NULL;


#define FIELD_UNKNOWN	0
#define FIELD_CONFIG	1
#define FIELD_CONFIG1	2
#define FIELD_CONFIG2   3

#define MAX_FIELDS	4

char fieldnames[MAX_FIELDS][20]={
	"unknown",
	"config",
	"config1",
	"config2",
};

#if 0
static int parse_format(char *string, int *field_type, unsigned long long *mask) {

	int i,firstnum,secondnum,shift,bits;
	char format_string[BUFSIZ];

	*mask=0;

	/* get format */
	/* according to Documentation/ABI/testing/sysfs-bus-event_source-devices-format */
	/* the format is something like config1:1,6-10,44 */

	i=0;
	while(1) {
		format_string[i]=string[i];
		if (string[i]==':') {
			format_string[i]=0;
			break;
		}
		if (string[i]==0) break;
		i++;
	}

	if (!strcmp(format_string,"config")) {
		*field_type=FIELD_CONFIG;
	} else if (!strcmp(format_string,"config1")) {
		*field_type=FIELD_CONFIG1;
	} else if (!strcmp(format_string,"config2")) {
		*field_type=FIELD_CONFIG2;
	}
	else {
		*field_type=FIELD_UNKNOWN;
	}

	while(1) {

		/* Read first number */
		i++;
		firstnum=0;
		while(1) {
			if (string[i]==0) break;
			if (string[i]=='-') break;
			if (string[i]==',') break;
			if ((string[i]<'0') || (string[i]>'9')) {
				fprintf(stderr,"Unknown format char %c\n",string[i]);
				return -1;
			}
			firstnum*=10;
			firstnum+=(string[i])-'0';
			i++;
		}
		shift=firstnum;

		/* check if no second num */
		if ((string[i]==0) || (string[i]==',')) {
			bits=1;
		}
		else {
			/* Read second number */
			i++;
			secondnum=0;
			while(1) {
				if (string[i]==0) break;
				if (string[i]=='-') break;
				if (string[i]==',') break;
				if ((string[i]<'0') || (string[i]>'9')) {
					fprintf(stderr,"Unknown format char %c\n",string[i]);
					return -1;
				}
				secondnum*=10;
				secondnum+=(string[i])-'0';
				i++;
			}
			bits=(secondnum-firstnum)+1;
		}

		if (bits==64) {
			*mask|=0xffffffffffffffffULL;
		} else {
			*mask|=((1ULL<<bits)-1)<<shift;
		}

		if (string[i]==0) break;

	}
	return 0;
}

char *field_to_name(int field) {

	if (field>=MAX_FIELDS) return fieldnames[0];

	return fieldnames[field];
}

static unsigned long long separate_bits(unsigned long long value,
					unsigned long long mask) {

	int value_bit=0,i;
	unsigned long long result=0;

	for(i=0;i<64;i++) {
		if ((1ULL<<i)&mask) {
			result|=((value>>value_bit)&1)<<i;
			value_bit++;
		}
	}

	return result;
}

static int update_configs(int pmu, char *field,
			long long value,
			long long *c,
			long long *c1,
			long long *c2) {

	int i;

	for(i=0;i<events[pmu].num_formats;i++) {
		if (!strcmp(field,events[pmu].formats[i].name)) {
			if (events[pmu].formats[i].field==FIELD_CONFIG) {
				*c|=separate_bits(value,
						events[pmu].formats[i].mask);
				return 0;
			}

			if (events[pmu].formats[i].field==FIELD_CONFIG1) {
				*c1|=separate_bits(value,
						events[pmu].formats[i].mask);
				return 0;
			}

			if (events[pmu].formats[i].field==FIELD_CONFIG2) {
				*c2|=separate_bits(value,
						events[pmu].formats[i].mask);
				return 0;
			}

		}
	}

	return 0;
}


#endif

static int init_trace_events(void) {

	DIR *dir,*event_dir,*format_dir;
	struct dirent *entry,*event_entry,*format_entry;
	char dir_name[BUFSIZ],event_name[BUFSIZ],event_value[BUFSIZ],
		temp_name[BUFSIZ],format_name[BUFSIZ],format_value[BUFSIZ];
	int type,format_num=0,generic_num=0;
	FILE *fff;
	int result,subevents;
	int num_subevents,current_event,current_subevent;


	/* Count number of trace events */
	/* This may break if events are ever added/removed on the fly? */

	dir=opendir(SYSFS);
	if (dir==NULL) {
		fprintf(stderr,"Unable to opendir "
			SYSFS " : %s\n",
			strerror(errno));
		return -1;
	}

	while(1) {
		entry=readdir(dir);
		if (entry==NULL) break;
		if (entry->d_type!=DT_DIR) continue;
		if (!strcmp(".",entry->d_name)) continue;
		if (!strcmp("..",entry->d_name)) continue;
		num_events++;
	}

	if (num_events<1) return -1;

	events=calloc(num_events,sizeof(struct event_type));
	if (events==NULL) {
		return -1;
	}

	/******************/
	/* Add each event */
	/******************/

	rewinddir(dir);
	current_event=0;

	while(1) {
		entry=readdir(dir);
		if (entry==NULL) break;
		if (entry->d_type!=DT_DIR) continue;
		if (!strcmp(".",entry->d_name)) continue;
		if (!strcmp("..",entry->d_name)) continue;

		/* read name */
		events[current_event].name=strdup(entry->d_name);
		sprintf(dir_name,SYSFS"/%s",
			entry->d_name);

		/* Scan for subevents */
		num_subevents=0;
		event_dir=opendir(dir_name);
		if (event_dir==NULL) {
			fprintf(stderr,"Unable to opendir %s : %s\n",
			dir_name,
			strerror(errno));
			return -1;
		}

		while(1) {
			event_entry=readdir(event_dir);
			if (event_entry==NULL) break;
			if (event_entry->d_type!=DT_DIR) continue;
			if (!strcmp(".",event_entry->d_name)) continue;
			if (!strcmp("..",event_entry->d_name)) continue;
//			printf("%s:%s\n",entry->d_name,event_entry->d_name);
			num_subevents++;
		}

		events[current_event].num_subevents=num_subevents;

		events[current_event].subevent=calloc(
				num_subevents,sizeof(struct subevent_type));
		if (events[current_event].subevent==NULL) {
			return -1;
		}

		/* Add each subevent */
		current_subevent=0;
		rewinddir(event_dir);

		while(1) {
			event_entry=readdir(event_dir);
			if (event_entry==NULL) break;
			if (event_entry->d_type!=DT_DIR) continue;
			if (!strcmp(".",event_entry->d_name)) continue;
			if (!strcmp("..",event_entry->d_name)) continue;

			events[current_event].subevent[current_subevent].name=
				strdup(event_entry->d_name);
			current_subevent++;

		}

#if 0
		/***********************/
		/* Scan format strings */
		/***********************/
		sprintf(format_name,"%s/format",dir_name);
		format_dir=opendir(format_name);
		if (format_dir==NULL) {
			/* Can be normal to have no format strings */
		}
		else {
			/* Count format strings */
			while(1) {
				format_entry=readdir(format_dir);
				if (format_entry==NULL) break;
				if (!strcmp(".",format_entry->d_name)) continue;
				if (!strcmp("..",format_entry->d_name)) continue;
				events[pmu_num].num_formats++;
			}

			/* Allocate format structure */
			events[pmu_num].formats=calloc(events[pmu_num].num_formats,
							sizeof(struct format_type));
			if (events[pmu_num].formats==NULL) {
				events[pmu_num].num_formats=0;
				return -1;
			}

			/* Read format string info */
			rewinddir(format_dir);
			format_num=0;
			while(1) {
				format_entry=readdir(format_dir);

				if (format_entry==NULL) break;
				if (!strcmp(".",format_entry->d_name)) continue;
				if (!strcmp("..",format_entry->d_name)) continue;

				events[pmu_num].formats[format_num].name=
					strdup(format_entry->d_name);
				sprintf(temp_name,"%s/format/%s",
					dir_name,format_entry->d_name);
				fff=fopen(temp_name,"r");
				if (fff!=NULL) {
					result=fscanf(fff,"%s",format_value);
					if (result==1) { 
						events[pmu_num].formats[format_num].value=
						strdup(format_value);
					}
					fclose(fff);

					parse_format(format_value,
						&events[pmu_num].formats[format_num].field,
						&events[pmu_num].formats[format_num].mask);
					format_num++;
				}
			}
			closedir(format_dir);
		}

		/***********************/
		/* Scan generic events */
		/***********************/
		sprintf(event_name,"%s/events",dir_name);
		event_dir=opendir(event_name);
		if (event_dir==NULL) {
			/* It's sometimes normal to have no generic events */
		}
		else {

			/* Count generic events */
			while(1) {
				event_entry=readdir(event_dir);
				if (event_entry==NULL) break;
				if (!strcmp(".",event_entry->d_name)) continue;
				if (!strcmp("..",event_entry->d_name)) continue;
				events[pmu_num].num_generic_events++;
			}

			/* Allocate generic events */
			events[pmu_num].generic_events=calloc(
				events[pmu_num].num_generic_events,
				sizeof(struct generic_event_type));
			if (events[pmu_num].generic_events==NULL) {
				events[pmu_num].num_generic_events=0;
				return -1;
			}

			/* Read in generic events */
			rewinddir(event_dir);
			generic_num=0;
			while(1) {
				event_entry=readdir(event_dir);
				if (event_entry==NULL) break;
				if (!strcmp(".",event_entry->d_name)) continue;
				if (!strcmp("..",event_entry->d_name)) continue;

				events[pmu_num].generic_events[generic_num].name=
					strdup(event_entry->d_name);
				sprintf(temp_name,"%s/events/%s",
					dir_name,event_entry->d_name);
				fff=fopen(temp_name,"r");
				if (fff!=NULL) {
					result=fscanf(fff,"%s",event_value);
					if (result==1) {
						events[pmu_num].generic_events[generic_num].value=
							strdup(event_value);
					}
					fclose(fff);
				}
				parse_generic(pmu_num,event_value,
						&events[pmu_num].generic_events[generic_num].config,
						&events[pmu_num].generic_events[generic_num].config1,
						&events[pmu_num].generic_events[generic_num].config2);
				generic_num++;
			}
			closedir(event_dir);
		}
#endif
		current_event++;
	}

	closedir(dir);

	return 0;

}


void dump_trace_events(void) {

	int i,j;

	printf("\nDumping Trace Events\n\n");

	for(i=0;i < num_events;i++) {

		for(j=0;j<events[i].num_subevents;j++) {

			printf("%s:%s type=%d\n",
				events[i].name,
				events[i].subevent[j].name,
				events[i].subevent[j].id);
		}
#if 0

		if (events[i].num_formats) {
			printf("\tFormats:\n");
			for(j=0;j<events[i].num_formats;j++) {
				printf("\t\t%s : ",events[i].formats[j].name);
				printf("%s\n",events[i].formats[j].value);
				printf("\t\t\tfield: %s mask: %llx\n",
					field_to_name(events[i].formats[j].field),
					events[i].formats[j].mask);
			}
		}
		if (events[i].num_generic_events) {
			printf("\tGeneric Events:\n");
			for(j=0;j<events[i].num_generic_events;j++) {
				printf("\t\t%s : ",
					events[i].generic_events[j].name);
				printf("%s\n",events[i].generic_events[j].value);
				printf("\t\t\tconfig: %llx  config1: %llx\n",
					events[i].generic_events[j].config,
					events[i].generic_events[j].config1);
			}
		}
#endif
	}
}

int main(int argc, char **argv) {

	init_trace_events();

	dump_trace_events();

	return 0;

}
