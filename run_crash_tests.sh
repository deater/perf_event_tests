#!/bin/sh

cd $(dirname $0)
PERF_EVENT_ROOT=${PWD}

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

${PERF_EVENT_ROOT}/crashes/arm_map_hw_event_oops
${PERF_EVENT_ROOT}/crashes/arm_validate_event_oops
${PERF_EVENT_ROOT}/crashes/pe_task_schedule_panic
${PERF_EVENT_ROOT}/crashes/pe_inherit_memleak
${PERF_EVENT_ROOT}/crashes/oflo_sw_cpu_clock_crash
${PERF_EVENT_ROOT}/crashes/sw_event_config_overflow
${PERF_EVENT_ROOT}/crashes/offcore_response_mask
${PERF_EVENT_ROOT}/crashes/perf_mmap_close_bug
# Run twice, sometimes takes two runs
${PERF_EVENT_ROOT}/crashes/nmi_soft_lockup
${PERF_EVENT_ROOT}/crashes/nmi_soft_lockup
# Run twice, sometimes takes two runs
${PERF_EVENT_ROOT}/crashes/perf_ftrace_event_crash
${PERF_EVENT_ROOT}/crashes/perf_ftrace_event_crash
# Run twice, sometimes takes two runs
${PERF_EVENT_ROOT}/crashes/tracepoint_irq_work_exit
${PERF_EVENT_ROOT}/crashes/tracepoint_irq_work_exit
${PERF_EVENT_ROOT}/crashes/css_cgroup_crash
${PERF_EVENT_ROOT}/crashes/rapl_crash

# Also run the warnings tests
${PERF_EVENT_ROOT}/warnings/core2_nmi_flood
${PERF_EVENT_ROOT}/warnings/kec_2122_warn
${PERF_EVENT_ROOT}/warnings/pec_1076_warn
