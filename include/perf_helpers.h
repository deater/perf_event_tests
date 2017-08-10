int perf_event_open(struct perf_event_attr *hw_event_uptr,
		    pid_t pid, int cpu, int group_fd, unsigned long flags);
int detect_processor(void);
int detect_vendor(void);
void arch_adjust_domain(struct perf_event_attr *pe,int quiet);
int copy_events(int *eventset);
int detect_nmi_watchdog(void);
pid_t mygettid( void );
int get_kernel_version(void);
int get_latency_load_event(unsigned long long *config,
			unsigned long long *config1,
			int *precise_ip,
			char *name);
int get_latency_store_event(unsigned long long *config,
			unsigned long long *config1,
			int *precise_ip,
			char *name);
int detect_architecture(void);

#define ARCH_UNKNOWN    -1
#define ARCH_X86        1
#define ARCH_X86_64     2
#define ARCH_POWER      3
#define ARCH_ARM        4
#define ARCH_ARM64      5

#define VENDOR_UNKNOWN -1
#define VENDOR_INTEL    1
#define VENDOR_AMD      2
#define VENDOR_IBM	3
#define VENDOR_ARM	4

#define PROCESSOR_UNKNOWN		-1
#define PROCESSOR_PENTIUM_PRO		1
#define PROCESSOR_PENTIUM_II		2
#define PROCESSOR_PENTIUM_III		3
#define PROCESSOR_PENTIUM_4		4
#define PROCESSOR_PENTIUM_M		5
#define PROCESSOR_COREDUO		6
#define PROCESSOR_CORE2			7
#define PROCESSOR_NEHALEM		8
#define PROCESSOR_NEHALEM_EX		9
#define PROCESSOR_WESTMERE		10
#define PROCESSOR_WESTMERE_EX		11
#define PROCESSOR_SANDYBRIDGE		12
#define PROCESSOR_ATOM			13
#define PROCESSOR_K7			14
#define PROCESSOR_K8			15
#define PROCESSOR_AMD_FAM10H		16
#define PROCESSOR_AMD_FAM11H		17
#define PROCESSOR_AMD_FAM14H		18
#define PROCESSOR_AMD_FAM15H		19
#define PROCESSOR_IVYBRIDGE		20
#define PROCESSOR_KNIGHTSCORNER		21
#define PROCESSOR_SANDYBRIDGE_EP	22
#define PROCESSOR_AMD_FAM16H		23
#define PROCESSOR_IVYBRIDGE_EP		24
#define PROCESSOR_HASWELL		25
#define PROCESSOR_ATOM_CEDARVIEW	26
#define PROCESSOR_ATOM_SILVERMONT	27
#define PROCESSOR_BROADWELL		28
#define PROCESSOR_HASWELL_EP		29
#define PROCESSOR_SKYLAKE		30
#define PROCESSOR_AMD_FAM17H		31
#define PROCESSOR_KABYLAKE		32
#define PROCESSOR_AMD_FAM12H		33
#define PROCESSOR_AMD_FAM13H		34



#define PROCESSOR_POWER3      103
#define PROCESSOR_POWER4      104
#define PROCESSOR_POWER5      105
#define PROCESSOR_POWER6      106
#define PROCESSOR_POWER7      107

#define PROCESSOR_CORTEX_A8	200
#define PROCESSOR_CORTEX_A9	201
#define PROCESSOR_CORTEX_A5	202
#define PROCESSOR_CORTEX_A15	203
#define PROCESSOR_ARM1176	204
#define PROCESSOR_CORTEX_A7	205
#define PROCESSOR_CORTEX_A57	206
#define PROCESSOR_CORTEX_A53	207

#define MAX_TEST_EVENTS 16



