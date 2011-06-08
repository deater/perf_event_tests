#!/bin/sh

export TESTS_QUIET=y

echo
echo "* Checking PAPI predefined events"
./papi/papi_br_cn
./papi/papi_br_ins
./papi/papi_br_msp

echo
echo "* Checking bugs that can silently produce wrong results"
./papi/core2_constraints

echo
echo "* Checking perf_event / libpfm4 features"
./papi/nhm_offcore_response
./papi/perfevent_sw_events
./papi/perfevent_generalized_events
echo
