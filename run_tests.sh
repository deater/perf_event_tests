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
echo "  + utils/check_paranoid"
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
echo "  + utils/get_cache_info"
echo -n "    "
$TESTS_DIR/utils/get_cache_info
echo " + sysfs/events"
echo -n "    "
$TESTS_DIR/sysfs/events

echo
echo "* Checking generalized events"
echo "  + generalized_events/branches"
echo -n "    "
$TESTS_DIR/generalized_events/branches
echo "  + generalized_events/branch-misses"
echo -n "    "
$TESTS_DIR/generalized_events/branch-misses
echo "  + generalized_events/cycles"
echo -n "    "
$TESTS_DIR/generalized_events/cycles
echo "  + generalized_events/instructions"
echo -n "    "
$TESTS_DIR/generalized_events/instructions
echo "  + generalized_events/l1-dcache-stores"
echo -n "    "
$TESTS_DIR/generalized_events/l1-dcache-stores
echo "  + generalized_events/l1-dcache-loads"
echo -n "    "
$TESTS_DIR/generalized_events/l1-dcache-loads

echo
echo "* Checking Intel x86 specific features"
echo "  + x86_intel/fixed_ctr0"
echo -n "    "
$TESTS_DIR/x86_intel/fixed_ctr0
echo "  + x86_intel/fixed_ctr1"
echo -n "    "
$TESTS_DIR/x86_intel/fixed_ctr1
echo "  + x86_intel/fixed_ctr2"
echo -n "    "
$TESTS_DIR/x86_intel/fixed_ctr2
echo "  + x86_intel/offcore_response"
echo -n "    "
$TESTS_DIR/x86_intel/offcore_response
echo "  + x86_intel/offcore_response_mask"
echo -n "    "
$TESTS_DIR/x86_intel/offcore_response_mask
echo "  + x86_intel/uncore_events"
echo -n "    "
$TESTS_DIR/x86_intel/uncore_events

echo
echo "* Checking breakpoint support"
echo "  + breakpoints/breakpoint_support"
echo -n "    "
$TESTS_DIR/breakpoints/breakpoint_support

echo
echo "* Checking basic perf_event functionality"
echo "  + attr_fields/disabled"
echo -n "    "
$TESTS_DIR/attr_fields/disabled
echo "  + attr_fields/exclusive"
echo -n "    "
$TESTS_DIR/attr_fields/exclusive
echo "  + attr_fields/format_id_support"
echo -n "    "
$TESTS_DIR/attr_fields/format_id_support
echo "  + attr_fields/inherit"
echo -n "    "
$TESTS_DIR/attr_fields/inherit
echo "  + attr_fields/inherit_stat"
echo -n "    "
$TESTS_DIR/attr_fields/inherit_stat
echo "  + attr_fields/pinned"
echo -n "    "
$TESTS_DIR/attr_fields/pinned
echo "  + attr_fields/read_format_range_check"
echo -n "    "
$TESTS_DIR/attr_fields/read_format_range_check
echo "  + attr_fields/sample_type_range_check"
echo -n "    "
$TESTS_DIR/attr_fields/sample_type_range_check
echo "  + attr_fields/exclude_user"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_user
echo "  + attr_fields/exclude_kernel"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_kernel
echo "  + attr_fields/exclude_hv"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_hv
echo "  + attr_fields/exclude_guest"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_guest
echo "  + attr_fields/exclude_host"
echo -n "    "
$TESTS_DIR/attr_fields/exclude_host
echo "  + flags/flags_range_check"
echo -n "    "
$TESTS_DIR/flags/flags_range_check
echo "  + flags/flags_cgroup"
echo -n "    "
$TESTS_DIR/flags/flags_cgroup
echo "  + flags/flags_fd_no_group"
echo -n "    "
$TESTS_DIR/flags/flags_fd_no_group
echo "  + flags/flags_fd_output"
echo -n "    "
$TESTS_DIR/flags/flags_fd_output

echo "  + socket/socket_pass"
echo -n "    "
$TESTS_DIR/socket/socket_pass

echo
echo "* Checking attr.size behavior"
echo "  + size/too_big"
echo -n "    "
$TESTS_DIR/size/too_big
echo "  + size/too_little"
echo -n "    "
$TESTS_DIR/size/too_little
echo "  + size/just_right"
echo -n "    "
$TESTS_DIR/size/just_right
echo "  + size/header_mismatch"
echo -n "    "
$TESTS_DIR/size/header_mismatch





echo
echo "* Checking perf_event ioctl calls"
echo "  + ioctl/ioctl_flag_group"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_flag_group
echo "  + ioctl/ioctl_id"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_id
echo "  + ioctl/ioctl_period"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_period
echo "  + ioctl/ioctl_set_filter"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_set_filter
echo "  + ioctl/ioctl_set_output"
echo -n "    "
$TESTS_DIR/ioctl/ioctl_set_output

echo
echo "* Checking error returns"
echo "  + error_returns/e2big"
echo -n "    "
$TESTS_DIR/error_returns/e2big
echo "  + error_returns/eacces"
echo -n "    "
$TESTS_DIR/error_returns/eacces
echo "  + error_returns/ebadf"
echo -n "    "
$TESTS_DIR/error_returns/ebadf
echo "  + error_returns/efault"
echo -n "    "
$TESTS_DIR/error_returns/efault
echo "  + error_returns/einval"
echo -n "    "
$TESTS_DIR/error_returns/einval
echo "  + error_returns/emfile"
echo -n "    "
$TESTS_DIR/error_returns/emfile
echo "  + error_returns/enoent"
echo -n "    "
$TESTS_DIR/error_returns/enoent
echo "  + error_returns/enospc"
echo -n "    "
$TESTS_DIR/error_returns/enospc
echo "  + error_returns/eopnotsupp"
echo -n "    "
$TESTS_DIR/error_returns/eopnotsupp
echo "  + error_returns/eoverflow"
echo -n "    "
$TESTS_DIR/error_returns/eoverflow
echo "  + error_returns/eperm"
echo -n "    "
$TESTS_DIR/error_returns/eperm
echo "  + error_returns/esrch"
echo -n "    "
$TESTS_DIR/error_returns/esrch
echo "  + error_returns/non-existent"
echo -n "    "
$TESTS_DIR/error_returns/non-existent

echo
echo "* Checking multithread functionality"
echo "  + multithread/fork_then_read"
echo -n "    "
$TESTS_DIR/multithread/fork_then_read


echo
echo "* Checking overflow functionality"
echo "  + overflow/breakpoint_overflow"
echo -n "    "
$TESTS_DIR/overflow/breakpoint_overflow
echo "  + overflow/simple_overflow_leader"
echo -n "    "
$TESTS_DIR/overflow/simple_overflow_leader
echo "  + overflow/simple_overflow_sibling"
echo -n "    "
$TESTS_DIR/overflow/simple_overflow_sibling
echo "  + overflow/simultaneous_overflow"
echo -n "    "
$TESTS_DIR/overflow/simultaneous_overflow
echo "  + overflow/simultaneous_group_overflow"
echo -n "    "
$TESTS_DIR/overflow/simultaneous_group_overflow
echo "  + overflow/simul_oneshot_group_overflow"
echo -n "    "
$TESTS_DIR/overflow/simul_oneshot_group_overflow
echo "  + overflow/single_shot_overflow"
echo -n "    "
$TESTS_DIR/overflow/single_shot_overflow
echo "  + overflow/single_shot_w_enable"
echo -n "    "
$TESTS_DIR/overflow/single_shot_w_enable
echo "  + overflow/wakeup_events_overflow"
echo -n "    "
$TESTS_DIR/overflow/wakeup_events_overflow
echo "  + overflow/overflow_requires_wakeup"
echo -n "    "
$TESTS_DIR/overflow/overflow_requires_wakeup
echo "  + overflow/overflow_poll"
echo -n "    "
$TESTS_DIR/overflow/overflow_poll
echo "  + overflow/overflow_large"
echo -n "    "
$TESTS_DIR/overflow/overflow_large
echo "  + overflow/overflow_skid"
echo -n "    "
$TESTS_DIR/overflow/overflow_skid

echo
echo "* Checking tracepoint functionality"
echo "  + tracepoints/tracepoint_alias"
echo -n "    "
$TESTS_DIR/tracepoints/tracepoint_alias

echo
echo "* Checking mmap record sample functionality"
echo "  + record_sample/print_record_sample"
echo -n "    "
$TESTS_DIR/record_sample/print_record_sample
echo "  + record_sample/validate_record_sample"
echo -n "    "
$TESTS_DIR/record_sample/validate_record_sample
echo "  + record_sample/lost_record_sample"
echo -n "    "
$TESTS_DIR/record_sample/lost_record_sample
echo "  + record_sample/multiple_mmap_sizes"
echo -n "    "
$TESTS_DIR/record_sample/multiple_mmap_sizes
echo "  + record_sample/sample_branch_stack"
echo -n "    "
$TESTS_DIR/record_sample/sample_branch_stack
echo "  + record_sample/record_comm"
echo -n "    "
$TESTS_DIR/record_sample/record_comm
echo "  + record_sample/record_comm_exec"
echo -n "    "
$TESTS_DIR/record_sample/record_comm_exec
echo "  + record_sample/record_fork"
echo -n "    "
$TESTS_DIR/record_sample/record_fork
echo "  + record_sample/record_mmap"
echo -n "    "
$TESTS_DIR/record_sample/record_mmap
echo "  + record_sample/record_mmap2"
echo -n "    "
$TESTS_DIR/record_sample/record_mmap2
echo "  + record_sample/record_mmap_data"
echo -n "    "
$TESTS_DIR/record_sample/record_mmap_data
echo "  + record_sample/sample_data_src"
echo -n "    "
$TESTS_DIR/record_sample/sample_data_src
echo "  + record_sample/sample_regs_intr"
echo -n "    "
$TESTS_DIR/record_sample/sample_regs_intr
echo "  + record_sample/sample_regs_user"
echo -n "    "
$TESTS_DIR/record_sample/sample_regs_user
echo "  + record_sample/sample_stack_user"
echo -n "    "
$TESTS_DIR/record_sample/sample_stack_user
echo "  + record_sample/sample_weight"
echo -n "    "
$TESTS_DIR/record_sample/sample_weight
echo "  + record_sample/overwrite_mmap"
echo -n "    "
$TESTS_DIR/record_sample/overwrite_mmap

echo
echo "* Checking bugs that PAPI has to work around"
echo "  + bugs/sync_read_required"
echo -n "    "
$TESTS_DIR/bugs/sync_read_required
echo "  + bugs/read_group_attached"
echo -n "    "
$TESTS_DIR/bugs/read_group_attached
echo "  + bugs/check_schedulability"
echo -n "    "
$TESTS_DIR/bugs/check_schedulability
echo "  + multiplexing/check_papi_multiplexing"
echo -n "    "
$TESTS_DIR/multiplexing/check_papi_multiplexing
echo "  + multiplexing/check_multiplexing"
echo -n "    "
$TESTS_DIR/multiplexing/check_multiplexing

echo
echo "* Checking other bugs"
echo "  + corner_cases/sw_start_leader"
echo -n "    "
$TESTS_DIR/corner_cases/sw_start_leader
echo "  + corner_cases/hw_sw_mix"
echo -n "    "
$TESTS_DIR/corner_cases/hw_sw_mix

echo
echo "* Checking bugs that can silently produce wrong results"
echo "  + constraints/check_constraints"
echo -n "    "
$TESTS_DIR/constraints/check_constraints
echo "  + corner_cases/context_switch_user_kernel"
echo -n "    "
$TESTS_DIR/corner_cases/context_switch_user_kernel

echo
echo "* Checking interfaces that have changed over the years"
echo "  + corner_cases/overflow_requires_mmap"
echo -n "    "
$TESTS_DIR/corner_cases/overflow_requires_mmap
echo "  + corner_cases/signal_after_exec"
echo -n "    "
$TESTS_DIR/corner_cases/signal_after_exec
echo "  + corner_cases/sampled_notleader_refresh"
echo -n "    "
$TESTS_DIR/corner_cases/sampled_notleader_refresh

echo
echo "* Checking for corner-cases in the ABI (not necessarily bugs)"
echo "  + corner_cases/multiple_active"
echo -n "    "
$TESTS_DIR/corner_cases/multiple_active
echo "  + corner_cases/max_num_events"
echo -n "    "
$TESTS_DIR/corner_cases/max_num_events
echo "  + corner_cases/max_multiplex"
echo -n "    "
$TESTS_DIR/corner_cases/max_multiplex
echo "  + corner_cases/reset_leader"
echo -n "    "
$TESTS_DIR/corner_cases/reset_leader
echo "  + corner_cases/check_reset_mpx"
echo -n "    "
$TESTS_DIR/corner_cases/check_reset_mpx
echo "  + corner_cases/huge_events_start"
echo -n "    "
$TESTS_DIR/corner_cases/huge_events_start
echo "  + corner_cases/huge_group_start"
echo -n "    "
$TESTS_DIR/corner_cases/huge_group_start



echo
echo "* Checking for fast RDPMC support"
echo "  + rdpmc/rdpmc_support"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_support
echo "  + rdpmc/rdpmc_validation"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_validation
echo "  + rdpmc/rdpmc_multiplexing"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_multiplexing
echo "  + rdpmc/rdpmc_reset"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_reset
echo "  + rdpmc/rdpmc_group"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_group
echo "  + rdpmc/rdpmc_attach"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_attach
echo "  + rdpmc/rdpmc_multiattach"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_multiattach
echo "  + rdpmc/rdpmc_multiattach_papi"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_multiattach_papi
echo "  + rdpmc/rdpmc_pthreads"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_pthreads
echo "  + rdpmc/rdpmc_pthreads_group"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_pthreads_group
echo "  + rdpmc/rdpmc_attach_multi_enable"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_attach_multi_enable
echo "  + rdpmc/rdpmc_exec"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_exec
echo "  + rdpmc/rdpmc_exec_papi"
echo -n "    "
$TESTS_DIR/rdpmc/rdpmc_exec_papi

echo
echo "* Checking OpenMP support"
echo "  + openmp/openmp_test"
echo -n "    "
$TESTS_DIR/openmp/openmp_test
echo "  + openmp/openmp_overflow"
echo -n "    "
$TESTS_DIR/openmp/openmp_overflow
echo "  + openmp/openmp_sample"
echo -n "    "
$TESTS_DIR/openmp/openmp_sample

echo
echo "* Checking proposed interface updates (not in any released kernel)"
echo "  + corner_cases/ioctl_refresh_0"
echo -n "    "
$TESTS_DIR/corner_cases/ioctl_refresh_0
echo "  + corner_cases/wrong_size_enospc"
echo -n "    "
$TESTS_DIR/corner_cases/wrong_size_enospc

echo
echo "* Checking for major bugs that are unlikely to ever be fixed"
echo "  + bugs/nmi_watchdog_group_leader"
echo -n "    "
$TESTS_DIR/bugs/nmi_watchdog_group_leader

echo
