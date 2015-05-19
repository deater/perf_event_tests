#define LOG_FAILURES	0
#define FSYNC_EVERY	0

#define TYPE_ALL		0xffffffff
#define TYPE_MMAP		0x0001
#define TYPE_OVERFLOW		0x0002
#define TYPE_OPEN		0x0004
#define TYPE_CLOSE		0x0008
#define TYPE_READ		0x0010
#define TYPE_WRITE		0x0020
#define TYPE_IOCTL		0x0040
#define TYPE_FORK		0x0080
/* missing */
#define TYPE_PRCTL		0x0200
#define TYPE_POLL		0x0400
#define TYPE_MILLION		0x0800
#define TYPE_ACCESS		0x1000
#define TYPE_TRASH_MMAP		0x2000

extern int trigger_failure_logging;
extern int logging;
extern int stop_after;

extern int log_fd;
extern char log_buffer[BUFSIZ];



