#!/bin/sh

export TESTS_QUIET=y

echo
echo "* Checking generalized events"
./validation/branches
./validation/instructions
echo
echo "* Checking offcore response events"
./validation/offcore_response
echo
echo "* Checking basic perf_event functionality"
./validation/simple_overflow

echo
echo "* Checking bugs that PAPI has to work around"
./bugs/sync_read_required
./bugs/read_group_at_once
./bugs/format_id_support
./bugs/check_schedulability
./bugs/check_papi_multiplexing

echo
echo "* Checking bugs that can silently produce wrong results"
./validation/check_constraints

echo
echo "* Checking for unintended ABI behavior"
./bugs/sampled_notleader_refresh

echo
echo "* Checking missing functionality (not really errors)"
./bugs/overflow_requires_mmap

echo
