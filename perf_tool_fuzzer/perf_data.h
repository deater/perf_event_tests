struct perf_file_section {
	uint64_t offset;	/* offset from start of file */
	uint64_t size;		/* size of the section */
};

struct perf_header {
	char magic[8];		/* PERFILE2 */
	uint64_t size;		/* size of the header */
	uint64_t attr_size;	/* size of an attribute in attrs */
	struct perf_file_section attrs;
	struct perf_file_section data;
	struct perf_file_section event_types;
	uint64_t flags;
	uint64_t flags1[3];
};

#define	HEADER_RESERVED		0
#define	HEADER_TRACING_DATA	1
#define	HEADER_BUILD_ID		2
#define	HEADER_HOSTNAME		3
#define	HEADER_OSRELEASE	4
#define	HEADER_VERSION		5
#define	HEADER_ARCH		6
#define	HEADER_NRCPUS		7
#define	HEADER_CPUDESC		8
#define	HEADER_CPUID		9
#define	HEADER_TOTAL_MEM	10
#define	HEADER_CMDLINE		11
#define	HEADER_EVENT_DESC	12
#define	HEADER_CPU_TOPOLOGY	13
#define	HEADER_NUMA_TOPOLOGY	14
#define	HEADER_BRANCH_STACK	15
#define	HEADER_PMU_MAPPINGS	16
#define	HEADER_GROUP_DESC	17
#define	HEADER_AUXTRACE		18
#define	HEADER_STAT		19
#define	HEADER_CACHE		20
#define	HEADER_SAMPLE_TIME	21
#define	HEADER_SAMPLE_TOPOLOGY	22
#define	HEADER_CLOCKID		23
#define	HEADER_DIR_FORMAT	24
#define	HEADER_BPF_PROG_INFO	25
#define	HEADER_BPF_BTF		26
#define	HEADER_COMPRESSED	27

