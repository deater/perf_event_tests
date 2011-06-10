CC	= gcc
CC_R	= $(CC) -pthread
CFLAGS	= -g -O2 -Wall -I../include

#PAPI_INCLUDE = -I/home/vweaver1/research/papi/papi.cvs.perfctr/src
#PAPI_LIB = /home/vweaver1/research/papi/papi.cvs.perfctr/src/libpapi.a

PAPI_INCLUDE = -I/usr/local/lib
PAPI_LIB = /usr/local/lib/libpapi.a


all:	core2_constraints \
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
	papi_fp_ins \
	papi_fp_ops \
	papi_hw_int \
	papi_l1_dca \
	papi_l2_dca \
	papi_l2_dcm \
	papi_l2_dcr \
	papi_l2_tcm \
	papi_l2_tcr \
	papi_tot_cyc \
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


papi_l2_dca:	papi_l2_dca.o cache_test.o
	$(CC) $(LFLAGS) -o papi_l2_dca papi_l2_dca.o cache_test.o $(PAPILIB) $(UTILS)

papi_l2_dca.o:	papi_l2_dca.c
	$(CC) $(CFLAGS) $(INCLUDE) -c papi_l2_dca.c


papi_l2_dcm:	papi_l2_dcm.o cache_test.o
	$(CC) $(LFLAGS) -o papi_l2_dcm papi_l2_dcm.o cache_test.o $(PAPILIB) $(UTILS)

papi_l2_dcm.o:	papi_l2_dcm.c
	$(CC) $(CFLAGS) $(INCLUDE) -c papi_l2_dcm.c


papi_l2_dcr:	papi_l2_dcr.o cache_test.o
	$(CC) $(LFLAGS) -o papi_l2_dcr papi_l2_dcr.o cache_test.o $(PAPILIB) $(UTILS)

papi_l2_dcr.o:	papi_l2_dcr.c
	$(CC) $(CFLAGS) $(INCLUDE) -c papi_l2_dcr.c


papi_l2_tcm:	papi_l2_tcm.o cache_test.o
	$(CC) $(LFLAGS) -o papi_l2_tcm papi_l2_tcm.o cache_test.o $(PAPILIB) $(UTILS)

papi_l2_tcm.o:	papi_l2_tcm.c
	$(CC) $(CFLAGS) $(INCLUDE) -c papi_l2_tcm.c


papi_l2_tcr:	papi_l2_tcr.o cache_test.o
	$(CC) $(LFLAGS) -o papi_l2_tcr papi_l2_tcr.o cache_test.o $(PAPILIB) $(UTILS)

papi_l2_tcr.o:	papi_l2_tcr.c
	$(CC) $(CFLAGS) $(INCLUDE) -c papi_l2_tcr.c


papi_cache_info.o:	papi_cache_info.c
	$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c papi_cache_info.c


papi_tot_cyc:	papi_tot_cyc.o
	$(CC) $(LFLAGS) -o papi_tot_cyc papi_tot_cyc.o $(PAPILIB) $(UTILS)

papi_tot_cyc.o:	papi_tot_cyc.c
	$(CC) $(CFLAGS) $(INCLUDE) -c papi_tot_cyc.c

papi_tot_ins:	papi_tot_ins.o
	$(CC) $(LFLAGS) -o papi_tot_ins papi_tot_ins.o $(PAPILIB) $(UTILS)

papi_tot_ins.o:	papi_tot_ins.c
	$(CC) $(CFLAGS) $(INCLUDE) -c papi_tot_ins.c



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
	papi_hw_int \
	papi_l1_dca \
	papi_l2_dca \
	papi_l2_dcm \
	papi_l2_dcr \
	papi_l2_tcm \
	papi_l2_tcr \
	papi_tot_cyc \
	papi_tot_ins \
	papi_fp_ins \
	papi_fp_ops \
	core2_constraints \
	nhm_offcore_response \
	perfevent_sw_events \
	perfevent_generalized_events

install:	all
