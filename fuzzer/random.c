/*
 * Routines to get randomness/set seeds.
 */
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "shm.h"
#include "sanitise.h"

extern int user_set_seed;

#define TRUE 1
#define FALSE 0

/* The actual seed lives in the shm. This variable is used
 * to store what gets passed in from the command line -s argument */
unsigned int seed = 0;

unsigned int new_seed(void)
{
	struct timeval t;
	unsigned int r;

	r = rand();
	if (!(rand() % 2)) {
		gettimeofday(&t, 0);
		r |= t.tv_usec;
	}
	return r;
}

/*
 * If we passed in a seed with -s, use that. Otherwise make one up from time of day.
 */
unsigned int init_seed(unsigned int seedparam)
{
	if (user_set_seed == TRUE)
		printf("[%d] Using user passed random seed: %u\n", getpid(), seedparam);
	else {
		seedparam = new_seed();

		printf("Initial random seed from time of day: %u\n", seedparam);
	}

	return seedparam;
}

/* Mix in the pidslot so that all children get different randomness.
 * we can't use the actual pid or anything else 'random' because otherwise reproducing
 * seeds with -s would be much harder to replicate.
 */
void set_seed(unsigned int pidslot)
{
	srand(shm->seed + (pidslot + 1));
	shm->seeds[pidslot] = shm->seed;
}

/*
 * Periodically reseed.
 *
 * We do this so we can log a new seed every now and again, so we can cut down on the
 * amount of time necessary to reproduce a bug.
 * Caveat: Not used if we passed in our own seed with -s
 */
void reseed(void)
{
	shm->need_reseed = FALSE;
	shm->reseed_counter = 0;

	/* don't change the seed if we passed -s */
	if (user_set_seed == TRUE)
		return;

	/* We are reseeding. */
	shm->seed = new_seed();

	fprintf(stderr,"[%d] Random reseed: %u\n", getpid(), shm->seed);

}

unsigned long rand64(void)
{
	unsigned long r = 0;

	switch (rand() % 5) {

	/* Sometimes pick a not-so-random number. */
	case 0:	return get_interesting_value();

	/* limit to RAND_MAX (31 bits) */
	case 1:	r = rand();
		break;

	 /* do some gymnastics here to get > RAND_MAX
	  * Based on very similar routine stolen from iknowthis. Thanks Tavis.
	  */
	case 2:
		r = rand() & rand();
#if __WORDSIZE == 64
		r <<= 32;
		r |= rand() & rand();
#endif
		break;

	case 3:
		r = rand() | rand();
#if __WORDSIZE == 64
		r <<= 32;
		r |= rand() | rand();
#endif
		break;

	case 4:
		r = rand();
#if __WORDSIZE == 64
		r <<= 32;
		r |= rand();
#endif
		break;

	default:
		break;
	}
	return r;
}
