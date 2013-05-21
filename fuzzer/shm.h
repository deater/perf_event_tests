#define MAX_NR_CHILDREN 64

struct shm_s {
	unsigned long a1[MAX_NR_CHILDREN];
	unsigned long a2[MAX_NR_CHILDREN];
	unsigned long a3[MAX_NR_CHILDREN];
	unsigned long a4[MAX_NR_CHILDREN];
	unsigned long a5[MAX_NR_CHILDREN];
	unsigned long a6[MAX_NR_CHILDREN];

	unsigned int seed;
	unsigned int seeds[MAX_NR_CHILDREN];
	unsigned int reseed_counter;
        int need_reseed;

};
extern struct shm_s *shm;

