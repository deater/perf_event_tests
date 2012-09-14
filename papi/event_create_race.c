/* Check for event creation race with libpfm4 PAPI */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

#include <pthread.h>

#include "papiStdEventDefs.h"
#include "papi.h"


#include "test_utils.h"

#define MAX_EVENTS 4096

/* too big to fit on stack */
char event_names[MAX_EVENTS][BUFSIZ];
int event_codes[MAX_EVENTS];
int num_events;

#define NUM_THREADS 4

void *our_thread( void *arg ) {

  int retval,i;

  printf("Entering thread\n");

  retval = PAPI_register_thread(  );
  for(i=0;i<num_events;i++) {
     PAPI_event_name_to_code (event_names[i], &event_codes[i]);
  }

  printf("Done thread\n");

  retval = PAPI_unregister_thread(  );

  return NULL;
}

int main( int argc, char **argv ) {

  int i, k, num_umasks;
    int retval,rc;
    PAPI_event_info_t info;
    int numcmp, cid;
    int quiet;
    char test_string[]="Checking for event create race...";
    char *ptr;
    char temp_name[BUFSIZ];

    pthread_attr_t attr;
    pthread_t threads[NUM_THREADS];

    quiet=test_quiet();

	/* Initialize PAPI library */
    retval = PAPI_library_init( PAPI_VER_CURRENT );
    if ( retval != PAPI_VER_CURRENT ) {
       test_fail(test_string);
    }

    numcmp = PAPI_num_components(  );

    num_events = 0;

    for(cid=0; cid<numcmp; cid++ ) {
       i = 0 | PAPI_NATIVE_MASK;
       PAPI_enum_cmp_event( &i, PAPI_ENUM_FIRST, cid );

       do {
          memset( &info, 0, sizeof ( info ) );
	  retval = PAPI_get_event_info( i, &info );

	      /* This event may not exist */
	  if ( retval != PAPI_OK ) continue;

	      /* save the main event name */
	  strncpy(temp_name,info.symbol,BUFSIZ);

	      /* Handle umasks */
		
	  k = i;
	  num_umasks=0;
	  if ( PAPI_enum_event( &k, PAPI_NTV_ENUM_UMASKS ) == PAPI_OK ) {
	     do {
		retval = PAPI_get_event_info( k, &info );
		if ( retval == PAPI_OK ) {
		      /* handle libpfm4-style events which have a */
		      /* pmu::event type event name               */
                   if ((ptr=strstr(info.symbol, "::"))) {
                      ptr+=2;
		   }
                   else {
                      ptr=info.symbol;
		   }

                   if ( ( strchr( ptr, ':' ) ) == NULL ) {
		      /* shouldn't happen */
		      if (!quiet) printf("Weird umask!\n");
		      test_fail(test_string);
		   }
		   else {
		     //	      if (!quiet) printf("%s\n",info.symbol);
		      num_umasks++;
	              if (num_events<MAX_EVENTS) {
			 strncpy(event_names[num_events],info.symbol,BUFSIZ);
		      }
		      num_events++;
		   }
		}
	     } while ( PAPI_enum_event( &k,PAPI_NTV_ENUM_UMASKS) == PAPI_OK );
		  
	  }
	  if (num_umasks==0) {
	    //	     if (!quiet) printf("%s\n",temp_name);
	     if (num_events<MAX_EVENTS) {
	        strncpy(event_names[num_events],temp_name,BUFSIZ);
	     }
	     num_events++;
	  }

       } while ( PAPI_enum_event( &i, PAPI_ENUM_EVENTS ) == PAPI_OK );

    }

    if (!quiet) printf( "Total events reported: %d\n", num_events );
    if (num_events>MAX_EVENTS) num_events=MAX_EVENTS;


    /* shut down the library */
    PAPI_shutdown();
    
	/* Initialize PAPI library again */
    retval = PAPI_library_init( PAPI_VER_CURRENT );
    if ( retval != PAPI_VER_CURRENT ) {
       test_fail(test_string);
    }

    //    for(i=0;i<num_events;i++) {
    //  PAPI_event_name_to_code (event_names[i], &event_codes[i]);
    //}


    /* start some threads */


    retval = PAPI_thread_init( ( unsigned long ( * )( void ) ) 
			       ( pthread_self ) );

    pthread_attr_init( &attr );

    if (!quiet) printf("Starting threads...\n");

    for(i=0;i<NUM_THREADS;i++) {
      rc = pthread_create( &threads[i], &attr, our_thread, NULL);
      if ( rc ) {
         test_fail( test_string);
      }
    }


    pthread_attr_destroy( &attr );

    for(i=0;i<NUM_THREADS;i++) {
       pthread_join( threads[i], NULL );
    }

    if (!quiet) printf("Done with threads...\n");

    for(i=0;i<num_events;i++) {
      if (!quiet) printf("%x %s\n",event_codes[i],event_names[i]);
    }

    /* shut down the library again */
    PAPI_shutdown();

    test_pass(test_string);

    //    pthread_exit( NULL );


    return 0;

}
