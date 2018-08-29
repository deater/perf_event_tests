/* Parses the trace events exported by the kernel	*/
/* Under /sys/kernel/debug/tracing			*/

#define SYSFS "/sys/kernel/tracing/events/"
//#define SYSFS "/sys/kernel/debug/tracing/events/"
//#define SYSFS "./fakesys/snb_ep/sys/bus/event_source/devices/"

#define _DEFAULT_SOURCE
//#define _BSD_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define MAX_FORMAT	30

struct format_type {
	char *name;
	int type;
};

struct subevent_type {
	char *name;
	int id;
	int num_formats;
	struct format_type format[MAX_FORMAT];
};

struct event_type {
	char *name;
	int num_subevents;
	struct subevent_type *subevent;
};

static int num_events=0;
static int total_events=0;

static struct event_type *events=NULL;

#define FILTER_TYPE_INVALID	-1
#define FILTER_TYPE_INT		0
#define FILTER_TYPE_POINTER	1
#define FILTER_TYPE_STRING	2

static char filter_type_names[3][30]=
	{"FILTER_TYPE_INT",
	"FILTER_TYPE_PTR",
	"FILTER_TYPE_STR"
};



int parse_type(char *buffer, char *out_name) {

	char *type;
	char *name;
	int filter_type=FILTER_TYPE_INT;
	int i;

//	printf("%s",buffer);

	type=strtok(buffer," \t\n");
	if (type==NULL) return FILTER_TYPE_INVALID;

	if (!strcmp(type,"unsigned")) {
		type=strtok(NULL," \t\n");
		if (type==NULL) return FILTER_TYPE_INVALID;
	}

	if (!strcmp(type,"enum")) {
		type=strtok(NULL," \t\n");
		if (type==NULL) return FILTER_TYPE_INVALID;
	}

//	printf("type=%s ",type);

	name=strtok(NULL," \t\n");
	if (name==NULL) return FILTER_TYPE_INVALID;

	if (name[strlen(name)-1]!=';') {
		name=strtok(NULL," \t\n");
		if (name==NULL) return FILTER_TYPE_INVALID;
	}

	if (name[0]=='*') {
		name=strtok(NULL," \t\n");
		if (name==NULL) return FILTER_TYPE_INVALID;
		filter_type=FILTER_TYPE_POINTER;
	}

	for(i=0;i<strlen(name);i++) {
		if (name[i]==';') name[i]=0;
		if (name[i]=='[') {
			name[i]=0;
			filter_type=FILTER_TYPE_STRING;
		}
	}
//	printf("name=%s\n",name);

	strcpy(out_name,name);

	return filter_type;

}

static int init_trace_events(void) {

	DIR *dir,*event_dir;
	struct dirent *entry,*event_entry;
	char dir_name[BUFSIZ],filename[BUFSIZ*2],buffer[BUFSIZ];
	FILE *fff;
	int num_subevents,current_event,current_subevent,current_format;
	char field_name[BUFSIZ];
	int field_type;

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
		snprintf(dir_name,BUFSIZ,SYSFS"/%s",entry->d_name);

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
			total_events++;
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

			/* get id */
			snprintf(filename,BUFSIZ*2,"%s/%s/id",dir_name,
					event_entry->d_name);
			fff=fopen(filename,"r");
			if (fff==NULL) {
				fprintf(stderr,"error opening %s\n",filename);
				return -1;
			}
			fscanf(fff,"%d",
				&events[current_event].subevent[current_subevent].id);
			fclose(fff);

			/* get format */
			sprintf(filename,"%s/%s/format",dir_name,
					event_entry->d_name);
			fff=fopen(filename,"r");
			if (fff==NULL) {
				fprintf(stderr,"error opening %s\n",filename);
				return -1;
			}

			current_format=0;
			while(1) {
				if (fgets(buffer,BUFSIZ,fff)==NULL) break;
				if (!strncmp(buffer,"\tfield:",7)) {
					field_type=parse_type(buffer+7,field_name);
					events[current_event].subevent[current_subevent].format[current_format].type=field_type;
					events[current_event].subevent[current_subevent].format[current_format].name=strdup(field_name);
					current_format++;
					if (current_format>MAX_FORMAT) {
						fprintf(stderr,"Format too high!\n");
						return -1;
					}
					events[current_event].subevent[current_subevent].num_formats=current_format;
				}
			}
			fclose(fff);

			current_subevent++;

		}

		current_event++;
	}

	closedir(dir);

	return 0;

}


void dump_trace_events(void) {

	int i,j,k;

	printf("\nDumping Trace Events\n\n");

	for(i=0;i < num_events;i++) {

		for(j=0;j<events[i].num_subevents;j++) {

			printf("%s:%s id=%d\n",
				events[i].name,
				events[i].subevent[j].name,
				events[i].subevent[j].id);

			for(k=0;k<events[i].subevent[j].num_formats;k++) {
				printf("\t%d %s\n",
					events[i].subevent[j].format[k].type,
					events[i].subevent[j].format[k].name);
			}
		}

	}
}




void dump_trace_header(void) {

	FILE *fff;

	int i,j,k;

	fff=fopen("trace_filters.h","w");
	if (fff==NULL) {
		fprintf(stderr,"Could not open!\n");
		return;
	}

	fprintf(fff,"/* Auto-generated by trace_event_parser */\n\n");

	fprintf(fff,"struct trace_filter_t {\n");
	fprintf(fff,"\tint type;\n");
	fprintf(fff,"\tchar name[50];\n");
	fprintf(fff,"};\n\n");

	fprintf(fff,"struct trace_event_t {\n");
	fprintf(fff,"\tint id;\n");
	fprintf(fff,"\tchar name[50];\n");
	fprintf(fff,"\tint num_filters;\n");
	fprintf(fff,"\tstruct trace_filter_t filter[30];\n");
	fprintf(fff,"};\n\n");

	fprintf(fff,"#define NUM_EVENTS %d\n\n",total_events);

	fprintf(fff,"#define FILTER_TYPE_INT 0\n");
	fprintf(fff,"#define FILTER_TYPE_PTR 1\n");
	fprintf(fff,"#define FILTER_TYPE_STR 2\n");

	fprintf(fff,"struct trace_event_t trace_events[NUM_EVENTS] = {\n");

	for(i=0;i < num_events;i++) {

		for(j=0;j<events[i].num_subevents;j++) {

			fprintf(fff,"\t{ .id=%d, .name=\"%s:%s\", "
				".num_filters=%d,\n",
				events[i].subevent[j].id,
				events[i].name,
				events[i].subevent[j].name,
				events[i].subevent[j].num_formats);


			for(k=0;k<events[i].subevent[j].num_formats;k++) {
				fprintf(fff,"\t\t.filter[%d].type=%s, "
					".filter[%d].name=\"%s\",\n",
					k,filter_type_names[events[i].subevent[j].format[k].type],
					k,events[i].subevent[j].format[k].name);
			}
			fprintf(fff,"\t},\n");

		}

	}

	fprintf(fff,"};\n");

	fclose(fff);

}

int main(int argc, char **argv) {

	init_trace_events();

//	dump_trace_events();
	dump_trace_header();

	return 0;

}
