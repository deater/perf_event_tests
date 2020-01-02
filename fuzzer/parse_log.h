int parse_open_event(char *line,
	int *orig_fd, pid_t *pid, int *cpu, int *group_fd, long int *flags,
	struct perf_event_attr *pe);

