#define NUM_EVENTS 100000

struct event_data_t {
	int active;
	int fd;
	struct perf_event_attr attr;
	pid_t pid;
	int cpu;
	int group_fd;
	unsigned long flags;
	int read_size;
	int number_in_group;
	struct sigaction sa;
	char *mmap;
	int mmap_size;
	int last_refresh;
	int overflows;
	int throttles;
};

extern struct event_data_t event_data[NUM_EVENTS];

int find_random_active_event(void);
int find_random_active_sampling_event(void);
