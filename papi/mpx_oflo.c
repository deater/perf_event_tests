/* This code tests simultaneous multiplex and overflow  */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu          */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "papi.h"

#include "papi_helpers.h"

#include "test_utils.h"

#include "matrix_multiply.h"



int total=0;

void
handler( int EventSet, void *address, long long overflow_vector, void *context ) {


  total++;
}

static char events[TEST_MAX_EVENTS][TEST_MAX_STRLEN];

int main(int argc, char **argv) {

  int retval,quiet,i,max;
  int EventSet = PAPI_NULL;
  char test_string[]="Testing simultaneous overflow and multiplexing...";
  long long values[32];
  int processor;
    int event;


  quiet=test_quiet();


  /* Initialize the library */
  retval = PAPI_library_init( PAPI_VER_CURRENT );
  if (retval != PAPI_VER_CURRENT) {
    if (!quiet) printf("Error! PAPI_library_init %d\n",retval);
    test_fail(test_string);
  }

  /* Detect Processor */
  processor=detect_processor();
  if (processor==PROCESSOR_UNKNOWN) {
    if (!quiet) printf("Unknown processor!\n");
    test_skip(test_string);
  }

  /* copy event names */
  if (copy_events((char *)events)) {
    if (!quiet) printf("Unknown processor!\n");
    test_skip(test_string);
  }

  /* Initialize multiplexing */ 
  retval = PAPI_multiplex_init(  );
  if ( retval == PAPI_ENOSUPP) {
    if (!quiet) printf("Multiplexing not supported\n");
    test_skip(test_string);
  }
  else if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_multiplex_init() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_assign_eventset_component( EventSet, 0 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_assign_eventset_component() failed\n");
    test_fail(test_string);
  }

    retval=PAPI_event_name_to_code(events[0],&event);
  retval = PAPI_add_event( EventSet, event );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  max=6;
  for(i=1;i<6;i++) {

    retval=PAPI_event_name_to_code(events[i],&event);
    if (retval!=PAPI_OK) {
      if (!quiet) printf( "PAPI_event_name_to_code() failed %s\n",events[i]);
      test_fail(test_string);
    }

     retval = PAPI_add_event( EventSet, event );
     if ( retval != PAPI_OK ) {
       //       if (!quiet) printf( "PAPI_add_event() failed %d %s\n",i,
       //		   PAPI_descr_error(retval));
       // test_fail(test_string);
       max=i;
       break;
     }
  }

  /* No MPX no OFLO */

  if (!quiet) printf("No mpx, no oflo\n");

  retval = PAPI_start(EventSet);
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_start() failed\n");
    test_fail(test_string);
  }

  naive_matrix_multiply(1);

  retval = PAPI_stop( EventSet, values );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_stop() failed\n");
    test_fail(test_string);
  }


  if (!quiet) {

    for(i=0;i<max;i++) {
      printf("\t%s: %lld\n",events[i],values[i]);
    }
    printf("\tCycle oflos (100k) %d\n",
	   total);
  }


  /* MPX no OFLO */

  if (!quiet) printf("Mpx, no oflo\n");

  retval = PAPI_cleanup_eventset( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_destroy_eventset( &EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_destroy_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_assign_eventset_component( EventSet, 0 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_assign_eventset_component() failed\n");
    test_fail(test_string);
  }

    retval=PAPI_event_name_to_code(events[0],&event);

  retval = PAPI_add_event( EventSet, event );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() %s failed\n",events[0]);
    test_fail(test_string);
  }

  retval = PAPI_set_multiplex( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_set_multiplex() failed\n");
    test_fail(test_string);
  }

  max=6;
  for(i=1;i<6;i++) {

    retval=PAPI_event_name_to_code(events[i],&event);
    if (retval!=PAPI_OK) {
      if (!quiet) printf( "PAPI_event_name_to_code() failed %s\n",events[i]);
      test_fail(test_string);
    }

     retval = PAPI_add_event( EventSet, event );
     if ( retval != PAPI_OK ) {
       //       if (!quiet) printf( "PAPI_add_event() failed %d %s\n",i,
       //		   PAPI_descr_error(retval));
       // test_fail(test_string);
       max=i;
       break;
     }
  }

  retval = PAPI_start(EventSet);
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_start() failed\n");
    test_fail(test_string);
  }

  naive_matrix_multiply(1);

  retval = PAPI_stop( EventSet, values );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_stop() failed\n");
    test_fail(test_string);
  }

  if (!quiet) {

    for(i=0;i<max;i++) {
      printf("\t%s: %lld\n",events[i],values[i]);
    }
    printf("\tCycle oflos (100k) %d\n",
	   total);
  }


  /* no MPX OFLO */

  if (!quiet) printf("no mpx, oflo\n");

  retval = PAPI_cleanup_eventset( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_destroy_eventset( &EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_destroy_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_assign_eventset_component( EventSet, 0 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_assign_eventset_component() failed\n");
    test_fail(test_string);
  }

    retval=PAPI_event_name_to_code(events[0],&event);
  retval = PAPI_add_event( EventSet, event );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  max=6;
  for(i=1;i<6;i++) {

    retval=PAPI_event_name_to_code(events[i],&event);
    if (retval!=PAPI_OK) {
      if (!quiet) printf( "PAPI_event_name_to_code() failed %s\n",events[i]);
      test_fail(test_string);
    }

     retval = PAPI_add_event( EventSet, event );
     if ( retval != PAPI_OK ) {
       //       if (!quiet) printf( "PAPI_add_event() failed %d %s\n",i,
       //		   PAPI_descr_error(retval));
       // test_fail(test_string);
       max=i;
       break;
     }
  }

  retval=PAPI_event_name_to_code(events[0],&event);
  retval = PAPI_overflow( EventSet, event, 100000, 0, handler );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_overflow() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_start(EventSet);
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_start() failed\n");
    test_fail(test_string);
  }

  naive_matrix_multiply(1);

  retval = PAPI_stop( EventSet, values );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_stop() failed\n");
    test_fail(test_string);
  }

  if (!quiet) {

    for(i=0;i<max;i++) {
      printf("\t%s: %lld\n",events[i],values[i]);
    }
    printf("\tCycle oflos (100k) %d\n",
	   total);
  }




  /* MPX OFLO */

  total=0;

  if (!quiet) printf("mpx, oflo\n");

  retval = PAPI_cleanup_eventset( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }


  retval = PAPI_destroy_eventset( &EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_destroy_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_assign_eventset_component( EventSet, 0 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_assign_eventset_component() failed\n");
    test_fail(test_string);
  }

    retval=PAPI_event_name_to_code(events[0],&event);
  retval = PAPI_add_event( EventSet, event);
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_set_multiplex( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_set_multiplex() failed\n");
    test_fail(test_string);
  }

  for(i=1;i<6;i++) {

    retval=PAPI_event_name_to_code(events[i],&event);
    if (retval!=PAPI_OK) {
      if (!quiet) printf( "PAPI_event_name_to_code() failed %s\n",events[i]);
      test_fail(test_string);
    }

     retval = PAPI_add_event( EventSet, event );
     if ( retval != PAPI_OK ) {
       if (!quiet) printf( "PAPI_add_event() failed %d %s\n",i,
			   PAPI_descr_error(retval));
        test_fail(test_string);
     }
  }

  retval=PAPI_event_name_to_code(events[0],&event);
  retval = PAPI_overflow( EventSet, event, 100000, 0, handler );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_overflow() failed.  This is expected when using SW mpx\n");
  }

  retval = PAPI_start(EventSet);
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_start() failed\n");
    test_fail(test_string);
  }

  naive_matrix_multiply(1);

  retval = PAPI_stop( EventSet, values );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_stop() failed\n");
    test_fail(test_string);
  }

  if (!quiet) {

    for(i=0;i<6;i++) {
      printf("\t%s: %lld\n",events[i],values[i]);
    }
    printf("\tCycle oflos (100k) %d\n",
	   total);
  }


  PAPI_shutdown(  );


  test_pass(test_string);

  return 0;
}
