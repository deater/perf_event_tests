unsigned long long mmap_read_self(void *addr,
					 unsigned long long *enabled,
					 unsigned long long *running);
int detect_rdpmc(int quiet);

extern unsigned long long rdtsc(void);
extern unsigned long long rdpmc(unsigned int counter);

#ifdef __aarch64__
#define ARCH_EVENT_CONFIG1_VAL 0x2
#else
#define ARCH_EVENT_CONFIG1_VAL 0
#endif
