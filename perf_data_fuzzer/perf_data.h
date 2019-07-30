struct our_perf_event_header {
	uint32_t type;
	uint16_t misc;
	uint16_t size;
};

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

/* Note, BUILD_ID_SIZE is 20, but this is aligned to a 64-bit boundary */
/* so 24 */


#define BUILD_ID_SIZE	24

/* ugh, the size of filename varies, as it can be as big as the size */
/* of the largest existing headersize-rest of the padding, and this */
/* is all in a union */

struct build_id_event {
	struct our_perf_event_header header;
	pid_t pid;
	uint8_t	build_id[BUILD_ID_SIZE];
	char filename[200-36];
};

struct nr_cpus {
	int32_t nr_cpus_available; /* CPUs not yet onlined */
	uint32_t nr_cpus_online;
};

struct perf_header_string {
	uint32_t len;
	char string[]; /* zero terminated */
};

struct perf_header_string_list {
	uint32_t nr;
	struct perf_header_string strings[]; /* variable length records */
};


struct perf_header_event_desc_events {
	struct perf_event_attr attr;  /* size of attr_size */
	uint32_t nr_ids;
	struct perf_header_string event_string;
	uint64_t ids[];	/* nr_ids */
};


struct perf_header_event_desc {
       uint32_t nr; /* number of events */
       uint32_t attr_size; /* size of each perf_event_attr */
       struct perf_header_event_desc_events events[];  /* Variable length records */
};

struct perf_header_cpu_topology_cpus {
	uint32_t core_id;
	uint32_t socket_id;
};

struct perf_header_cpu_topology {

	struct perf_header_string_list cores; /* Variable length */
	struct perf_header_string_list threads; /* Variable length */

	/* Second revision of HEADER_CPU_TOPOLOGY also has the following */
	/* 'nr' comes from previously processed HEADER_NRCPUS's nr_cpu_avail */
	struct perf_header_cpu_topology_cpus cpus[];
};

        /* Third revision of HEADER_CPU_TOPOLOGY also has the following */
struct perf_header_cpu_topology_pt3 {
	struct perf_header_string_list dies; /* Variable length */
	uint32_t die_id[]; /* from previously processed HEADER_NR_CPUS, VLA */
};

struct perf_header_numa_topology_nodes {
	uint32_t nodenr;
	uint64_t mem_total;
	uint64_t mem_free;
	struct perf_header_string cpus;
} __attribute((packed));

struct perf_header_numa_topology {
       uint32_t nr;
       struct perf_header_numa_topology_nodes nodes[];
};

struct perf_header_pmu {
	uint32_t pmu_type;
	struct perf_header_string pmu_name;
};

struct perf_header_pmu_mapping {
	uint32_t nr;
	struct perf_header_pmu pmu[];
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

