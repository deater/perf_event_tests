/************************ BEGIN **************************/

#ifndef __KERNEL__
#include <linux/version.h>
#include "bpf.h"
#define __NR_CPUS__ 8
typedef int u32;
#else
#include "/home/vince/research/linux-kernel/linux-git/include/uapi/linux/bpf.h"
#endif



typedef unsigned long long u64;

struct bpf_map_def {
	unsigned int type;
	unsigned int key_size;
	unsigned int value_size;
	unsigned int max_entries;
};

#define SEC(NAME) __attribute__((section(NAME), used))
static u64 (*ktime_get_ns)(void) =
	(void *)BPF_FUNC_ktime_get_ns;
static int (*trace_printk)(const char *fmt, int fmt_size, ...) =
	(void *)BPF_FUNC_trace_printk;
static int (*get_smp_processor_id)(void) =
	(void *)BPF_FUNC_get_smp_processor_id;
      static int (*perf_event_output)(void *, struct bpf_map_def *, int, void *, unsigned long) =
            (void *)BPF_FUNC_perf_event_output;
    
      struct bpf_map_def SEC("maps") channel = {
            .type = BPF_MAP_TYPE_PERF_EVENT_ARRAY,
            .key_size = sizeof(int),
            .value_size = sizeof(u32),
            .max_entries = __NR_CPUS__,
      };
    
      SEC("func_write=sys_write")
      int func_write(void *ctx)
      {
            struct {
                    u64 ktime;
                    int cpuid;
            } __attribute__((packed)) output_data;
            char error_data[] = "Error: failed to output: %d\n";
    
            output_data.cpuid = get_smp_processor_id();
            output_data.ktime = ktime_get_ns();
            int err = perf_event_output(ctx, &channel, get_smp_processor_id(),
                                        &output_data, sizeof(output_data));
            if (err)
                    trace_printk(error_data, sizeof(error_data), err);
            return 0;
      }
      char _license[] SEC("license") = "GPL";
      int _version SEC("version") = LINUX_VERSION_CODE;
      /************************ END ***************************/
