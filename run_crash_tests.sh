#!/bin/sh

echo
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "            WARNING               "
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo
echo "These tests can crash your kernel!"
echo "Only run if you have properly patched kernels!"
echo "Be sure to sync your disk first too."
echo
echo "Waiting 5 seconds before running to give you "
echo "time to change your mind..."
echo
sleep 5

./crashes/arm_map_hw_event_oops
./crashes/arm_validate_event_oops
./crashes/pe_task_schedule_panic
./crashes/pe_inherit_memleak
./crashes/oflo_sw_cpu_clock_crash
./crashes/sw_event_config_overflow
./crashes/offcore_response_mask
./crashes/perf_mmap_close_bug
# Run twice, sometimes takes two runs
./crashes/nmi_soft_lockup
./crashes/nmi_soft_lockup
# Run twice, sometimes takes two runs
./crashes/perf_ftrace_event_crash
./crashes/perf_ftrace_event_crash
# Run twice, sometimes takes two runs
./crashes/tracepoint_irq_work_exit
./crashes/tracepoint_irq_work_exit
./crashes/css_cgroup_crash
./crashes/rapl_crash

# Also run the warnings tests
./warnings/core2_nmi_flood
./warnings/kec_2122_warn
./warnings/pec_1076_warn
