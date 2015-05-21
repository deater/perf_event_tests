extern int attempt_determinism;

/* Try to allow minimal determinism while at the same	*/
/* time skipping unnecessary system calls.		*/
#if 0
/* Minimal syscalls needed for bug I am tracking */
static int ignore_but_dont_skip_mmap=1;
static int ignore_but_dont_skip_overflow=1;
static int ignore_but_dont_skip_open=0;
static int ignore_but_dont_skip_close=0;
static int ignore_but_dont_skip_read=1;
static int ignore_but_dont_skip_write=1;
static int ignore_but_dont_skip_ioctl=1;
static int ignore_but_dont_skip_fork=0;
static int ignore_but_dont_skip_prctl=1;
static int ignore_but_dont_skip_poll=1;
static int ignore_but_dont_skip_million=1;
static int ignore_but_dont_skip_access=1;
static int ignore_but_dont_skip_trash_mmap=1;
#endif

struct skip_t {
	int mmap;
	int overflow;
	int open;
	int close;
	int read;
	int write;
	int ioctl;
	int fork;
	int prctl;
	int poll;
	int million;
	int access;
	int trash_mmap;
};

extern struct skip_t ignore_but_dont_skip;
