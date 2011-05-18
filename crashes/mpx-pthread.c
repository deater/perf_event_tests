#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <pthread.h>

#include <string.h>

#include <papi.h>

double loop(int n) {

    int i;
    double a = 0.0012;

    for (i = 0; i < n; i++) {
      a += 0.01;
    }
    return a;
}

void *thread(void *arg) {

    int eventset;
    long long values[PAPI_MPX_DEF_DEG];
    int events[PAPI_MPX_DEF_DEG], numevents = 0;

    events[0] = PAPI_TOT_CYC;
    events[1] = PAPI_TOT_INS;
    numevents = 2;

    eventset = PAPI_NULL;

    PAPI_create_eventset(&eventset);

    PAPI_add_events(eventset, events, numevents);

    //PAPI_start(eventset);

    loop(10000000);

    //PAPI_stop(eventset, values);

    PAPI_cleanup_eventset(eventset);

    //PAPI_destroy_eventset(&eventset);

    pthread_exit(NULL);
}


int main(int argc, char **argv) {

    int nthreads=1, ret, i;
    pthread_t *threads;

    nthreads=16;

    PAPI_library_init(PAPI_VER_CURRENT);
    PAPI_thread_init(pthread_self);

    printf("Creating %d threads:\n", nthreads);

    threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t));
    if ( threads == NULL ) {
        fputs("Error allocating thread memory.\n", stderr);
        exit(1);
    }

    /* Create the threads */
    for (i = 0; i < nthreads; i++) {
        ret = pthread_create(&threads[i], NULL, thread, NULL);
        if ( ret != 0 ) {
            fprintf(stderr, "Reached thread creation limit of %d.\n", i);
            exit(0);
        }
    }

    /* Wait for thread completion */
    for (i = 0; i < nthreads; i++) {
        ret = pthread_join(threads[i],NULL);
        if ( ret != 0 ) {
            fprintf(stderr, "Error joining with thread %d.\n", i);
            exit(1);
        }
    }

    return 0;
}

