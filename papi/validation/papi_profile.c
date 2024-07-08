#include <stdio.h>
#include <stdlib.h>

#include "papi.h"


double do_flops(int how_many) {

	double vector[2048],out[2048],sum;
	int i,j;

	for(i=0;i<2048;i++) vector[i]=rand()%1024;

	for(i=0;i<how_many;i++) {
		for(j=0;j<2048;j++) out[j]+=vector[j];
	}

	sum=0.0;

	for(i=0;i<2048;i++) sum+=out[i];

	return sum;
}

/* urgh.  this means 1? */
#define FULL_SCALE 65536

int main(int arc, char **argv) {

	int retval;
	int i=0,j;
	void *profbuf;
	unsigned long blength;
	char *start,*end;
	unsigned int scale = FULL_SCALE;
	int EventSet = PAPI_NULL;
	int PAPI_event = PAPI_TOT_CYC;
	int thresh=1000; /* ? */
	int bucket;
	const PAPI_exe_info_t *prginfo;
	long long values[2];
	int num_buckets,bucket_size;
	long long llength;
	long plength;
	unsigned short buf_16;
	int n=1;
	unsigned short **buf16;

	int profflags[5] = {
		PAPI_PROFIL_POSIX,
		PAPI_PROFIL_POSIX | PAPI_PROFIL_RANDOM,
		PAPI_PROFIL_POSIX | PAPI_PROFIL_WEIGHTED,
		PAPI_PROFIL_POSIX | PAPI_PROFIL_COMPRESS,
		PAPI_PROFIL_POSIX | PAPI_PROFIL_WEIGHTED |
		PAPI_PROFIL_RANDOM | PAPI_PROFIL_COMPRESS
        };

	/*************************/
	/* Init the PAPI library */
	/*************************/
	retval = PAPI_library_init( PAPI_VER_CURRENT );
	if (retval!= PAPI_VER_CURRENT ) {
		fprintf(stderr,"Error! could not initialize PAPI\n");
		return 0;
	}

	prginfo = PAPI_get_executable_info(  );
	if (prginfo==NULL) {
		fprintf(stderr,"Error getting executable info\n");
		return 0;
	}

	start = prginfo->address_info.text_start;
        end = prginfo->address_info.text_end;


	/**********************/
	/* Set up an eventset */
	/**********************/
	retval = PAPI_create_eventset(&EventSet);

	retval = PAPI_add_named_event(EventSet,"PAPI_TOT_CYC");
	if (retval!=PAPI_OK) {
		fprintf(stderr,"Error adding event! %s\n",
			PAPI_strerror(retval));
		return 0;
	}

	retval = PAPI_add_named_event(EventSet,"PAPI_TOT_INS");
	if (retval!=PAPI_OK) {
		fprintf(stderr,"Error adding event! %s\n",
			PAPI_strerror(retval));
		return 0;
	}

	plength = end-start;
	llength = ( ( long long ) plength * scale );


	bucket = PAPI_PROFIL_BUCKET_16;
	bucket_size = sizeof(short);

	num_buckets = ( llength / 65536 / 2 );
	blength = num_buckets * bucket_size;


	profbuf = calloc( blength, sizeof(char) );
	if (profbuf==NULL) {
		fprintf(stderr,"Error allocating memory!\n");
		return 0;
	}

	retval = PAPI_profil( profbuf,
				blength,
				start,
				scale,
				EventSet,
				PAPI_event,
				thresh,
				profflags[i] | bucket );

	if (retval!=PAPI_OK) {
		fprintf(stderr,"Error starting profile! %d %s\n",
			retval,
			PAPI_strerror(retval));
	}

	/* Start measuring */
	PAPI_start( EventSet );

	/* Our benchmark */
	do_flops( 100000 );

	/* Stop measuring */
	PAPI_stop( EventSet, values );

	printf("CYC=%lld INS=%lld\n",values[0],values[1]);

	retval = PAPI_profil( profbuf,
				blength,
				start,
				scale,
				EventSet,
				PAPI_event,
				0,
				profflags[i]);

	if (retval!=PAPI_OK) {
		fprintf(stderr,"Error ending profile! %d %s\n",
			retval,
			PAPI_strerror(retval));
	}

	printf( "\n------------------------------------------------------------\n" );
        printf( "PAPI_profil() hash table, Bucket size: %d bits.\n",
                        bucket_size * 8 );
        printf( "Number of buckets: %d.\nLength of buffer: %ld bytes.\n",
                        num_buckets, blength );
        printf( "------------------------------------------------------------\n" );
        printf( "address\t\t\tflat\trandom\tweight\tcomprs\tall\n\n" );

	buf16 = ( unsigned short **) profbuf;

	for ( i = 0; i < num_buckets; i++ ) {
		for ( j = 0, buf_16 = 0; j < n; j++ ) {
			buf_16 |= ( buf16[j] )[i];
		}
                if ( buf_16 ) {
                   printf( "%#-16llx",
                           (long long) (unsigned long)start +
                           ( ( ( long long ) i * scale ) >> 15 ) );
                   for ( j = 0, buf_16 = 0; j < n; j++ )
                       printf( "\t%d", ( buf16[j] )[i] );
                   printf( "\n" );
		}
	}

//	prof_out( start, 5, bucket, num_buckets, scale );


	free(profbuf);

	return 0;
}
