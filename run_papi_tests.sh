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
./papi/papi_bru_idl
./papi/papi_btac_m
./papi/papi_ca_cln
./papi/papi_ca_inv
./papi/papi_ca_itv
./papi/papi_ca_shr
./papi/papi_csr_fal
./papi/papi_csr_suc
./papi/papi_csr_tot
./papi/papi_fp_ins
./papi/papi_fp_ops
./papi/papi_fpu_idl
./papi/papi_fxu_idl
./papi/papi_hw_int
./papi/papi_l1_dca
./papi/papi_l1_dcm
./papi/papi_l1_icm
./papi/papi_l1_ldm
./papi/papi_l1_stm
./papi/papi_l1_tcm
./papi/papi_l2_dca
./papi/papi_l2_dcm
./papi/papi_l2_dcr
./papi/papi_l2_icm
./papi/papi_l2_ldm
./papi/papi_l2_stm
./papi/papi_l2_tcm
./papi/papi_l2_tcr
./papi/papi_l3_dch
./papi/papi_l3_dcm
./papi/papi_l3_icm
./papi/papi_l3_ldm
./papi/papi_l3_stm
./papi/papi_l3_tcm
./papi/papi_lsu_idl
./papi/papi_prf_dm
./papi/papi_tlb_dm
./papi/papi_tlb_im
./papi/papi_tlb_sd
./papi/papi_tlb_tl
./papi/papi_tot_cyc
./papi/papi_tot_ins

echo
echo "* Checking general PAPI functionality"
./papi/check_mhz_value

echo
echo "* Checking bugs that can silently produce wrong results"
./papi/core2_constraints

echo
echo "* Checking perf_event / libpfm4 features"
./papi/nhm_offcore_response
./papi/perfevent_sw_events
./papi/perfevent_generalized_events
echo
