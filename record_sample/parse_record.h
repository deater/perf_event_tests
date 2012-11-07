struct validate_values {
  int pid;
  int tid;
};

int perf_mmap_read( void *our_mmap, int sample_type, 
		    int read_format, 
		    struct validate_values *validate,
		    int quiet );


