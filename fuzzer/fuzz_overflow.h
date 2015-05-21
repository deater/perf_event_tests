#define MAX_THROTTLES		10

void our_handler(int signum, siginfo_t *info, void *uc);
void sigio_handler(int signum, siginfo_t *info, void *uc);
void orderly_shutdown(void);
