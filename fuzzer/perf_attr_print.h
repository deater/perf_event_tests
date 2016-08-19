void perf_pretty_print_mmap_prot(int prot);
void perf_pretty_print_flags(FILE *fff,long int flags);
void perf_pretty_print_mmap_flags(int flags);
//static void perf_pretty_print_type(int type);
//static void perf_pretty_print_cache(long long config);
//static void perf_pretty_print_config(long long type, long long config);
//static void perf_pretty_print_sample_type(unsigned long long sample_type);
//static void perf_pretty_print_read_format(unsigned long long read_format);
//static void perf_pretty_print_breakpoint_type(int bp_type);
void perf_pretty_print_attr(FILE *fff,struct perf_event_attr *pe, int fd);
void perf_pretty_print_event(FILE *fff,int fd, int original_pid,
				struct perf_event_attr *pe,
				pid_t pid, int cpu,
				int group_fd, unsigned long flags);
void perf_pretty_print_event_short(FILE *fff,int fd, int original_pid,
				struct perf_event_attr *pe,
				pid_t pid, int cpu,
				int group_fd, unsigned long flags);
