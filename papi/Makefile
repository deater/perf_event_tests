CC	= gcc
CC_R	= $(CC) -pthread
CFLAGS	= -g -O2 -Wall -I../include

PAPI_INCLUDE = -I/home/vweaver1/research/papi/papi.cvs.perfctr/src
PAPI_LIB = /home/vweaver1/research/papi/papi.cvs.perfctr/src/libpapi.a

#PAPI_INCLUDE = -I/usr/local/lib
#PAPI_LIB = /usr/local/lib/libpapi.a


all:	check_mhz_value \
	check_papi_latency \
	core2_constraints \
	nhm_offcore_response \
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
	papi_fp_ins \
	papi_fp_ops \
	papi_fpu_idl \
	papi_fxu_idl \
	papi_hw_int \
	papi_l1_dca \
	papi_l1_dcm \
	papi_l1_icm \
	papi_l1_ldm \
	papi_l1_stm \
	papi_l1_tcm \
	papi_l2_dca \
	papi_l2_dcm \
	papi_l2_dcr \
	papi_l2_icm \
	papi_l2_ldm \
	papi_l2_stm \
	papi_l2_tcm \
	papi_l2_tcr \
	papi_l3_dch \
	papi_l3_dcm \
	papi_l3_icm \
	papi_l3_ldm \
	papi_l3_stm \
	papi_l3_tcm \
	papi_lsu_idl \
	papi_prf_dm \
	papi_tot_cyc \
	papi_tlb_dm \
	papi_tlb_im \
	papi_tlb_sd \
	papi_tlb_tl \
	papi_tot_ins 

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


###

nhm_offcore_response:	nhm_offcore_response.o ../lib/test_utils.o ../lib/matrix_multiply.o
	$(CC) $(LFLAGS) -o nhm_offcore_response nhm_offcore_response.o ../lib/test_utils.o ../lib/matrix_multiply.o $(PAPI_LIB)

nhm_offcore_response.o:	nhm_offcore_response.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c nhm_offcore_response.c	


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

clean:
	rm -f *.o core *~ \
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
	papi_fp_ins \
	papi_fp_ops \
	papi_fpu_idl \
	papi_fxu_idl \
	papi_hw_int \
	papi_l1_dca \
	papi_l1_dcm \
	papi_l1_icm \
	papi_l1_ldm \
	papi_l1_stm \
	papi_l1_tcm \
	papi_l2_dca \
	papi_l2_dcm \
	papi_l2_dcr \
	papi_l2_icm \
	papi_l2_ldm \
	papi_l2_stm \
	papi_l2_tcm \
	papi_l2_tcr \
	papi_l3_dch \
	papi_l3_dcm \
	papi_l3_icm \
	papi_l3_ldm \
	papi_l3_stm \
	papi_l3_tcm \
	papi_lsu_idl \
	papi_prf_dm \
	papi_tlb_dm \
	papi_tlb_im \
	papi_tlb_sd \
	papi_tlb_tl \
	papi_tot_cyc \
	papi_tot_ins \
	check_mhz_value \
	check_papi_latency \
	core2_constraints \
	nhm_offcore_response \
	perfevent_sw_events \
	perfevent_generalized_events

install:	all
