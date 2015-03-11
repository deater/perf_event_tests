#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "detect_cache.h"

#include "test_utils.h"


struct cache_info_t cache_info[MAX_CACHE_LEVEL][MAX_CACHE_TYPE];

static int cache_info_initialized=0;
static int max_level=0;

int gather_cache_info(int quiet, char *test_string) {

	int type,level,i,j,result;
	int size,line_size,associativity,sets;
	DIR *dir;
	FILE *fff;
	char filename[BUFSIZ],type_string[BUFSIZ];

	struct dirent *d;

	/* clear out cache structure */
	for(i=0;i<MAX_CACHE_LEVEL;i++) {
		for(j=0;j<MAX_CACHE_TYPE;j++) {
			cache_info[i][j].exists=0;
		}
	}

	/* open Linux cache dir */
	/* assume all CPUs same as cpu0.  Not necessarily a good assumption */

	dir=opendir("/sys/devices/system/cpu/cpu0/cache");
	if (dir==NULL) {
		if (!quiet) {
			fprintf(stderr,"Could not open "
				"/sys/devices/system/cpu/cpu0/cache\n");
		}
		return -1;
	}

	while(1) {
		d = readdir(dir);
		if (d==NULL) break;

		if (strncmp(d->d_name, "index", 5)) continue;

#if DEBUG
		printf("Found %s\n",d->d_name);
#endif

		/* level */
		sprintf(filename,
			"/sys/devices/system/cpu/cpu0/cache/%s/level",
			d->d_name);
		fff=fopen(filename,"r");
		if (fff==NULL) {
			fprintf(stderr,"Cannot open level.\n");
		}

		result=fscanf(fff,"%d",&level);
		fclose(fff);
		if (result!=1) printf("Could not read cache level\n");

		/* type */
		sprintf(filename,
			"/sys/devices/system/cpu/cpu0/cache/%s/type",d->d_name);
		fff=fopen(filename,"r");
		if (fff==NULL) {
			fprintf(stderr,"Cannot open type\n");
		}
		result=fscanf(fff,"%s",type_string);
		fclose(fff);
		if (result!=1) fprintf(stderr,"Could not read cache type\n");

		/* Size */
		sprintf(filename,
			"/sys/devices/system/cpu/cpu0/cache/%s/size",d->d_name);
		fff=fopen(filename,"r");
		if (fff==NULL) {
			fprintf(stderr,"Cannot open size\n");
		}
		result=fscanf(fff,"%d",&size);
		fclose(fff);
		if (result!=1) fprintf(stderr,"Could not read cache size\n");

		/* Line Size */
		sprintf(filename,
			"/sys/devices/system/cpu/cpu0/cache/%s/coherency_line_size",
			d->d_name);
		fff=fopen(filename,"r");
		if (fff==NULL) {
			fprintf(stderr,"Cannot open linesize\n");
		}
		result=fscanf(fff,"%d",&line_size);
		fclose(fff);
		if (result!=1) fprintf(stderr,"Could not read cache line-size\n");

		/* Associativity */
		sprintf(filename,
			"/sys/devices/system/cpu/cpu0/cache/%s/ways_of_associativity",
			d->d_name);
		fff=fopen(filename,"r");
		if (fff==NULL) {
			fprintf(stderr,"Cannot open associativity\n");
		}
		result=fscanf(fff,"%d",&associativity);
		fclose(fff);
		if (result!=1) fprintf(stderr,"Could not read cache associativity\n");

		/* Sets */
		sprintf(filename,
			"/sys/devices/system/cpu/cpu0/cache/%s/number_of_sets",
			d->d_name);
		fff=fopen(filename,"r");
		if (fff==NULL) {
			fprintf(stderr,"Cannot open sets\n");
		}
		result=fscanf(fff,"%d",&sets);
		fclose(fff);
		if (result!=1) fprintf(stderr,"Could not read cache sets\n");

		if (((size*1024)/line_size/associativity)!=sets) {
			fprintf(stderr,"Assert!  sets %d != expected %d\n",
				sets,((size*1024)/line_size/associativity));
		}

#if DEBUG
		printf("\tL%d %s cache\n",level,type_string);
		printf("\t%d kilobytes\n",size);
		printf("\t%d byte linesize\n",line_size);
		printf("\t%d-way associative\n",associativity);
		printf("\t%d lines\n",sets);
		printf("\tUnknown inclusivity\n");
		printf("\tUnknown replacement algorithm\n");
		printf("\tUnknown if victim cache\n");
#endif

		if (level>max_level) max_level=level;

		if (level>=MAX_CACHE_LEVEL) {
			if (!quiet) {
				fprintf(stderr,
					"Exceeded maximum cache level %d\n",
					MAX_CACHE_LEVEL);
       			}
			return -2;
		}

		type=CACHE_UNKNOWN;

		if (!strcmp(type_string,"Instruction")) {
			type=CACHE_I;
		}
		if (!strcmp(type_string,"Data")) {
			type=CACHE_D;
		}

		if (!strcmp(type_string,"Unified")) {
			type=CACHE_U;
		}

		cache_info[level][type].exists=1;
		cache_info[level][type].level=level;
		cache_info[level][type].type=type;
		cache_info[level][type].wpolicy=CACHE_UNKNOWN;
		cache_info[level][type].replace=CACHE_UNKNOWN;
		cache_info[level][type].size=size*1024;
		cache_info[level][type].lines=sets;
		cache_info[level][type].ways=associativity;
		cache_info[level][type].linesize=line_size;

	}
	cache_info_initialized=1;

	return 0;

}

void print_cache_info(int quiet, struct cache_info_t *cache_entry) {

	if (quiet) return;
	if (!cache_entry->exists) return;

	printf("L%d ",cache_entry->level);
	switch(cache_entry->type) {
		case CACHE_I: printf("Instruction"); break;
		case CACHE_D: printf("Data"); break;
		case CACHE_U: printf("Unified"); break;
		case CACHE_T: printf("Trace"); break;
		default: printf("Unknown"); break;
	}
	printf(" Cache\n");
	printf("\tSize: %d kilobytes\n",cache_entry->size/1024);
	printf("\tLine Size: %d bytes\n",cache_entry->linesize);
	printf("\tLines: %d\n",cache_entry->lines);
	printf("\tAssociativity: %d\n",cache_entry->ways);
	printf("\tInclusivity: UNKNOWN\n");
	printf("\tReplacement Policy: UNKNOWN\n");
}

int cache_get_max_levels(int quiet, char *test_string) {

	if (!cache_info_initialized) {
		gather_cache_info(quiet,test_string);
	}

	return max_level;
}
