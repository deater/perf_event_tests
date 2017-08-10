#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <stdlib.h>

#include <sys/utsname.h>

#include "perf_event.h"
#include "perf_helpers.h"

#ifndef __NR_perf_event_open

#if defined(__i386__)
#define __NR_perf_event_open    336
#elif defined(__x86_64__)
#define __NR_perf_event_open    298
#elif defined __powerpc__
#define __NR_perf_event_open    319
#elif defined __arm__
#define __NR_perf_event_open    364
#endif
#endif

int perf_event_open(struct perf_event_attr *hw_event_uptr,
		    pid_t pid, int cpu, int group_fd, unsigned long flags) {

	return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
			group_fd, flags);
}

pid_t mygettid( void )
{
#ifdef SYS_gettid
  return syscall( SYS_gettid );
#elif defined(__NR_gettid)
  return syscall( __NR_gettid );
#else
#error "cannot find gettid"
#endif
}

static int processor_type=-2;
static int processor_vendor=-2;

static int detect_processor_cpuinfo(void) {

	FILE *fff;
	int cpu_family=0,model=0;
	char string[BUFSIZ];

	fff=fopen("/proc/cpuinfo","r");
	if (fff==NULL) {
		fprintf(stderr,"ERROR!  Can't open /proc/cpuinfo\n");
		return PROCESSOR_UNKNOWN;
	}

	while(1) {

		if (fgets(string,BUFSIZ,fff)==NULL) break;

		/* Power6 */
		if (strstr(string,"POWER6")) {
			processor_vendor=VENDOR_IBM;
			processor_type=PROCESSOR_POWER6;
			return 0;
		}

		/* ARM */
		if (strstr(string,"CPU part")) {

			processor_vendor=VENDOR_ARM;

			if (strstr(string,"0xb76")) {
				processor_type=PROCESSOR_ARM1176;
				return 0;
			}

			if (strstr(string,"0xc05")) {
				processor_type=PROCESSOR_CORTEX_A5;
				return 0;
			}
			if (strstr(string,"0xc09")) {
				processor_type=PROCESSOR_CORTEX_A9;
				return 0;
			}
			if (strstr(string,"0xc08")) {
				processor_type=PROCESSOR_CORTEX_A8;
				return 0;
			}
			if (strstr(string,"0xc07")) {
				processor_type=PROCESSOR_CORTEX_A7;
				return 0;
			}
			if (strstr(string,"0xc0f")) {
				processor_type=PROCESSOR_CORTEX_A15;
				return 0;
			}

			if (strstr(string,"0xd03")) {
				processor_type=PROCESSOR_CORTEX_A53;
				return 0;
			}

			// Cortex R4 - 0xc14
			// Cortex R5 - 0xc15
			// ARM1136 - 0xb36
			// ARM1156 - 0xb56
			// ARM1176 - 0xb76
			// ARM11 MPCore - 0xb02

		}

		/* vendor */
		if (strstr(string,"vendor_id")) {
			if (strstr(string,"GenuineIntel")) {
				processor_vendor=VENDOR_INTEL;
			}
			if (strstr(string,"AuthenticAMD")) {
				processor_vendor=VENDOR_AMD;
			}
		}

		/* family */
		if (strstr(string,"cpu family")) {
			sscanf(string,"%*s %*s %*s %d",&cpu_family);
		}

		/* model */
		if ((strstr(string,"model")) && (!strstr(string,"model name")) ) {
			sscanf(string,"%*s %*s %d",&model);
		}
	}

	fclose(fff);

	if (processor_vendor==VENDOR_AMD) {
		switch(cpu_family) {
			case 0x6:
				processor_type=PROCESSOR_K7;
				break;
			case 0xf:
				processor_type=PROCESSOR_K8;
				break;
			case 0x10:
				processor_type=PROCESSOR_AMD_FAM10H;
				break;
			case 0x11:
				processor_type=PROCESSOR_AMD_FAM11H;
				break;
			case 0x12:
				processor_type=PROCESSOR_AMD_FAM12H;
				break;
			case 0x13:
				processor_type=PROCESSOR_AMD_FAM13H;
				break;
			case 0x14:
				processor_type=PROCESSOR_AMD_FAM14H;
				break;
			case 0x15:
				processor_type=PROCESSOR_AMD_FAM15H;
				break;
			case 0x16:
				processor_type=PROCESSOR_AMD_FAM16H;
				break;
			case 0x17:
				processor_type=PROCESSOR_AMD_FAM17H;
				break;
			default:
				processor_type=PROCESSOR_UNKNOWN;
				break;
		}
		return 0;
	}

	if (processor_vendor==VENDOR_INTEL) {

		if (cpu_family==6) {
			switch(model) {
				case 1:
					processor_type=PROCESSOR_PENTIUM_PRO;
					break;
				case 3:
				case 5:
				case 6:
					processor_type=PROCESSOR_PENTIUM_II;
					break;
				case 7:
				case 8:
				case 10:
				case 11:
					processor_type=PROCESSOR_PENTIUM_III;
					break;
				case 9:
				case 13:
					processor_type=PROCESSOR_PENTIUM_M;
					break;
				case 14:
					processor_type=PROCESSOR_COREDUO;
					break;
				case 15:
				case 22:
				case 23:
				case 29:
					processor_type=PROCESSOR_CORE2;
					break;
				case 28:
				case 38:
				case 39:
				case 53:
					processor_type=PROCESSOR_ATOM;
					break;
				case 54:
					processor_type=PROCESSOR_ATOM_CEDARVIEW;
					break;
				case 55:
				case 77:
					processor_type=PROCESSOR_ATOM_SILVERMONT;
					break;
				case 26:
				case 30:
				case 31:
					processor_type=PROCESSOR_NEHALEM;
					break;
				case 46:
					processor_type=PROCESSOR_NEHALEM_EX;
					break;
				case 37:
				case 44:
					processor_type=PROCESSOR_WESTMERE;
					break;
				case 47:
					processor_type=PROCESSOR_WESTMERE_EX;
					break;
				case 42:
					processor_type=PROCESSOR_SANDYBRIDGE;
					break;
				case 45:
					processor_type=PROCESSOR_SANDYBRIDGE_EP;
					break;
				case 58:
					processor_type=PROCESSOR_IVYBRIDGE;
					break;
				case 62:
					processor_type=PROCESSOR_IVYBRIDGE_EP;
					break;
				case 60:
				case 69:
				case 70:
					processor_type=PROCESSOR_HASWELL;
					break;
				case 63:
					processor_type=PROCESSOR_HASWELL_EP;
					break;
				case 61:
				case 71:
				case 79:
				case 86:
					processor_type=PROCESSOR_BROADWELL;
					break;
				case 78:
				case 85:
				case 94:
					processor_type=PROCESSOR_SKYLAKE;
					break;
				case 142:
				case 158:
					processor_type=PROCESSOR_KABYLAKE;
					break;


				default:
					processor_type=PROCESSOR_UNKNOWN;
			}
			return 0;
		}
		if (cpu_family==11) {
			processor_type=PROCESSOR_KNIGHTSCORNER;
			return 0;
		}

		if (cpu_family==15) {
			processor_type=PROCESSOR_PENTIUM_4;
			return 0;
		}
	}

	processor_type=PROCESSOR_UNKNOWN;

	return 0;
}


int detect_processor(void) {
	if (processor_type==-2) {
		detect_processor_cpuinfo();
	}
	return processor_type;
}

int detect_vendor(void) {

	if (processor_vendor==-2) {
		detect_processor_cpuinfo();
	}
	return processor_vendor;

}

void arch_adjust_domain(struct perf_event_attr *pe, int quiet) {

#ifdef __arm__

	int processor=detect_processor();

	if ((processor==PROCESSOR_CORTEX_A15) ||
		(processor==PROCESSOR_CORTEX_A7) ||
		(processor==PROCESSOR_CORTEX_A53)) {

	}
	else {
		/* older PMUs do not support exclude_user / exclude_kernel */
		pe->exclude_user=0;
		pe->exclude_kernel=0;
		pe->exclude_hv=0;

		if (!quiet) {
			printf("Adjusting domain to 0,0,0 for ARM %d\n",processor);
		}
	}
#endif
}


static int core2_events[MAX_TEST_EVENTS]={
  0x53003c, //"UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, //"INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5300c4, //"BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5300c5, //"MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x531282, //"ITLB:MISSES",                  /* PAPI_TLB_IM  */
  0x530108, //"DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  0x530080, //"L1I_READS",                    /* PAPI_L1_ICA  */
  0x530081, //"L1I_MISSES",                   /* PAPI_L1_ICM  */
  0x530143, //"L1D_ALL_REF",                  /* PAPI_L1_DCA  */
  0x530f45, //"L1D_REPL",                     /* PAPI_L1_DCM  */
  0x5300c8, //"HW_INT_RCV",                   /* PAPI_HW_INT  */
  0x530010, //"FP_COMP_OPS_EXE",              /* PAPI_FP_OPS  */
  0x5301c0, //"INST_RETIRED:LOADS",           /* PAPI_LD_INS  */
  0x5302c0, //"INST_RETIRED:STORES",          /* PAPI_SR_INS  */
  0x537f2e, //"L2_RQSTS:SELF:ANY:MESI",       /* PAPI_L2_TCA  */
  0x537024, //"L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

static int atom_events[MAX_TEST_EVENTS]={
  0x53003c, //"UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5300c4, // "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5300c5, // "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x530282, // "ITLB:MISSES",                  /* PAPI_TLB_IM  */
  0x530708, // "DATA_TLB_MISSES:DTLB_MISS",    /* PAPI_TLB_DM  */
  0x530380, //"ICACHE:ACCESSES",              /* PAPI_L1_ICA  */
  0x530280, // "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  0x532140, // "L1D_CACHE:LD",                 /* PAPI_L1_DCA  */
  0x537f2e, // "L2_RQSTS:SELF",                /* PAPI_L1_DCM  */
  0x5301c8, //"HW_INT_RCV",                   /* PAPI_HW_INT  */
  0x531fc7, // "SIMD_INST_RETIRED:ANY",        /* PAPI_FP_OPS  */
  0x537f29, //"L2_LD:SELF:ANY:MESI",          /* PAPI_LD_INS  */
  0x534f2a, //"L2_ST:SELF:MESI",              /* PAPI_SR_INS  */
  0x537f29, //"L2_LD:SELF:ANY:MESI",          /* PAPI_L2_TCA  */
  0x537024, //"L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

static int amd10h_events[MAX_TEST_EVENTS]={
  0x530076, // "CPU_CLK_UNHALTED",                         /* PAPI_TOT_CYC */
  0x5300c0, // "RETIRED_INSTRUCTIONS",                     /* PAPI_TOT_INS */
  0x5300c2, // "RETIRED_BRANCH_INSTRUCTIONS",              /* PAPI_BR_INS  */
  0x5300c3, // "RETIRED_MISPREDICTED_BRANCH_INSTRUCTIONS", /* PAPI_BR_MSP  */
  0x530385, // "L1_ITLB_MISS_AND_L2_ITLB_MISS:ALL",        /* PAPI_TLB_IM  */
  0x530746, // "L1_DTLB_AND_L2_DTLB_MISS",                 /* PAPI_TLB_DM  */
  0x530080, // "INSTRUCTION_CACHE_FETCHES",                /* PAPI_L1_ICA  */
  0x530081, // "INSTRUCTION_CACHE_MISSES",                 /* PAPI_L1_ICM  */
  0x530040, // "DATA_CACHE_ACCESSES",                      /* PAPI_L1_DCA  */
  0x530041, // "DATA_CACHE_MISSES",                        /* PAPI_L1_DCM  */
  0x5300cf, // "INTERRUPTS_TAKEN",                         /* PAPI_HW_INT  */
  0x530300, // "DISPATCHED_FPU:OPS_MULTIPLY:OPS_ADD",      /* PAPI_FP_OPS  */
  0x5300d0, // "DECODER_EMPTY",                            /* PAPI_LD_INS  */ /* nope */
  0x5300d1, // "DISPATCH_STALLS",                          /* PAPI_SR_INS  */ /* nope */
  0x533f7d, // "REQUESTS_TO_L2:ALL",                       /* PAPI_L2_TCA  */
  0x53037e, // "L2_CACHE_MISS:INSTRUCTIONS:DATA",          /* PAPI_L2_TCM  */
};

static int nehalem_events[MAX_TEST_EVENTS]={
  0x53003c, // "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x537f88, // "BR_INST_EXEC:ANY",             /* PAPI_BR_INS  */
  0x537f89, // "BR_MISP_EXEC:ANY",             /* PAPI_BR_MSP  */
  0x530185, // "ITLB_MISSES:ANY",              /* PAPI_TLB_IM  */
  0x530149, // "DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  0x530380, // "L1I:READS",                    /* PAPI_L1_ICA  */
  0x530280, // "L1I:MISSES",                   /* PAPI_L1_ICM  */
  0x530143, // "L1D_ALL_REF:ANY",              /* PAPI_L1_DCA  */
  0x530151, // "L1D:REPL",                     /* PAPI_L1_DCM  */
  0x53011d, // "HW_INT:RCV",                   /* PAPI_HW_INT  */
  0x530410, // "FP_COMP_OPS_EXE:SSE_FP",       /* PAPI_FP_OPS  */
  0x53010b, // "MEM_INST_RETIRED:LOADS",       /* PAPI_LD_INS  */
  0x53020b, // "MEM_INST_RETIRED:STORES",      /* PAPI_SR_INS  */
  0x53ff24, // "L2_RQSTS:REFERENCES",          /* PAPI_L2_TCA  */
  0x53c024, // "L2_RQSTS:PREFETCHES",          /* PAPI_L2_TCM  */
};

static int ivybridge_events[MAX_TEST_EVENTS]={
  0x53003c, // "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5304c4, // "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5304c5, // "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x530185, // "ITLB_MISSES:CAUSES_A_WALK",    /* PAPI_TLB_IM  */
  0x538108, // "DTLB_LOAD_MISSES:MISS_CAUSES_A_WALK", /* PAPI_TLB_DM  */
  0x531024, // "L2_RQSTS:CODE_RD_HIT",         /* PAPI_L1_ICA  */ /* nope */
  0x530280, // "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  0x532024, // "L2_RQSTS:CODE_RD_MISS"         /* PAPI_L1_DCA  */ /* nope */
  0x530151, // "L1D:REPLACEMENT",              /* PAPI_L1_DCM  */
  0x5301cb, // "HW_INTERRUPTS",                /* PAPI_HW_INT  */
  0x531eca, // "FP_ASSIST:ANY",                /* PAPI_FP_OPS  */ /* nope */
  0x5381d0, // "MEM_UOP_RETIRED:ANY_LOADS",    /* PAPI_LD_INS  */
  0x5382d0, // "MEM_UOP_RETIRED:ANY_STORES",   /* PAPI_SR_INS  */
  0x533024, // "L2_RQSTS:ALL_CODE_RD",         /* PAPI_L2_TCA  */ /*not quite*/
  0x534f2e, // "LAST_LEVEL_CACHE_REFERENCES",  /* PAPI_L2_TCM  */
};

static int haswell_events[MAX_TEST_EVENTS]={
  0x53003c, // "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5300c4, // "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5300c5, // "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x530185, // "ITLB_MISSES:MISS_CAUSES_A_WALK",    /* PAPI_TLB_IM  */
  0x538108, // "DTLB_LOAD_MISSES:MISS_CAUSES_A_WALK", /* PAPI_TLB_DM  */
  0x534424, // "L2_RQSTS:CODE_RD_HIT",         /* PAPI_L1_ICA  */ /* nope */
  0x530280, // "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  0x532424, // "L2_RQSTS:CODE_RD_MISS"         /* PAPI_L1_DCA  */ /* nope */
  0x530151, // "L1D:REPLACEMENT",              /* PAPI_L1_DCM  */
  0x5301cb, // "HW_INTERRUPTS",                /* PAPI_HW_INT  */ /* not doc? */
  0x531eca, // "FP_ASSIST:ANY",                /* PAPI_FP_OPS  */ /* nope */
  0x5381d0, // "MEM_UOPS_RETIRED:ALL_LOADS",    /* PAPI_LD_INS  */
  0x5382d0, // "MEM_UOPS_RETIRED:ALL_STORES",   /* PAPI_SR_INS  */
  0x53e424, // "L2_RQSTS:ALL_CODE_RD",         /* PAPI_L2_TCA  */ /*not quite*/
  0x534f2e, // "LONGEST_LAT_CACHE:REFERENCE",  /* PAPI_L2_TCM  */
};

static int broadwell_events[MAX_TEST_EVENTS]={
  0x53003c, // "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5300c4, // "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5300c5, // "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x530185, // "ITLB_MISSES:MISS_CAUSES_A_WALK",    /* PAPI_TLB_IM  */
  0x538108, // "DTLB_LOAD_MISSES:MISS_CAUSES_A_WALK", /* PAPI_TLB_DM  */
  0x534424, // "L2_RQSTS:CODE_RD_HIT",         /* PAPI_L1_ICA  */ /* nope */
  0x530280, // "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  0x532424, // "L2_RQSTS:CODE_RD_MISS"         /* PAPI_L1_DCA  */ /* nope */
  0x530151, // "L1D:REPLACEMENT",              /* PAPI_L1_DCM  */
  0x5301cb, // "HW_INTERRUPTS",                /* PAPI_HW_INT  */ /* not doc? */
  0x531eca, // "FP_ASSIST:ANY",                /* PAPI_FP_OPS  */ /* nope */
  0x5381d0, // "MEM_UOPS_RETIRED:ALL_LOADS",    /* PAPI_LD_INS  */
  0x5382d0, // "MEM_UOPS_RETIRED:ALL_STORES",   /* PAPI_SR_INS  */
  0x53e424, // "L2_RQSTS:ALL_CODE_RD",         /* PAPI_L2_TCA  */ /*not quite*/
  0x534f2e, // "LONGEST_LAT_CACHE:REFERENCE",  /* PAPI_L2_TCM  */
};

static int skylake_events[MAX_TEST_EVENTS]={
  0x53003c, // "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  0x5300c0, // "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  0x5300c4, // "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  0x5300c5, // "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  0x530185, // "ITLB_MISSES:MISS_CAUSES_A_WALK",    /* PAPI_TLB_IM  */
  0x538108, // "DTLB_LOAD_MISSES:MISS_CAUSES_A_WALK", /* PAPI_TLB_DM  */
  0x534424, // "L2_RQSTS:CODE_RD_HIT",         /* PAPI_L1_ICA  */ /* nope */
  0x530280, // "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  0x532424, // "L2_RQSTS:CODE_RD_MISS"         /* PAPI_L1_DCA  */ /* nope */
  0x530151, // "L1D:REPLACEMENT",              /* PAPI_L1_DCM  */
  0x5301cb, // "HW_INTERRUPTS",                /* PAPI_HW_INT  */ /* not doc? */
  0x531eca, // "FP_ASSIST:ANY",                /* PAPI_FP_OPS  */ /* nope */
  0x5381d0, // "MEM_UOPS_RETIRED:ALL_LOADS",    /* PAPI_LD_INS  */
  0x5382d0, // "MEM_UOPS_RETIRED:ALL_STORES",   /* PAPI_SR_INS  */
  0x53e424, // "L2_RQSTS:ALL_CODE_RD",         /* PAPI_L2_TCA  */ /*not quite*/
  0x534f2e, // "LONGEST_LAT_CACHE:REFERENCE",  /* PAPI_L2_TCM  */
};



static int power6_events[MAX_TEST_EVENTS]={
  0x10000a, // "PM_RUN_CYC",         	       /* PAPI_TOT_CYC */
  0x2,      // "PM_INST_CMPL",         	       /* PAPI_TOT_INS */
  0x430e6,  // "PM_BRU_FIN",                   /* PAPI_BR_INS  */
  0x400052, // "PM_BR_MPRED",                  /* PAPI_BR_MSP  */
  0x2000fe, // "PM_DATA_FROM_L2MISS",          /* PAPI_TLB_IM  */  /* nope */
  0x2000fe, // "PM_DATA_FROM_L2MISS",          /* PAPI_TLB_DM  */  /* nope */
  0x80086,  // "PM_ST_REF_L1",                 /* PAPI_L1_ICA  */ /* nope */
  0x100056, // "PM_L1_ICACHE_MISS",            /* PAPI_L1_ICM  */
  0x80082,  // "PM_LD_REF_L1",                 /* PAPI_L1_DCA  */  /* nope */
  0x80080, // "PM_LD_MISS_L1",                 /* PAPI_L1_DCM  */  /* nope */
  0x2000f8, // "PM_EXT_INT",                   /* PAPI_HW_INT  */
  0x1c0032, // "PM_FPU_FLOP",       	       /* PAPI_FP_OPS  */
  0x80082,  // "PM_LD_REF_L1",       	       /* PAPI_LD_INS  */
  0x80086,   // "PM_ST_REF_L1",      	       /* PAPI_SR_INS  */
  0x80082,  // "PM_LD_REF_L1",                 /* PAPI_L2_TCA  */ /* nope */
  0x2000fe, // "PM_DATA_FROM_L2MISS",          /* PAPI_L2_TCM  */ /* nope */
};

static int cortexA9_events[MAX_TEST_EVENTS]={
  0xff, // "CPU_CYCLES",                    /* PAPI_TOT_CYC */
  0x68, // "INST_OUT_OF_RENAME_STAGE",      /* PAPI_TOT_INS */
  0xc,  //"PC_WRITE",                      /* PAPI_BR_INS  */
  0x10, // "PC_BRANCH_MIS_PRED",            /* PAPI_BR_MSP  */
  0x2,  // "ITLB_MISS",                     /* PAPI_TLB_IM  */
  0x5,  // "DTLB_REFILL",                   /* PAPI_TLB_DM  */
  0x70, // "MAIN_UNIT_EXECUTED_INST",       /* PAPI_L1_ICA  */ /* nope */
  0x1,  //"IFETCH_MISS",                   /* PAPI_L1_ICM  */
  0x4,  //"DCACHE_ACCESS",                  /* PAPI_L1_DCA  */
  0x3,  //"DCACHE_REFILL",                 /* PAPI_L1_DCM  */
  0x93, //"EXT_INTERRUPTS",                /* PAPI_HW_INT  */
  0x73, //"FP_EXECUTED_INST",              /* PAPI_FP_OPS  */
  0x6,  //"DREAD",                         /* PAPI_LD_INS  */
  0x7,  //"DWRITE",                        /* PAPI_SR_INS  */
  0x4,  //"DCACHE_ACCESS",                 /* PAPI_L2_TCA  */  /* nope */
  0x3,  //"DCACHE_REFILL",                 /* PAPI_L2_TCM  */  /* nope */
};


int copy_events(int *eventset) {

	int processor,processor_notfound=0;

	processor=detect_processor();

	switch(processor) {
	case PROCESSOR_ATOM:
		memcpy(eventset,atom_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_CORE2:
		memcpy(eventset,core2_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_NEHALEM:
		memcpy(eventset,nehalem_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_IVYBRIDGE:
		memcpy(eventset,ivybridge_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_HASWELL:
	case PROCESSOR_HASWELL_EP:
		memcpy(eventset,haswell_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_BROADWELL:
		memcpy(eventset,broadwell_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_SKYLAKE:
	case PROCESSOR_KABYLAKE:
		memcpy(eventset,skylake_events,MAX_TEST_EVENTS*sizeof(int));
		break;

	case PROCESSOR_K7:
	case PROCESSOR_K8:
	case PROCESSOR_AMD_FAM10H:
	case PROCESSOR_AMD_FAM11H:
	case PROCESSOR_AMD_FAM12H:
	case PROCESSOR_AMD_FAM13H:
	case PROCESSOR_AMD_FAM14H:
	case PROCESSOR_AMD_FAM15H:
	case PROCESSOR_AMD_FAM16H:
	case PROCESSOR_AMD_FAM17H:
		memcpy(eventset,amd10h_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_POWER6:
		memcpy(eventset,power6_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	case PROCESSOR_CORTEX_A8:
 		memcpy(eventset,cortexA9_events,MAX_TEST_EVENTS*sizeof(int));
		break;
	default:
		processor_notfound=1;
	}

	return processor_notfound;

}


int detect_nmi_watchdog(void) {

	int watchdog_detected=0,watchdog_value=0;
	FILE *fff;

	fff=fopen("/proc/sys/kernel/nmi_watchdog","r");
	if (fff!=NULL) {
		if (fscanf(fff,"%d",&watchdog_value)==1) {
			if (watchdog_value>0) watchdog_detected=1;
		}
		fclose(fff);
	}

	return watchdog_detected;
}

int get_kernel_version(void) {

	struct utsname info;
	int major=0,minor=0,subminor=0;
	char *ptr;

	uname(&info);

	ptr=strtok(info.release,".");

	if (ptr) major=atoi(ptr);

	ptr=strtok(NULL,".");

	if (ptr) minor=atoi(ptr);

	ptr=strtok(NULL,".");

	if (ptr) subminor=atoi(ptr);

	return (major<<16)|(minor<<8)|subminor;
}


int get_latency_load_event(unsigned long long *config,
			unsigned long long *config1,
			int *precise_ip,
			char *name) {

	int processor,processor_notfound=0;

	processor=detect_processor();

	switch(processor) {

	case PROCESSOR_NEHALEM:
	case PROCESSOR_NEHALEM_EX:
		*config=0x100b;
		*config1=0x3;
		*precise_ip=2;
		strcpy(name,"MEM_INST_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	/* env LIBPFM_ENCODE_INACTIVE=1 \
		./check_events wsm::MEM_INST_RETIRED:LATENCY_ABOVE_THRESHOLD */
	case PROCESSOR_WESTMERE:
	case PROCESSOR_WESTMERE_EX:
		*config=0x100b;
		*config1=0x3;
		*precise_ip=2;
		strcpy(name,"MEM_INST_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	case PROCESSOR_SANDYBRIDGE:
	case PROCESSOR_SANDYBRIDGE_EP:
		*config=0x1cd;
		*config1=0x3;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	case PROCESSOR_IVYBRIDGE:
	case PROCESSOR_IVYBRIDGE_EP:
		*config=0x1cd;
		*config1=0x3;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	case PROCESSOR_HASWELL:
	case PROCESSOR_HASWELL_EP:
		*config=0x1cd;
		*config1=0x3;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	case PROCESSOR_BROADWELL:
		*config=0x1cd;
		*config1=0x3;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	default:
		*config=0x0;
		*config1=0x0;
		*precise_ip=0;
		strcpy(name,"UNKNOWN");
		processor_notfound=-1;
	}

	return processor_notfound;

}

int get_latency_store_event(unsigned long long *config,
			unsigned long long *config1,
			int *precise_ip,
			char *name) {

	int processor,processor_notfound=0;

	processor=detect_processor();

	switch(processor) {

	case PROCESSOR_SANDYBRIDGE:
	case PROCESSOR_SANDYBRIDGE_EP:
		*config=0x2cd;
		*config1=0x0;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	case PROCESSOR_IVYBRIDGE:
	case PROCESSOR_IVYBRIDGE_EP:
		*config=0x2cd;
		*config1=0x0;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	case PROCESSOR_HASWELL:
	case PROCESSOR_HASWELL_EP:
		*config=0x2cd;
		*config1=0x0;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	case PROCESSOR_BROADWELL:
		*config=0x2cd;
		*config1=0x0;
		*precise_ip=2;
		strcpy(name,"MEM_TRANS_RETIRED:LATENCY_ABOVE_THRESHOLD");
		break;
	default:
		*config=0x0;
		*config1=0x0;
		*precise_ip=0;
		strcpy(name,"UNKNOWN");
		processor_notfound=-1;
	}

	return processor_notfound;

}

int detect_architecture(void) {
#if defined(__i386__)
	return ARCH_X86;
#elif defined(__x86_64__)
	return ARCH_X86_64;
#elif defined __powerpc__
	return ARCH_POWER;
#elif defined __arm__
	return ARCH_ARM;
#elif defined __aarch64__
	return ARCH_ARM64;
#else
	return ARCH_UNKNOWN;
#endif
}
