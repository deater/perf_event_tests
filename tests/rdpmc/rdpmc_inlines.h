#if defined(__i386__) || defined (__x86_64__)

#define barrier() __asm__ volatile("" ::: "memory")

#elif defined(__aarch64__)

#define barrier()	asm volatile("dmb ish" : : : "memory")
#define isb()		asm volatile("isb" : : : "memory")

#else

#define barrier()

#endif


#if defined(__i386__) || defined (__x86_64__)

inline unsigned long long rdtsc(void) {

	unsigned a,d;

	__asm__ volatile("rdtsc" : "=a" (a), "=d" (d));

	return ((unsigned long long)a) | (((unsigned long long)d) << 32);
}

inline unsigned long long rdpmc(unsigned int counter) {

	unsigned int low, high;

	__asm__ volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (counter));

	return (unsigned long long)low | ((unsigned long long)high) <<32;
}

#elif defined(__aarch64__)


inline unsigned long long rdtsc(void) {

	return 0;

}

inline unsigned long long rdpmc(unsigned int counter) {

	unsigned long long ret=0;

	if (counter == PERF_COUNT_HW_CPU_CYCLES) {
		asm volatile("mrs %0, pmccntr_el0" : "=r" (ret));
	}
	else {
		asm volatile("msr pmselr_el0, %0" : : "r" ((counter-1)));
		asm volatile("mrs %0, pmxevcntr_el0" : "=r" (ret));
	}

	isb();


	return ret;
}

#else


inline unsigned long long rdtsc(void) {

	return 0;

}

inline unsigned long long rdpmc(unsigned int counter) {

	return 0;
}

#endif
