/* This code tests what happens if you unregister threads before */
/* destroying eventsets.                                         */
/* Prior to PAPI 4.2 this leaked memory, as could be seen if     */
/* PAPI is compiled with --with-debug and PAPI_DEBUG=LEAK        */

/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu           */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "papi.h"

#include "test_utils.h"

int main(int argc, char **argv) {

  int retval,quiet;
  int EventSet  = PAPI_NULL;
  int EventSet2 = PAPI_NULL;
  int EventSet3 = PAPI_NULL;
  int EventSet4 = PAPI_NULL;
  int EventSet5 = PAPI_NULL;
  int EventSet6 = PAPI_NULL;

  char test_string[]="Testing for leak of eventsets after unregister_thread...";

  quiet=test_quiet();


  /* Initialize the library */
  retval = PAPI_library_init( PAPI_VER_CURRENT );
  if (retval != PAPI_VER_CURRENT) {
    if (!quiet) printf("Error! PAPI_library_init %d\n",retval);
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

  retval = PAPI_add_event( EventSet, PAPI_TOT_INS );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_cleanup_eventset( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }

  /**************************************************************/
  /* Checking what happens if you double cleanup                */
  /* This isn't an error on any known PAPI, just a sanity check */
  /**************************************************************/

  if (!quiet) printf("Testing double clean...\n");
  retval = PAPI_cleanup_eventset( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_cleanup_eventset( EventSet );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }

  /****************************************************/
  /* check for memleak on unregister if we do destroy */
  /****************************************************/

  if (!quiet) printf("Testing cleanup/destroy/unregister...\n");

  retval = PAPI_register_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_register_thread() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet4 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_add_event( EventSet4, PAPI_TOT_INS );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_cleanup_eventset( EventSet4 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }


  retval = PAPI_destroy_eventset( &EventSet4 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_destroy_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_unregister_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_unregister_thread() failed\n");
    test_fail(test_string);
  }

  /********************************************************************/
  /* check for memleak on unregister if we cleanup but do not destroy */
  /********************************************************************/

  if (!quiet) printf("Testing cleanup/unregister...\n");

  retval = PAPI_register_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_register_thread() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet2 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_add_event( EventSet2, PAPI_TOT_INS );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_cleanup_eventset( EventSet2 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_cleanup_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_unregister_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_unregister_thread() failed\n");
    test_fail(test_string);
  }

  /****************************************************/
  /* check for memleak on unregister if we do nothing */
  /****************************************************/

  if (!quiet) printf("Testing unregister...\n");

  retval = PAPI_register_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_register_thread() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet3 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_add_event( EventSet3, PAPI_TOT_INS );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_unregister_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_unregister_thread() failed\n");
    test_fail(test_string);
  }


  /****************************************************/
  /* check if we unregister a running count           */
  /****************************************************/

  if (!quiet) printf("Testing unregister while running...\n");

  retval = PAPI_register_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_register_thread() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet5 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_add_event( EventSet5, PAPI_TOT_INS );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_start( EventSet5);
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_start() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_unregister_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_unregister_thread() failed\n");
    test_fail(test_string);
  }

  /****************************************************/
  /* check if we unregister twice                     */
  /****************************************************/

  if (!quiet) printf("Testing unregister twice...\n");

  retval = PAPI_register_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_register_thread() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_create_eventset( &EventSet6 );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_create_eventset() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_add_event( EventSet6, PAPI_TOT_INS );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_add_event() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_unregister_thread(  );
  if ( retval != PAPI_OK ) {
    if (!quiet) printf( "PAPI_unregister_thread() failed\n");
    test_fail(test_string);
  }

  retval = PAPI_unregister_thread(  );
  if ( retval == PAPI_OK ) {
    if (!quiet) printf( "PAPI_unregister_thread() twice succeded\n");
    test_fail(test_string);
  }


  PAPI_shutdown(  );

  test_pass(test_string);

  return 0;
}
