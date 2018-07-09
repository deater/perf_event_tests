#!/bin/sh

cd $(dirname $0)
PERF_EVENT_ROOT=${PWD}

export TESTS_QUIET=y
export TESTS_DIR=${PERF_EVENT_ROOT}/tests

echo
echo "**** RUNNING perf_event_tests ****"
uname -a

echo
echo "* Testing /proc/sys/kernel/perf_event_paranoid setting"
echo "  + tests/utils/check_paranoid"
echo -n "    "
$TESTS_DIR/utils/check_paranoid

if [ "$?" -ne "0" ]; then
	echo
	echo "*** PARANOID VALUE TOO HIGH, EXITING EARLY ***"
	echo
	exit;
fi

echo
echo "* Checking infrastructure"
echo "  + tests/utils/get_cache_info"
echo -n "    "
$TESTS_DIR/utils/get_cache_info
echo " + tests/sysfs/events"
echo -n "    "
$TESTS_DIR/sysfs/events

echo
echo "* Checking generalized events"
echo "  + tests/generalized_events/branches"
echo -n "    "
$TESTS_DIR/generalized_events/branches
echo "  + tests/generalized_events/branch-misses"
echo -n "    "
$TESTS_DIR/generalized_events/branch-misses
echo "  + tests/generalized_events/cycles"
echo -n "    "
$TESTS_DIR/generalized_events/cycles
echo "  + tests/generalized_events/instructions"
echo -n "    "
$TESTS_DIR/generalized_events/instructions
echo "  + tests/generalized_events/l1-dcache-stores"
echo -n "    "
$TESTS_DIR/generalized_events/l1-dcache-stores
echo "  + tests/generalized_events/l1-dcache-loads"
echo -n "    "
$TESTS_DIR/generalized_events/l1-dcache-loads

echo
echo "* Checking Intel x86 specific features"
echo "  + tests/x86_intel/fixed_ctr0"
echo -n "    "
$TESTS_DIR/x86_intel/fixed_ctr0
echo "  + tests/x86_intel/fixed_ctr1"
echo -n "    "
$TESTS_DIR/x86_intel/fixed_ctr1
echo "  + tests/x86_intel/fixed_ctr2"
echo -n "    "
$TESTS_DIR/x86_intel/fixed_ctr2
echo "  + tests/x86_intel/offcore_response"
echo -n "    "
$TESTS_DIR/x86_intel/offcore_response
echo "  + tests/x86_intel/offcore_response_mask"
echo -n "    "
$TESTS_DIR/x86_intel/offcore_response_mask
echo "  + tests/x86_intel/uncore_events"
echo -n "    "
$TESTS_DIR/x86_intel/uncore_events

echo
echo "* Checking breakpoint support"
echo "  + tests/breakpoints/breakpoint_support"
echo -n "    "
$TESTS_DIR/breakpoints/breakpoint_support

echo
echo "* Checking basic perf_event functionality"
echo "  + tests/attr_fields/disabled"
echo -n "    "
$TESTS_DIR/attr_fields/disabled
echo "  + tests/attr_fields/exclusive"
echo -n "    "
$TESTS_DIR/attr_fields/exclusive
echo "  + tests/attr_fields/format_id_support"
echo -n "    "
$TESTS_DIR/attr_fields/format_id_support
echo "  + tests/attr_fields/inherit"
echo -n "    "
$TESTS_DIR/attr_fields/inherit
echo "  + tests/attr_fields/inherit_stat"
echo -n "    "
$TESTS_DIR/attr_fields/inherit_stat
echo "  + tests/attr_fields/pinned"
echo -n "    "
$TESTS_DIR/attr_fields/pinned
echo "  + tests/attr_fields/read_format_range_check"
echo -n "    "
$TESTS_DIR/attr_fields/read_format_range_check
echo "  + tests/attr_fields/sample_type_range_check"
echo -n "    "
$TESTS_DIR/attr_fields/sample_type_range_check
echo "  + tests/attr_fields/exclude_user"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_user
echo "  + tests/attr_fields/exclude_kernel"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_kernel
echo "  + tests/attr_fields/exclude_hv"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_hv
echo "  + tests/attr_fields/exclude_idle"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_idle
echo "  + tests/attr_fields/exclude_guest"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_guest
echo "  + tests/attr_fields/exclude_host"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_host
echo "  + tests/flags/flags_range_check"
echo -n "    "
$TESTS_DIR/flags/flags_range_check
echo "  + tests/flags/flags_cgroup"
echo -n "    "
$TESTS_DIR/flags/flags_cgroup
echo "  + tests/flags/flags_fd_no_group"
echo -n "    "
$TESTS_DIR/flags/flags_fd_no_group
echo "  + tests/flags/flags_fd_output"
echo -n "    "
$TESTS_DIR/flags/flags_fd_output

echo "  + tests/socket/socket_pass"
echo -n "    "
$TESTS_DIR/socket/socket_pass

echo
echo "* Checking attr.size behavior"
echo "  + tests/size/too_big"
echo -n "    "
$TESTS_DIR/size/too_big
echo "  + tests/size/too_little"
echo -n "    "
$TESTS_DIR/size/too_little
echo "  + tests/size/just_right"
echo -n "    "
$TESTS_DIR/size/just_right
echo "  + tests/size/header_mismatch"
echo -n "    "
$TESTS_DIR/size/header_mismatch


echo
echo "* Checking perf_event ioctl calls"
echo "  + tests/ioctl/ioctl_flag_group"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_flag_group
echo "  + tests/ioctl/ioctl_0_enable"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_0_enable
echo "  + tests/ioctl/ioctl_1_disable"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_1_disable
echo "  + tests/ioctl/ioctl_2_refresh"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_2_refresh
echo "  + tests/ioctl/ioctl_3_reset"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_3_reset
echo "  + tests/ioctl/ioctl_4_period"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_4_period
echo "  + tests/ioctl/ioctl_5_set_output"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_5_set_output
echo "  + tests/ioctl/ioctl_6_set_filter"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_6_set_filter
echo "  + tests/ioctl/ioctl_7_id"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_7_id
echo "  + tests/ioctl/ioctl_8_set_bpf"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_8_set_bpf
echo "  + tests/ioctl/ioctl_9_pause_output"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_9_pause_output
echo "  + tests/ioctl/ioctl_10_query_bpf"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_10_query_bpf
echo "  + tests/ioctl/ioctl_11_modify_attributes"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_11_modify_attributes

echo
echo "* Checking perf_event prctl calls"
echo "  + tests/prctl/prctl"
echo -n "    "
$TESTS_DIR/prctl/prctl
echo "  + tests/prctl/prctl_child"
echo -n "    "
$TESTS_DIR/prctl/prctl_child
echo "  + tests/prctl/prctl_parent"
echo -n "    "
$TESTS_DIR/prctl/prctl_parent
echo "  + tests/prctl/prctl_inherit"
echo -n "    "
$TESTS_DIR/prctl/prctl_inherit
echo "  + tests/prctl/prctl_attach"
echo -n "    "
$TESTS_DIR/prctl/prctl_attach

echo
echo "* Checking error returns"
echo "  + tests/error_returns/e2big"
echo -n "    "
$TESTS_DIR/error_returns/e2big
echo "  + tests/error_returns/eacces"
echo -n "    "
$TESTS_DIR/error_returns/eacces
echo "  + tests/error_returns/ebadf"
echo -n "    "
$TESTS_DIR/error_returns/ebadf
echo "  + tests/error_returns/efault"
echo -n "    "
$TESTS_DIR/error_returns/efault
echo "  + tests/error_returns/einval"
echo -n "    "
$TESTS_DIR/error_returns/einval
echo "  + tests/error_returns/emfile"
echo -n "    "
$TESTS_DIR/error_returns/emfile
echo "  + tests/error_returns/enoent"
echo -n "    "
$TESTS_DIR/error_returns/enoent
echo "  + tests/error_returns/enospc"
echo -n "    "
$TESTS_DIR/error_returns/enospc
echo "  + tests/error_returns/eopnotsupp"
echo -n "    "
$TESTS_DIR/error_returns/eopnotsupp
echo "  + tests/error_returns/eoverflow"
echo -n "    "
$TESTS_DIR/error_returns/eoverflow
echo "  + tests/error_returns/eperm"
echo -n "    "
$TESTS_DIR/error_returns/eperm
echo "  + tests/error_returns/esrch"
echo -n "    "
$TESTS_DIR/error_returns/esrch
echo "  + tests/error_returns/non-existent"
echo -n "    "
$TESTS_DIR/error_returns/non-existent

echo
echo "* Checking multithread functionality"
echo "  + tests/multithread/fork_then_read"
echo -n "    "
$TESTS_DIR/multithread/fork_then_read


echo
echo "* Checking overflow functionality"
echo "  + tests/overflow/breakpoint_overflow"
echo -n "    "
$TESTS_DIR/overflow/breakpoint_overflow
echo "  + tests/overflow/simple_overflow_leader"
echo -n "    "
$TESTS_DIR/overflow/simple_overflow_leader
echo "  + tests/overflow/simple_overflow_sibling"
echo -n "    "
$TESTS_DIR/overflow/simple_overflow_sibling
echo "  + tests/overflow/simultaneous_overflow"
echo -n "    "
$TESTS_DIR/overflow/simultaneous_overflow
echo "  + tests/overflow/simultaneous_group_overflow"
echo -n "    "
$TESTS_DIR/overflow/simultaneous_group_overflow
echo "  + tests/overflow/simul_oneshot_group_overflow"
echo -n "    "
$TESTS_DIR/overflow/simul_oneshot_group_overflow
echo "  + tests/overflow/single_shot_overflow"
echo -n "    "
$TESTS_DIR/overflow/single_shot_overflow
echo "  + tests/overflow/single_shot_w_enable"
echo -n "    "
$TESTS_DIR/overflow/single_shot_w_enable
echo "  + tests/overflow/wakeup_events_overflow"
echo -n "    "
$TESTS_DIR/overflow/wakeup_events_overflow
echo "  + tests/overflow/overflow_requires_wakeup"
echo -n "    "
$TESTS_DIR/overflow/overflow_requires_wakeup
echo "  + tests/overflow/overflow_poll"
echo -n "    "
$TESTS_DIR/overflow/overflow_poll
echo "  + tests/overflow/overflow_large"
echo -n "    "
$TESTS_DIR/overflow/overflow_large
echo "  + tests/overflow/overflow_skid"
echo -n "    "
$TESTS_DIR/overflow/overflow_skid

echo
echo "* Checking tracepoint functionality"
echo "  + tests/tracepoints/tracepoint_alias"
echo -n "    "
$TESTS_DIR/tracepoints/tracepoint_alias

echo
echo "* Checking mmap record sample functionality"
echo "  + tests/record_sample/print_record_sample"
echo -n "    "
$TESTS_DIR/record_sample/print_record_sample
echo "  + tests/record_sample/validate_record_sample"
echo -n "    "
$TESTS_DIR/record_sample/validate_record_sample
echo "  + tests/record_sample/lost_record_sample"
echo -n "    "
$TESTS_DIR/record_sample/lost_record_sample
echo "  + tests/record_sample/multiple_mmap_sizes"
echo -n "    "
$TESTS_DIR/record_sample/multiple_mmap_sizes
echo "  + tests/record_sample/sample_branch_stack"
echo -n "    "
$TESTS_DIR/record_sample/sample_branch_stack
echo "  + tests/record_sample/record_comm"
echo -n "    "
$TESTS_DIR/record_sample/record_comm
echo "  + tests/record_sample/record_comm_exec"
echo -n "    "
$TESTS_DIR/record_sample/record_comm_exec
echo "  + tests/record_sample/record_fork"
echo -n "    "
$TESTS_DIR/record_sample/record_fork
echo "  + tests/record_sample/record_mmap"
echo -n "    "
$TESTS_DIR/record_sample/record_mmap
echo "  + tests/record_sample/record_mmap2"
echo -n "    "
$TESTS_DIR/record_sample/record_mmap2
echo "  + tests/record_sample/record_mmap_data"
echo -n "    "
$TESTS_DIR/record_sample/record_mmap_data
echo "  + tests/record_sample/sample_data_src"
echo -n "    "
$TESTS_DIR/record_sample/sample_data_src
echo "  + tests/record_sample/sample_regs_intr"
echo -n "    "
$TESTS_DIR/record_sample/sample_regs_intr
echo "  + tests/record_sample/sample_regs_user"
echo -n "    "
$TESTS_DIR/record_sample/sample_regs_user
echo "  + tests/record_sample/sample_stack_user"
echo -n "    "
$TESTS_DIR/record_sample/sample_stack_user
echo "  + tests/record_sample/sample_weight"
echo -n "    "
$TESTS_DIR/record_sample/sample_weight
echo "  + tests/record_sample/overwrite_mmap"
echo -n "    "
$TESTS_DIR/record_sample/overwrite_mmap
echo "  + tests/record_sample/mmap_multiple"
echo -n "    "
$TESTS_DIR/record_sample/mmap_multiple


echo
echo "* Checking bugs that PAPI has to work around"
echo "  + tests/bugs/sync_read_required"
echo -n "    "
$TESTS_DIR/bugs/sync_read_required
echo "  + tests/bugs/read_group_attached"
echo -n "    "
$TESTS_DIR/bugs/read_group_attached
echo "  + tests/bugs/check_schedulability"
echo -n "    "
$TESTS_DIR/bugs/check_schedulability
echo "  + tests/multiplexing/check_papi_multiplexing"
echo -n "    "
$TESTS_DIR/multiplexing/check_papi_multiplexing
echo "  + tests/multiplexing/check_multiplexing"
echo -n "    "
$TESTS_DIR/multiplexing/check_multiplexing

echo
echo "* Checking other bugs"
echo "  + tests/corner_cases/sw_start_leader"
echo -n "    "
$TESTS_DIR/corner_cases/sw_start_leader
echo "  + tests/corner_cases/hw_sw_mix"
echo -n "    "
$TESTS_DIR/corner_cases/hw_sw_mix

echo
echo "* Checking bugs that can silently produce wrong results"
echo "  + tests/constraints/check_constraints"
echo -n "    "
$TESTS_DIR/constraints/check_constraints
echo "  + tests/corner_cases/context_switch_user_kernel"
echo -n "    "
$TESTS_DIR/corner_cases/context_switch_user_kernel

echo
echo "* Checking interfaces that have changed over the years"
echo "  + tests/corner_cases/overflow_requires_mmap"
echo -n "    "
$TESTS_DIR/corner_cases/overflow_requires_mmap
echo "  + tests/corner_cases/signal_after_exec"
echo -n "    "
$TESTS_DIR/corner_cases/signal_after_exec
echo "  + tests/corner_cases/sampled_notleader_refresh"
echo -n "    "
$TESTS_DIR/corner_cases/sampled_notleader_refresh

echo
echo "* Checking for corner-cases in the ABI (not necessarily bugs)"
echo "  + tests/corner_cases/multiple_active"
echo -n "    "
$TESTS_DIR/corner_cases/multiple_active
echo "  + tests/corner_cases/max_num_events"
echo -n "    "
$TESTS_DIR/corner_cases/max_num_events
echo "  + tests/corner_cases/max_multiplex"
echo -n "    "
$TESTS_DIR/corner_cases/max_multiplex
echo "  + tests/corner_cases/reset_leader"
echo -n "    "
$TESTS_DIR/corner_cases/reset_leader
echo "  + tests/corner_cases/check_reset_mpx"
echo -n "    "
$TESTS_DIR/corner_cases/check_reset_mpx
echo "  + tests/corner_cases/huge_events_start"
echo -n "    "
$TESTS_DIR/corner_cases/huge_events_start
echo "  + tests/corner_cases/huge_group_start"
echo -n "    "
$TESTS_DIR/corner_cases/huge_group_start
echo "  + tests/corner_cases/signal_after_close"
echo -n "    "
$TESTS_DIR/corner_cases/signal_after_close



echo
echo "* Checking for fast RDPMC support"
echo "  + tests/rdpmc/rdpmc_support"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_support
echo "  + tests/rdpmc/rdpmc_validation"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_validation
echo "  + tests/rdpmc/rdpmc_multiplexing"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_multiplexing
echo "  + tests/rdpmc/rdpmc_reset"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_reset
echo "  + tests/rdpmc/rdpmc_group"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_group
echo "  + tests/rdpmc/rdpmc_attach"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_attach
echo "  + tests/rdpmc/rdpmc_multiattach"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_multiattach
echo "  + tests/rdpmc/rdpmc_multiattach_papi"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_multiattach_papi
echo "  + tests/rdpmc/rdpmc_pthreads"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_pthreads
echo "  + tests/rdpmc/rdpmc_pthreads_group"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_pthreads_group
echo "  + tests/rdpmc/rdpmc_attach_multi_enable"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_attach_multi_enable
echo "  + tests/rdpmc/rdpmc_exec"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_exec
echo "  + tests/rdpmc/rdpmc_exec_papi"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_exec_papi

echo
echo "* Checking OpenMP support"
echo "  + tests/openmp/openmp_test"
echo -n "    "
$TESTS_DIR/openmp/openmp_test
echo "  + tests/openmp/openmp_overflow"
echo -n "    "
$TESTS_DIR/openmp/openmp_overflow
echo "  + tests/openmp/openmp_sample"
echo -n "    "
$TESTS_DIR/openmp/openmp_sample

echo
echo "* Checking proposed interface updates (not in any released kernel)"
echo "  + tests/corner_cases/ioctl_refresh_0"
echo -n "    "
$TESTS_DIR/corner_cases/ioctl_refresh_0
echo "  + tests/corner_cases/wrong_size_enospc"
echo -n "    "
$TESTS_DIR/corner_cases/wrong_size_enospc

echo
echo "* Checking for major bugs that are unlikely to ever be fixed"
echo "  + tests/bugs/nmi_watchdog_group_leader"
echo -n "    "
$TESTS_DIR/bugs/nmi_watchdog_group_leader

echo
