struct fuzzer_stats_t {
	long long total_iterations;
	long long overflows;
	long long sigios;
	long long current_open;
	long long open_attempts,open_successful;
	long long close_attempts,close_successful;
	long long mmap_attempts,mmap_successful;
	long long read_attempts,read_successful;
	long long write_attempts,writes_successful;
	long long ioctl_attempts,ioctl_successful;
	long long prctl_attempts,prctl_successful;
	long long fork_attempts,fork_successful;
	long long poll_attempts,poll_successful;
	long long access_attempts,access_successful;
	long long trash_mmap_attempts,trash_mmap_successful;
};

extern struct fuzzer_stats_t stats;
