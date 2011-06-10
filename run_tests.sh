#!/bin/sh

export TESTS_QUIET=y

echo
echo "* Checking generalized events"
./validation/branches
./validation/branch-misses
./validation/instructions
./validation/l1-dcache-stores
./validation/l1-dcache-loads
echo
echo "* Checking offcore response events"
./validation/offcore_response
echo
echo "* Checking basic perf_event functionality"
./validation/simple_overflow_leader
./validation/simple_overflow_sibling
./validation/format_id_support
./validation/non-existent

echo
echo "* Checking bugs that PAPI has to work around"
./bugs/sync_read_required
./bugs/read_group_attached
./bugs/check_schedulability
./bugs/check_papi_multiplexing

echo
echo "* Checking bugs that can silently produce wrong results"
./validation/check_constraints

echo
echo "* Checking for unintended ABI behavior"
./bugs/sampled_notleader_refresh
./bugs/ioctl_refresh_0

echo
echo "* Checking missing functionality (not really errors)"
./bugs/overflow_requires_mmap

echo
