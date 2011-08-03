int perf_event_open(struct perf_event_attr *hw_event_uptr,
		    pid_t pid, int cpu, int group_fd, unsigned long flags);
int detect_processor(void);
void arch_adjust_domain(struct perf_event_attr *pe,int quiet);

#define PROCESSOR_UNKNOWN     -1
#define PROCESSOR_PENTIUM_PRO  1
#define PROCESSOR_PENTIUM_II   2
#define PROCESSOR_PENTIUM_III  3
#define PROCESSOR_PENTIUM_4    4
#define PROCESSOR_PENTIUM_M    5
#define PROCESSOR_COREDUO      6
#define PROCESSOR_CORE2        7
#define PROCESSOR_NEHALEM      8
#define PROCESSOR_NEHALEM_EX   9
#define PROCESSOR_WESTMERE    10
#define PROCESSOR_WESTMERE_EX 11
#define PROCESSOR_SANDYBRIDGE 12
#define PROCESSOR_ATOM        13
#define PROCESSOR_K7          14
#define PROCESSOR_K8          15
#define PROCESSOR_AMD_FAM10H  16
#define PROCESSOR_AMD_FAM11H  17
#define PROCESSOR_AMD_FAM14H  18
#define PROCESSOR_AMD_FAM15H  19

