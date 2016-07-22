CC	= gcc
CC_R	= $(CC) -pthread
CFLAGS	= -g -O2 -Wall -I../include

include ../Makefile.papi.inc


all:	check_mhz_value \
	check_papi_latency \
	core2_constraints \
	cpufreq_scaling \
	unregister \
	event_create_race \
	mpx_oflo \
	offcore_response \
	perfevent_sw_events \
	perfevent_generalized_events \
	papi_br_cn \
	papi_br_ins \
	papi_br_msp \
	papi_br_ntk \
	papi_br_prc \
	papi_br_tkn \
	papi_br_ucn \
	papi_bru_idl \
	papi_btac_m \
	papi_ca_cln \
	papi_ca_inv \
	papi_ca_itv \
	papi_ca_shr \
	papi_ca_snp \
	papi_csr_fal \
	papi_csr_suc \
	papi_csr_tot \
	papi_dp_ops \
	papi_fad_ins \
	papi_fdv_ins \
	papi_fma_ins \
	papi_fml_ins \
	papi_fnv_ins \
	papi_fp_ins \
	papi_fp_ops \
	papi_fp_stal \
	papi_fpu_idl \
	papi_fsq_ins \
	papi_ful_ccy \
	papi_ful_icy \
	papi_fxu_idl \
	papi_hw_int \
	papi_int_ins \
	papi_l1_dca \
	papi_l1_dch \
	papi_l1_dcm \
	papi_l1_dcr \
	papi_l1_dcw \
	papi_l1_ica \
	papi_l1_ich \
	papi_l1_icm \
	papi_l1_icr \
	papi_l1_icw \
	papi_l1_ldm \
	papi_l1_stm \
	papi_l1_tca \
	papi_l1_tch \
	papi_l1_tcm \
	papi_l1_tcr \
	papi_l1_tcw \
	papi_l2_dca \
	papi_l2_dch \
	papi_l2_dcm \
	papi_l2_dcr \
	papi_l2_dcw \
	papi_l2_ica \
	papi_l2_ich \
	papi_l2_icm \
	papi_l2_icr \
	papi_l2_icw \
	papi_l2_ldm \
	papi_l2_stm \
	papi_l2_tca \
	papi_l2_tch \
	papi_l2_tcm \
	papi_l2_tcr \
	papi_l2_tcw \
	papi_l3_dca \
	papi_l3_dch \
	papi_l3_dcm \
	papi_l3_dcr \
	papi_l3_dcw \
	papi_l3_ica \
	papi_l3_ich \
	papi_l3_icm \
	papi_l3_icr \
	papi_l3_icw \
	papi_l3_ldm \
	papi_l3_stm \
	papi_l3_tca \
	papi_l3_tch \
	papi_l3_tcm \
	papi_l3_tcr \
	papi_l3_tcw \
	papi_ld_ins \
	papi_lst_ins \
	papi_lsu_idl \
	papi_mem_rcy \
	papi_mem_scy \
	papi_mem_wcy \
	papi_prf_dm \
	papi_res_stl \
	papi_sp_ops \
	papi_sr_ins \
	papi_stl_ccy \
	papi_stl_icy \
	papi_syc_ins \
	papi_tlb_dm \
	papi_tlb_im \
	papi_tlb_sd \
	papi_tlb_tl \
	papi_tot_cyc \
	papi_tot_iis \
	papi_tot_ins \
	papi_vec_dp \
	papi_vec_ins \
	papi_vec_sp \
	papi_profile

#	papi_bgl_oed \
#	papi_bgl_ts_32b \
#	papi_bgl_ts_full \
#	papi_bgl_tr_dpkt \
#	papi_bgl_tr_full 


###

papi_br_cn:	papi_br_cn.o ../lib/test_utils.o ../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o papi_br_cn papi_br_cn.o \
		../lib/test_utils.o ../lib/branches_testcode.o $(PAPI_LIB) 

papi_br_cn.o:	papi_br_cn.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_br_cn.c


###

papi_br_ins:	papi_br_ins.o ../lib/test_utils.o ../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o papi_br_ins papi_br_ins.o \
		 ../lib/test_utils.o ../lib/branches_testcode.o $(PAPI_LIB)

papi_br_ins.o:	papi_br_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_br_ins.c


###

papi_br_msp:	papi_br_msp.o ../lib/test_utils.o ../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o papi_br_msp papi_br_msp.o \
		../lib/test_utils.o ../lib/branches_testcode.o $(PAPI_LIB) 

papi_br_msp.o:	papi_br_msp.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_br_msp.c

###

papi_br_tkn:	papi_br_tkn.o ../lib/test_utils.o ../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o papi_br_tkn papi_br_tkn.o \
		../lib/test_utils.o ../lib/branches_testcode.o $(PAPI_LIB) 

papi_br_tkn.o:	papi_br_tkn.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_br_tkn.c


###

papi_br_ntk:	papi_br_ntk.o ../lib/test_utils.o ../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o papi_br_ntk papi_br_ntk.o \
		../lib/test_utils.o ../lib/branches_testcode.o $(PAPI_LIB) 

papi_br_ntk.o:	papi_br_ntk.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_br_ntk.c

###

papi_br_prc:	papi_br_prc.o ../lib/test_utils.o ../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o papi_br_prc papi_br_prc.o \
		../lib/test_utils.o ../lib/branches_testcode.o $(PAPI_LIB) 

papi_br_prc.o:	papi_br_prc.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_br_prc.c

###

papi_br_ucn:	papi_br_ucn.o ../lib/test_utils.o ../lib/branches_testcode.o
	$(CC) $(LFLAGS) -o papi_br_ucn papi_br_ucn.o \
		../lib/test_utils.o ../lib/branches_testcode.o $(PAPI_LIB) 

papi_br_ucn.o:	papi_br_ucn.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_br_ucn.c


###

papi_bru_idl:	papi_bru_idl.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_bru_idl papi_bru_idl.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_bru_idl.o:	papi_bru_idl.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_bru_idl.c

###

papi_btac_m:	papi_btac_m.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_btac_m papi_btac_m.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_btac_m.o:	papi_btac_m.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_btac_m.c


###

papi_ca_cln:	papi_ca_cln.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ca_cln papi_ca_cln.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ca_cln.o:	papi_ca_cln.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ca_cln.c

###

papi_ca_inv:	papi_ca_inv.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ca_inv papi_ca_inv.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ca_inv.o:	papi_ca_inv.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ca_inv.c

###

papi_ca_itv:	papi_ca_itv.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ca_itv papi_ca_itv.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ca_itv.o:	papi_ca_itv.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ca_itv.c

###

papi_ca_shr:	papi_ca_shr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ca_shr papi_ca_shr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ca_shr.o:	papi_ca_shr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ca_shr.c

###

papi_ca_snp:	papi_ca_snp.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ca_snp papi_ca_snp.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ca_snp.o:	papi_ca_snp.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ca_snp.c

###

papi_csr_fal:	papi_csr_fal.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_csr_fal papi_csr_fal.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_csr_fal.o:	papi_csr_fal.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_csr_fal.c

###

papi_csr_suc:	papi_csr_suc.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_csr_suc papi_csr_suc.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_csr_suc.o:	papi_csr_suc.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_csr_suc.c

###

papi_csr_tot:	papi_csr_tot.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_csr_tot papi_csr_tot.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_csr_tot.o:	papi_csr_tot.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_csr_tot.c


###

papi_fp_ins:	papi_fp_ins.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o papi_fp_ins papi_fp_ins.o \
		../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

papi_fp_ins.o:	papi_fp_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fp_ins.c


###

papi_fp_ops:	papi_fp_ops.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o papi_fp_ops papi_fp_ops.o \
		../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

papi_fp_ops.o:	papi_fp_ops.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fp_ops.c



###

papi_fpu_idl:	papi_fpu_idl.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fpu_idl papi_fpu_idl.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fpu_idl.o:	papi_fpu_idl.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fpu_idl.c

###

papi_fxu_idl:	papi_fxu_idl.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fxu_idl papi_fxu_idl.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fxu_idl.o:	papi_fxu_idl.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fxu_idl.c


###

papi_hw_int:	papi_hw_int.o ../lib/test_utils.o ../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o papi_hw_int papi_hw_int.o \
		../lib/test_utils.o ../lib/instructions_testcode.o $(PAPI_LIB)

papi_hw_int.o:	papi_hw_int.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_hw_int.c

###

papi_l1_dca:	papi_l1_dca.o papi_cache_info.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_dca papi_l1_dca.o papi_cache_info.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_dca.o:	papi_l1_dca.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_dca.c

###

papi_l1_dcm:	papi_l1_dcm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_dcm papi_l1_dcm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_dcm.o:	papi_l1_dcm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_dcm.c

###

papi_l1_icm:	papi_l1_icm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_icm papi_l1_icm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_icm.o:	papi_l1_icm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_icm.c


###

papi_l1_ldm:	papi_l1_ldm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_ldm papi_l1_ldm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_ldm.o:	papi_l1_ldm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_ldm.c

###

papi_l1_stm:	papi_l1_stm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_stm papi_l1_stm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_stm.o:	papi_l1_stm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_stm.c


###

papi_l1_tcm:	papi_l1_tcm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_tcm papi_l1_tcm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_tcm.o:	papi_l1_tcm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_tcm.c


###

papi_l2_dca:	papi_l2_dca.o papi_cache_info.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_dca papi_l2_dca.o papi_cache_info.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_dca.o:	papi_l2_dca.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_dca.c

###

papi_l2_dcm:	papi_l2_dcm.o papi_cache_info.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_dcm papi_l2_dcm.o \
		papi_cache_info.o ../lib/test_utils.o $(PAPI_LIB)

papi_l2_dcm.o:	papi_l2_dcm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_dcm.c

###

papi_l2_dcr:	papi_l2_dcr.o papi_cache_info.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_dcr papi_l2_dcr.o \
		papi_cache_info.o ../lib/test_utils.o $(PAPI_LIB)

papi_l2_dcr.o:	papi_l2_dcr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_dcr.c

###

papi_l2_icm:	papi_l2_icm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_icm papi_l2_icm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_icm.o:	papi_l2_icm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_icm.c

###

papi_l2_ldm:	papi_l2_ldm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_ldm papi_l2_ldm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_ldm.o:	papi_l2_ldm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_ldm.c

###

papi_l2_stm:	papi_l2_stm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_stm papi_l2_stm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_stm.o:	papi_l2_stm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_stm.c

###

papi_l2_tcm:	papi_l2_tcm.o papi_cache_info.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_tcm papi_l2_tcm.o \
		papi_cache_info.o ../lib/test_utils.o $(PAPI_LIB)

papi_l2_tcm.o:	papi_l2_tcm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_tcm.c


###

papi_l2_tcr:	papi_l2_tcr.o papi_cache_info.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_tcr papi_l2_tcr.o \
		papi_cache_info.o ../lib/test_utils.o $(PAPI_LIB)

papi_l2_tcr.o:	papi_l2_tcr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_tcr.c


###

papi_l3_dch:	papi_l3_dch.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_dch papi_l3_dch.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_dch.o:	papi_l3_dch.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_dch.c


###

papi_l3_dcm:	papi_l3_dcm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_dcm papi_l3_dcm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_dcm.o:	papi_l3_dcm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_dcm.c

###

papi_l3_icm:	papi_l3_icm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_icm papi_l3_icm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_icm.o:	papi_l3_icm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_icm.c

###

papi_l3_ldm:	papi_l3_ldm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_ldm papi_l3_ldm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_ldm.o:	papi_l3_ldm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_ldm.c

###

papi_l3_stm:	papi_l3_stm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_stm papi_l3_stm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_stm.o:	papi_l3_stm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_stm.c

###

papi_l3_tcm:	papi_l3_tcm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_tcm papi_l3_tcm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_tcm.o:	papi_l3_tcm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_tcm.c


###

papi_lsu_idl:	papi_lsu_idl.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_lsu_idl papi_lsu_idl.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_lsu_idl.o:	papi_lsu_idl.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_lsu_idl.c

###

papi_prf_dm:	papi_prf_dm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_prf_dm papi_prf_dm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_prf_dm.o:	papi_prf_dm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_prf_dm.c


###

papi_tlb_dm:	papi_tlb_dm.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_tlb_dm papi_tlb_dm.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_tlb_dm.o:	papi_tlb_dm.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_tlb_dm.c

###

papi_tlb_im:	papi_tlb_im.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_tlb_im papi_tlb_im.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_tlb_im.o:	papi_tlb_im.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_tlb_im.c

###

papi_tlb_sd:	papi_tlb_sd.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_tlb_sd papi_tlb_sd.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_tlb_sd.o:	papi_tlb_sd.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_tlb_sd.c


###

papi_tlb_tl:	papi_tlb_tl.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_tlb_tl papi_tlb_tl.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_tlb_tl.o:	papi_tlb_tl.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_tlb_tl.c



####

papi_cache_info.o:	papi_cache_info.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_cache_info.c

###

papi_tot_cyc:	papi_tot_cyc.o ../lib/test_utils.o \
		../lib/matrix_multiply.o ../lib/nops_testcode.o
	$(CC) $(LFLAGS) -o papi_tot_cyc papi_tot_cyc.o \
		../lib/test_utils.o \
		../lib/matrix_multiply.o ../lib/nops_testcode.o $(PAPI_LIB)

papi_tot_cyc.o:	papi_tot_cyc.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_tot_cyc.c

###

papi_tot_ins:	papi_tot_ins.o \
		../lib/test_utils.o ../lib/instructions_testcode.o 
	$(CC) $(LFLAGS) -o papi_tot_ins papi_tot_ins.o \
		../lib/test_utils.o ../lib/instructions_testcode.o $(PAPI_LIB)

papi_tot_ins.o:	papi_tot_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_tot_ins.c


###

papi_mem_scy:	papi_mem_scy.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_mem_scy papi_mem_scy.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_mem_scy.o:  papi_mem_scy.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_mem_scy.c

###

papi_mem_rcy:	papi_mem_rcy.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_mem_rcy papi_mem_rcy.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_mem_rcy.o:  papi_mem_rcy.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_mem_rcy.c

###

papi_mem_wcy:	papi_mem_wcy.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_mem_wcy papi_mem_wcy.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_mem_wcy.o:  papi_mem_wcy.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_mem_wcy.c

###

papi_stl_icy:	papi_stl_icy.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_stl_icy papi_stl_icy.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_stl_icy.o:  papi_stl_icy.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_stl_icy.c

###

papi_ful_icy:	papi_ful_icy.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ful_icy papi_ful_icy.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ful_icy.o:  papi_ful_icy.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ful_icy.c

###

papi_stl_ccy:	papi_stl_ccy.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_stl_ccy papi_stl_ccy.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_stl_ccy.o:  papi_stl_ccy.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_stl_ccy.c

###

papi_ful_ccy:	papi_ful_ccy.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ful_ccy papi_ful_ccy.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ful_ccy.o:  papi_ful_ccy.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ful_ccy.c

###

papi_fma_ins:	papi_fma_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fma_ins papi_fma_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fma_ins.o:  papi_fma_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fma_ins.c

###

papi_tot_iis:	papi_tot_iis.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_tot_iis papi_tot_iis.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_tot_iis.o:  papi_tot_iis.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_tot_iis.c

###

papi_int_ins:	papi_int_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_int_ins papi_int_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_int_ins.o:  papi_int_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_int_ins.c

###

papi_ld_ins:	papi_ld_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_ld_ins papi_ld_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_ld_ins.o:  papi_ld_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_ld_ins.c

###

papi_sr_ins:	papi_sr_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_sr_ins papi_sr_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_sr_ins.o:  papi_sr_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_sr_ins.c


###

papi_vec_ins:	papi_vec_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_vec_ins papi_vec_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_vec_ins.o:  papi_vec_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_vec_ins.c

###

papi_res_stl:	papi_res_stl.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_res_stl papi_res_stl.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_res_stl.o:  papi_res_stl.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_res_stl.c

###

papi_fp_stal:	papi_fp_stal.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fp_stal papi_fp_stal.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fp_stal.o:  papi_fp_stal.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fp_stal.c

###

papi_lst_ins:	papi_lst_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_lst_ins papi_lst_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_lst_ins.o:  papi_lst_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_lst_ins.c

###

papi_syc_ins:	papi_syc_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_syc_ins papi_syc_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_syc_ins.o:  papi_syc_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_syc_ins.c

###

papi_l1_dch:	papi_l1_dch.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_dch papi_l1_dch.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_dch.o:  papi_l1_dch.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_dch.c

###

papi_l2_dch:	papi_l2_dch.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_dch papi_l2_dch.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_dch.o:  papi_l2_dch.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_dch.c

###

papi_l3_dca:	papi_l3_dca.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_dca papi_l3_dca.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_dca.o:  papi_l3_dca.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_dca.c

###

papi_l1_dcr:	papi_l1_dcr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_dcr papi_l1_dcr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_dcr.o:  papi_l1_dcr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_dcr.c

###

papi_l3_dcr:	papi_l3_dcr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_dcr papi_l3_dcr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_dcr.o:  papi_l3_dcr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_dcr.c

###

papi_l1_dcw:	papi_l1_dcw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_dcw papi_l1_dcw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_dcw.o:  papi_l1_dcw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_dcw.c

###

papi_l2_dcw:	papi_l2_dcw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_dcw papi_l2_dcw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_dcw.o:  papi_l2_dcw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_dcw.c

###

papi_l3_dcw:	papi_l3_dcw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_dcw papi_l3_dcw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_dcw.o:  papi_l3_dcw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_dcw.c

###

papi_l1_ich:	papi_l1_ich.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_ich papi_l1_ich.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_ich.o:  papi_l1_ich.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_ich.c

###

papi_l2_ich:	papi_l2_ich.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_ich papi_l2_ich.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_ich.o:  papi_l2_ich.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_ich.c

###

papi_l3_ich:	papi_l3_ich.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_ich papi_l3_ich.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_ich.o:  papi_l3_ich.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_ich.c

###

papi_l1_ica:	papi_l1_ica.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_ica papi_l1_ica.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_ica.o:  papi_l1_ica.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_ica.c

###

papi_l2_ica:	papi_l2_ica.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_ica papi_l2_ica.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_ica.o:  papi_l2_ica.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_ica.c

###

papi_l3_ica:	papi_l3_ica.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_ica papi_l3_ica.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_ica.o:  papi_l3_ica.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_ica.c

###

papi_l1_icr:	papi_l1_icr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_icr papi_l1_icr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_icr.o:  papi_l1_icr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_icr.c

###

papi_l2_icr:	papi_l2_icr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_icr papi_l2_icr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_icr.o:  papi_l2_icr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_icr.c

###

papi_l3_icr:	papi_l3_icr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_icr papi_l3_icr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_icr.o:  papi_l3_icr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_icr.c

###

papi_l1_icw:	papi_l1_icw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_icw papi_l1_icw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_icw.o:  papi_l1_icw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_icw.c

###

papi_l2_icw:	papi_l2_icw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_icw papi_l2_icw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_icw.o:  papi_l2_icw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_icw.c

###

papi_l3_icw:	papi_l3_icw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_icw papi_l3_icw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_icw.o:  papi_l3_icw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_icw.c

###

papi_l1_tch:	papi_l1_tch.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_tch papi_l1_tch.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_tch.o:  papi_l1_tch.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_tch.c

###

papi_l2_tch:	papi_l2_tch.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_tch papi_l2_tch.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_tch.o:  papi_l2_tch.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_tch.c

###

papi_l3_tch:	papi_l3_tch.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_tch papi_l3_tch.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_tch.o:  papi_l3_tch.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_tch.c

###

papi_l1_tca:	papi_l1_tca.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_tca papi_l1_tca.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_tca.o:  papi_l1_tca.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_tca.c

###

papi_l2_tca:	papi_l2_tca.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_tca papi_l2_tca.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_tca.o:  papi_l2_tca.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_tca.c

###

papi_l3_tca:	papi_l3_tca.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_tca papi_l3_tca.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_tca.o:  papi_l3_tca.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_tca.c

###

papi_l1_tcr:	papi_l1_tcr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_tcr papi_l1_tcr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_tcr.o:  papi_l1_tcr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_tcr.c

###

papi_l3_tcr:	papi_l3_tcr.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_tcr papi_l3_tcr.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_tcr.o:  papi_l3_tcr.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_tcr.c

###

papi_l1_tcw:	papi_l1_tcw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l1_tcw papi_l1_tcw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l1_tcw.o:  papi_l1_tcw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l1_tcw.c

###

papi_l2_tcw:	papi_l2_tcw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l2_tcw papi_l2_tcw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l2_tcw.o:  papi_l2_tcw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l2_tcw.c

###

papi_l3_tcw:	papi_l3_tcw.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_l3_tcw papi_l3_tcw.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_l3_tcw.o:  papi_l3_tcw.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_l3_tcw.c

###

papi_fml_ins:	papi_fml_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fml_ins papi_fml_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fml_ins.o:  papi_fml_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fml_ins.c

###

papi_fad_ins:	papi_fad_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fad_ins papi_fad_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fad_ins.o:  papi_fad_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fad_ins.c

###

papi_fdv_ins:	papi_fdv_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fdv_ins papi_fdv_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fdv_ins.o:  papi_fdv_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fdv_ins.c

###

papi_fsq_ins:	papi_fsq_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fsq_ins papi_fsq_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fsq_ins.o:  papi_fsq_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fsq_ins.c

###

papi_fnv_ins:	papi_fnv_ins.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_fnv_ins papi_fnv_ins.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_fnv_ins.o:  papi_fnv_ins.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_fnv_ins.c

###

papi_sp_ops:	papi_sp_ops.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_sp_ops papi_sp_ops.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_sp_ops.o:  papi_sp_ops.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_sp_ops.c

###

papi_dp_ops:	papi_dp_ops.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_dp_ops papi_dp_ops.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_dp_ops.o:  papi_dp_ops.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_dp_ops.c

###

papi_vec_sp:	papi_vec_sp.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_vec_sp papi_vec_sp.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_vec_sp.o:  papi_vec_sp.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_vec_sp.c

###

papi_vec_dp:	papi_vec_dp.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_vec_dp papi_vec_dp.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_vec_dp.o:  papi_vec_dp.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_vec_dp.c

###

papi_bgl_oed:	papi_bgl_oed.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_bgl_oed papi_bgl_oed.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_bgl_oed.o:  papi_bgl_oed.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_bgl_oed.c

###

papi_bgl_ts_32b:	papi_bgl_ts_32b.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_bgl_ts_32b papi_bgl_ts_32b.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_bgl_ts_32b.o:  papi_bgl_ts_32b.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_bgl_ts_32b.c

###

papi_bgl_ts_full:	papi_bgl_ts_full.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_bgl_ts_full papi_bgl_ts_full.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_bgl_ts_full.o:  papi_bgl_ts_full.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_bgl_ts_full.c

###

papi_bgl_tr_dpkt:	papi_bgl_tr_dpkt.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_bgl_tr_dpkt papi_bgl_tr_dpkt.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_bgl_tr_dpkt.o:  papi_bgl_tr_dpkt.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_bgl_tr_dpkt.c

###

papi_bgl_tr_full:	papi_bgl_tr_full.o ../lib/test_utils.o
	$(CC) $(LFLAGS) -o papi_bgl_tr_full papi_bgl_tr_full.o \
		../lib/test_utils.o $(PAPI_LIB)

papi_bgl_tr_full.o:  papi_bgl_tr_full.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_bgl_tr_full.c



###

check_mhz_value:	check_mhz_value.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o check_mhz_value check_mhz_value.o \
		../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

check_mhz_value.o:	check_mhz_value.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c check_mhz_value.c



###

check_papi_latency:	check_papi_latency.o ../lib/test_utils.o ../lib/instructions_testcode.o
	$(CC) $(LFLAGS) -o check_papi_latency check_papi_latency.o \
		../lib/test_utils.o ../lib/instructions_testcode.o $(PAPI_LIB) -lrt

check_papi_latency.o:	check_papi_latency.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c check_papi_latency.c

####

core2_constraints:	core2_constraints.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o core2_constraints core2_constraints.o \
		../lib/test_utils.o ../lib/matrix_multiply.o \
		$(PAPI_LIB)


core2_constraints.o:	core2_constraints.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c core2_constraints.c


####

unregister:	unregister.o ../lib/test_utils.o 
	$(CC) $(LFLAGS) -o unregister unregister.o \
		../lib/test_utils.o \
		$(PAPI_LIB) -lpthread


unregister.o:	unregister.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c unregister.c

####

event_create_race:	event_create_race.o ../lib/test_utils.o 
	$(CC) $(LFLAGS) -o event_create_race event_create_race.o \
		../lib/test_utils.o \
		$(PAPI_LIB) -lpthread


event_create_race.o:	event_create_race.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c event_create_race.c


####

mpx_oflo:	mpx_oflo.o ../lib/test_utils.o  ../lib/matrix_multiply.o ../lib/papi_helpers.o
	$(CC) $(LFLAGS) -o mpx_oflo mpx_oflo.o \
		../lib/test_utils.o ../lib/matrix_multiply.o \
		$(PAPI_LIB) ../lib/papi_helpers.o


mpx_oflo.o:	mpx_oflo.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c mpx_oflo.c


###

cpufreq_scaling:	cpufreq_scaling.o ../lib/test_utils.o \
		../lib/matrix_multiply.o ../lib/nops_testcode.o
	$(CC) $(LFLAGS) -o cpufreq_scaling cpufreq_scaling.o \
		../lib/test_utils.o \
		../lib/matrix_multiply.o ../lib/nops_testcode.o $(PAPI_LIB)

cpufreq_scaling.o:	cpufreq_scaling.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c cpufreq_scaling.c

###

offcore_response:	offcore_response.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o offcore_response offcore_response.o ../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

offcore_response.o:	offcore_response.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c offcore_response.c	


###

perfevent_sw_events:	perfevent_sw_events.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o perfevent_sw_events perfevent_sw_events.o  ../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

perfevent_sw_events.o:	perfevent_sw_events.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c perfevent_sw_events.c


###

perfevent_generalized_events:	perfevent_generalized_events.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o perfevent_generalized_events perfevent_generalized_events.o  ../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

perfevent_generalized_events.o:	perfevent_generalized_events.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c perfevent_generalized_events.c


###

papi_profile:	papi_profile.o
	$(CC) $(LFLAGS) $(PAPI_LIB) -o papi_profile papi_profile.o

papi_profile.o:	papi_profile.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_profile.c


clean:
	rm -f *.o core *~ \
	papi_bgl_oed \
	papi_bgl_tr_dpkt \
	papi_bgl_tr_full \
	papi_bgl_ts_32b \
	papi_bgl_ts_full \
	papi_br_cn \
	papi_br_ins \
	papi_br_ins \
	papi_br_msp \
	papi_br_ntk \
	papi_br_prc \
	papi_br_tkn \
	papi_br_ucn \
	papi_bru_idl \
	papi_btac_m \
	papi_ca_cln \
	papi_ca_inv \
	papi_ca_itv \
	papi_ca_shr \
	papi_ca_snp \
	papi_csr_fal \
	papi_csr_suc \
	papi_csr_tot \
	papi_dp_ops \
	papi_fad_ins \
	papi_fdv_ins \
	papi_fma_ins \
	papi_fml_ins \
	papi_fnv_ins \
	papi_fp_ins \
	papi_fp_ops \
	papi_fp_stal \
	papi_fpu_idl \
	papi_fsq_ins \
	papi_ful_ccy \
	papi_ful_icy \
	papi_fxu_idl \
	papi_hw_int \
	papi_int_ins \
	papi_l1_dca \
	papi_l1_dch \
	papi_l1_dcm \
	papi_l1_dcr \
	papi_l1_dcw \
	papi_l1_ica \
	papi_l1_ich \
	papi_l1_icm \
	papi_l1_icr \
	papi_l1_icw \
	papi_l1_ldm \
	papi_l1_stm \
	papi_l1_tca \
	papi_l1_tch \
	papi_l1_tcm \
	papi_l1_tcr \
	papi_l1_tcw \
	papi_l2_dca \
	papi_l2_dch \
	papi_l2_dcm \
	papi_l2_dcr \
	papi_l2_dcw \
	papi_l2_ica \
	papi_l2_ich \
	papi_l2_icm \
	papi_l2_icr \
	papi_l2_icw \
	papi_l2_ldm \
	papi_l2_stm \
	papi_l2_tca \
	papi_l2_tch \
	papi_l2_tcm \
	papi_l2_tcr \
	papi_l2_tcw \
	papi_l3_dca \
	papi_l3_dch \
	papi_l3_dcm \
	papi_l3_dcr \
	papi_l3_dcw \
	papi_l3_ica \
	papi_l3_ich \
	papi_l3_icm \
	papi_l3_icr \
	papi_l3_icw \
	papi_l3_ldm \
	papi_l3_stm \
	papi_l3_tca \
	papi_l3_tch \
	papi_l3_tcm \
	papi_l3_tcr \
	papi_l3_tcw \
	papi_ld_ins \
	papi_lst_ins \
	papi_lsu_idl \
	papi_mem_rcy \
	papi_mem_scy \
	papi_mem_wcy \
	papi_prf_dm \
	papi_res_stl \
	papi_sp_ops \
	papi_sr_ins \
	papi_stl_ccy \
	papi_stl_icy \
	papi_syc_ins \
	papi_tlb_dm \
	papi_tlb_im \
	papi_tlb_sd \
	papi_tlb_tl \
	papi_tot_cyc \
	papi_tot_iis \
	papi_tot_ins \
	papi_vec_dp \
	papi_vec_ins \
	papi_vec_sp \
	check_mhz_value \
	check_papi_latency \
	core2_constraints \
	cpufreq_scaling \
	unregister \
	event_create_race \
	mpx_oflo \
	offcore_response \
	perfevent_sw_events \
	perfevent_generalized_events \
	papi_profile

install:	all
