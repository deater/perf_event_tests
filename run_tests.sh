#!/bin/sh

export TESTS_QUIET=y

echo
echo "**** RUNNING perf_event_tests ****"
uname -a

echo
echo "* Checking infrastructure"
echo "  + ./validation/get_cache_info"
echo -n "    "
./validation/get_cache_info
echo " + ./sysfs/events"
echo -n "    "
./sysfs/events

echo
echo "* Checking generalized events"
echo "  + ./generalized_events/branches"
echo -n "    "
./generalized_events/branches
echo "  + ./generalized_events/branch-misses"
echo -n "    "
./generalized_events/branch-misses
echo "  + ./generalized_events/cycles"
echo -n "    "
./generalized_events/cycles
echo "  + ./generalized_events/instructions"
echo -n "    "
./generalized_events/instructions
echo "  + ./generalized_events/l1-dcache-stores"
echo -n "    "
./generalized_events/l1-dcache-stores
echo "  + ./generalized_events/l1-dcache-loads"
echo -n "    "
./generalized_events/l1-dcache-loads

echo
echo "* Checking offcore response events"
echo "  + ./validation/offcore_response"
echo -n "    "
./validation/offcore_response
echo "  + ./validation/offcore_response_mask"
echo -n "    "
./validation/offcore_response_mask

echo
echo "* Checking events that require special permissions"
echo "  + ./validation_privileged/uncore_events"
echo -n "    "
./validation_privileged/uncore_events

echo
echo "* Checking basic perf_event functionality"
echo "  + ./validation/format_id_support"
echo -n "    "
./validation/format_id_support
echo "  + ./validation/non-existent"
echo -n "    "
./validation/non-existent
echo "  + ./validation/breakpoint_support"
echo -n "    "
./validation/breakpoint_support
echo "  + ./validation/inherit"
echo -n "    "
./validation/inherit
echo "  + ./validation/inherit_stat"
echo -n "    "
./validation/inherit_stat
echo "  + ./validation/enable_on_exec"
echo -n "    "
./validation/enable_on_exec
echo "  + ./validation/ioctl_flag_group"
echo -n "    "
./validation/ioctl_flag_group

echo
echo "* Checking overflow functionality"
echo "  + ./overflow/breakpoint_overflow"
echo -n "    "
./overflow/breakpoint_overflow
echo "  + ./overflow/simple_overflow_leader"
echo -n "    "
./overflow/simple_overflow_leader
echo "  + ./overflow/simple_overflow_sibling"
echo -n "    "
./overflow/simple_overflow_sibling
echo "  + ./overflow/simultaneous_overflow"
echo -n "    "
./overflow/simultaneous_overflow
echo "  + ./overflow/simultaneous_group_overflow"
echo -n "    "
./overflow/simultaneous_group_overflow
echo "  + ./overflow/simul_oneshot_group_overflow"
echo -n "    "
./overflow/simul_oneshot_group_overflow
echo "  + ./overflow/single_shot_overflow"
echo -n "    "
./overflow/single_shot_overflow
echo "  + ./overflow/single_shot_w_enable"
echo -n "    "
./overflow/single_shot_w_enable
echo "  + ./overflow/wakeup_events_overflow"
echo -n "    "
./overflow/wakeup_events_overflow
echo "  + ./overflow/overflow_requires_wakeup"
echo -n "    "
./overflow/overflow_requires_wakeup


echo
echo "* Checking mmap record sample functionality"
echo "  + ./record_sample/print_record_sample"
echo -n "    "
./record_sample/print_record_sample
echo "  + ./record_sample/validate_record_sample"
echo -n "    "
./record_sample/validate_record_sample
echo "  + ./record_sample/lost_record_sample"
echo -n "    "
./record_sample/lost_record_sample
echo "  + ./record_sample/multiple_mmap_sizes"
echo -n "    "
./record_sample/multiple_mmap_sizes

echo
echo "* Checking bugs that PAPI has to work around"
echo "  + ./bugs/sync_read_required"
echo -n "    "
./bugs/sync_read_required
echo "  + ./bugs/read_group_attached"
echo -n "    "
./bugs/read_group_attached
echo "  + ./bugs/check_schedulability"
echo -n "    "
./bugs/check_schedulability
echo "  + ./bugs/check_papi_multiplexing"
echo -n "    "
./bugs/check_papi_multiplexing
echo "  + ./bugs/check_multiplexing"
echo -n "    "
./bugs/check_multiplexing

echo
echo "* Checking other bugs"
echo "  + ./corner_cases/sw_start_leader"
echo -n "    "
./corner_cases/sw_start_leader
echo "  + ./corner_cases/hw_sw_mix"
echo -n "    "
./corner_cases/hw_sw_mix

echo
echo "* Checking bugs that can silently produce wrong results"
echo "  + ./validation/check_constraints"
echo -n "    "
./validation/check_constraints
echo "  + ./corner_cases/context_switch_user_kernel"
echo -n "    "
./corner_cases/context_switch_user_kernel

echo
echo "* Checking interfaces that have changed over the years"
echo "  + ./corner_cases/overflow_requires_mmap"
echo -n "    "
./corner_cases/overflow_requires_mmap
echo "  + ./corner_cases/signal_after_exec"
echo -n "    "
./corner_cases/signal_after_exec
echo "  + ./corner_cases/sampled_notleader_refresh"
echo -n "    "
./corner_cases/sampled_notleader_refresh

echo
echo "* Checking for corner-cases in the ABI (not necessarily bugs)"
echo "  + ./corner_cases/multiple_active"
echo -n "    "
./corner_cases/multiple_active
echo "  + ./corner_cases/max_multiplex"
echo -n "    "
./corner_cases/max_multiplex
echo "  + ./corner_cases/reset_leader"
echo -n "    "
./corner_cases/reset_leader
echo "  + ./corner_cases/check_reset_mpx"
echo -n "    "
./corner_cases/check_reset_mpx

echo
echo "* Checking for fast RDPMC support"
echo "  + ./validation/rdpmc_support"
echo -n "    "
./validation/rdpmc_support
echo "  + ./validation/rdpmc_validation"
echo -n "    "
./validation/rdpmc_validation

echo
echo "* Checking proposed interface updates (not in any released kernel)"
echo "  + ./corner_cases/ioctl_refresh_0"
echo -n "    "
./corner_cases/ioctl_refresh_0
echo "  + ./corner_cases/wrong_size_enospc"
echo -n "    "
./corner_cases/wrong_size_enospc

echo
echo "* Checking for major bugs that are unlikely to ever be fixed"
echo "  + ./bugs/nmi_watchdog_group_leader"
echo -n "    "
./bugs/nmi_watchdog_group_leader

echo
