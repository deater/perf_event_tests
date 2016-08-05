#define RAW_NONE	0
#define RAW_IBS_FETCH	1
#define RAW_IBS_OP	2

struct validate_values {
	int pid;
	int tid;
	int events;
	unsigned long branch_low;
	unsigned long branch_high;
};

long long perf_mmap_read(
		void *our_mmap, int mmap_size, long long prev_head,
		int sample_type, int read_format, long long reg_mask,
		struct validate_values *validate,
		int quiet, int *events_read,
		int raw_type );


