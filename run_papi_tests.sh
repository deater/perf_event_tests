#!/bin/sh

export TESTS_QUIET=y

echo
echo "* Checking PAPI predefined events"
./papi/papi_br_cn
./papi/papi_br_ins
./papi/papi_br_msp
./papi/papi_br_ntk
./papi/papi_br_prc
./papi/papi_br_tkn
./papi/papi_br_ucn
./papi/papi_fp_ins
./papi/papi_fp_ops
./papi/papi_hw_int

echo
echo "* Checking bugs that can silently produce wrong results"
./papi/core2_constraints

echo
echo "* Checking perf_event / libpfm4 features"
./papi/nhm_offcore_response
./papi/perfevent_sw_events
./papi/perfevent_generalized_events
echo
