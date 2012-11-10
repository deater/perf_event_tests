struct validate_values {
  int pid;
  int tid;
  int events;
};

long long perf_mmap_read( void *our_mmap, int mmap_size, long long prev_head,
		    int sample_type, int read_format, 
		    struct validate_values *validate,
		    int quiet );


