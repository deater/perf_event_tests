/* This test runs rdpmc() without enabling it via perf_event_open()/mmap() */

/* It's not a validation test, but just a utility for experimenting with */
/* rdpmc behavior */

/* Try messing with /sys/devices/cpu/rdpmc */
/* 0 should be disabled for all */
/* 1 should be per-process enabled if perf_event_open() followed by mmap() */
/* 2 should be enabled for all */

#include <stdio.h>


inline unsigned long long rdpmc(unsigned int counter) {

        unsigned int low, high;

        __asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

        return (unsigned long long)low | ((unsigned long long)high) <<32;
}


int main(int argc, char **argv) {


	printf("%lld\n",rdpmc(0));

	return 0;
}
