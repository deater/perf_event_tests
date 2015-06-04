long long perf_mmap_read(int which);
void trash_random_mmap(void);
int setup_mmap(int which);
void mmap_random_event(int type);
void unmap_mmap(int i,int from_sigio);
int setup_mmap_aux(int which_fd, int which_mmap);
