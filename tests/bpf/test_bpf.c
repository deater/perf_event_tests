#include "linux/bpf.h"
#include <linux/version.h>
#include <linux/ptrace.h>
#include <stdint.h>

#define SEC(NAME) __attribute__((section(NAME), used))

static int (*bpf_map_delete_elem)(void *map, void *key) =
	(void *) BPF_FUNC_map_delete_elem;
static int (*bpf_trace_printk)(const char *fmt, int fmt_size, ...) =
	(void *) BPF_FUNC_trace_printk;

struct bpf_map_def {
	unsigned int type;
	unsigned int key_size;
	unsigned int value_size;
	unsigned int max_entries;
};

struct pair {
	uint64_t val;
	uint64_t ip;
};

struct bpf_map_def SEC("maps") my_map = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(long),
	.value_size = sizeof(struct pair),
	.max_entries = 1000000,
};

struct bpf_map_def SEC("maps") my_map2 = {
	.type = BPF_MAP_TYPE_HASH,
	.key_size = sizeof(long),
	.value_size = sizeof(struct pair),
	.max_entries = 1000000,
};

SEC("cache_free=kmem_cache_free%return")
int bpf_prog1(struct pt_regs *ctx)
{
	long ptr = ctx->r14;
	bpf_map_delete_elem(&my_map2, &ptr);
	return 0;
}

SEC("mybpfprog=__alloc_pages_nodemask")
int bpf_prog_my(struct pt_regs *ctx)
{
	char fmt[] = "Haha\n";

	long ptr = ctx->r14;
	bpf_trace_printk(fmt, sizeof(fmt));
	bpf_map_delete_elem(&my_map, &ptr);
	return 0;
}

char _license[] SEC("license") = "GPL";
uint32_t _version SEC("version") = LINUX_VERSION_CODE;
