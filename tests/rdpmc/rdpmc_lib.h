inline unsigned long long rdtsc(void);
inline unsigned long long rdpmc(unsigned int counter);
unsigned long long mmap_read_self(void *addr,
					 unsigned long long *enabled,
					 unsigned long long *running);
int detect_rdpmc(int quiet);
