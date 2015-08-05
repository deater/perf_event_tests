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

#else

inline unsigned long long rdtsc(void) {

	return 0;

}

inline unsigned long long rdpmc(unsigned int counter) {

	return 0;
}

#endif
