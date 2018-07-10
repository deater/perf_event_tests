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
	int mmap;
//	char *mmap;
//	int mmap_size;
	int last_refresh;
	int overflows;
	int throttles;
};

extern struct event_data_t event_data[NUM_EVENTS];
extern int active_events;

int find_random_active_event(void);
int find_random_active_sampling_event(void);
int find_random_active_breakpoint_event(void);
int find_empty_event(void);
int lookup_event(int fd);
