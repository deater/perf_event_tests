#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <asm/unistd.h>

#include "papi.h"
#include "papi_helpers.h"

int detect_processor(void) {

   const PAPI_hw_info_t *hwinfo;

   hwinfo = PAPI_get_hardware_info(  );
   if (hwinfo==NULL) return PROCESSOR_UNKNOWN;

   /* Power6 */
   /* PAPI_VENDOR_IBM */
   if (strstr(hwinfo->model_string,"POWER6")) {
      return PROCESSOR_POWER6;
   }

   /* ARM Cortex A9 */
   /* PAPI_VENDOR_ARM */
   if (strstr(hwinfo->model_string,"CPU part")) {
      if (strstr(hwinfo->model_string,"0xc09")) {
	 return PROCESSOR_CORTEX_A9;
      }
   }

   if (hwinfo->vendor==PAPI_VENDOR_AMD) {
      if (hwinfo->cpuid_family==0x6) {
	 return PROCESSOR_K7;
      }
      if (hwinfo->cpuid_family==0xf) {
	 return PROCESSOR_K8;
      }
      if (hwinfo->cpuid_family==0x10) {
	 return PROCESSOR_AMD_FAM10H;
      }
      if (hwinfo->cpuid_family==0x11) {
	 return PROCESSOR_AMD_FAM11H;
      }
      if (hwinfo->cpuid_family==0x14) {
	 return PROCESSOR_AMD_FAM14H;
      }
      if (hwinfo->cpuid_family==0x15) {
	 return PROCESSOR_AMD_FAM15H;
      }
   }
   
   if (hwinfo->vendor==PAPI_VENDOR_INTEL) {
    
      if (hwinfo->cpuid_family==6) {
	 switch(hwinfo->cpuid_model) {
	  case 1:
	  case 3:
	    return PROCESSOR_PENTIUM_PRO;
	  case 5:
	  case 6:
	    return PROCESSOR_PENTIUM_II;
	  case 7:
	  case 8:
	  case 10:
	  case 11:
	    return PROCESSOR_PENTIUM_III;
	  case 9:
	  case 13:
	    return PROCESSOR_PENTIUM_M;
	  case 14:
	    return PROCESSOR_COREDUO;
	  case 15:
	  case 22:
	  case 23:
	  case 29:
	    return PROCESSOR_CORE2;	    
	  case 26:
	  case 30:
	  case 31:
	    return PROCESSOR_NEHALEM;
	  case 28:
	  case 38:
	    return PROCESSOR_ATOM;
	  case 37:
	  case 44:
	    return PROCESSOR_WESTMERE;
	  case 42:
	    return PROCESSOR_SANDYBRIDGE;
	  case 45:
	    return PROCESSOR_SANDYBRIDGE_EP;
	  case 46:
	    return PROCESSOR_NEHALEM_EX;
	  case 58:
            return PROCESSOR_IVYBRIDGE;
	 }
      }
      if (hwinfo->cpuid_family==11) {
	 return PROCESSOR_KNIGHTSCORNER;
      }
      if (hwinfo->cpuid_family==15) {
	 return PROCESSOR_PENTIUM_4;
      }
   }

   return PROCESSOR_UNKNOWN;
}

static char core2_events[TEST_MAX_EVENTS][TEST_MAX_STRLEN]={
  "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  "ITLB:MISSES",                  /* PAPI_TLB_IM  */
  "DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  "L1I_READS",                    /* PAPI_L1_ICA  */
  "L1I_MISSES",                   /* PAPI_L1_ICM  */
  "L1D_ALL_REF",                  /* PAPI_L1_DCA  */
  "L1D_REPL",                     /* PAPI_L1_DCM  */
  "HW_INT_RCV",                   /* PAPI_HW_INT  */
  "FP_COMP_OPS_EXE",              /* PAPI_FP_OPS  */
  "INST_RETIRED:LOADS",           /* PAPI_LD_INS  */
  "INST_RETIRED:STORES",          /* PAPI_SR_INS  */
  "L2_RQSTS:SELF:ANY:MESI",       /* PAPI_L2_TCA  */
  "L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

static char atom_events[TEST_MAX_EVENTS][TEST_MAX_STRLEN]={
  "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  "BRANCH_INSTRUCTIONS_RETIRED",  /* PAPI_BR_INS  */
  "MISPREDICTED_BRANCH_RETIRED",  /* PAPI_BR_MSP  */
  "ITLB:MISSES",                  /* PAPI_TLB_IM  */
  "DATA_TLB_MISSES:DTLB_MISS",    /* PAPI_TLB_DM  */
  "ICACHE:ACCESSES",              /* PAPI_L1_ICA  */
  "ICACHE:MISSES",                /* PAPI_L1_ICM  */
  "L1D_CACHE:LD",                 /* PAPI_L1_DCA  */
  "L2_RQSTS:SELF",                /* PAPI_L1_DCM  */
  "HW_INT_RCV",                   /* PAPI_HW_INT  */
  "SIMD_INST_RETIRED:ANY",        /* PAPI_FP_OPS  */
  "L2_LD:SELF:ANY:MESI",          /* PAPI_LD_INS  */
  "L2_ST:SELF:MESI",              /* PAPI_SR_INS  */
  "L2_LD:SELF:ANY:MESI",          /* PAPI_L2_TCA  */
  "L2_LINES_IN:SELF:ANY",         /* PAPI_L2_TCM  */
};

static char amd10h_events[TEST_MAX_EVENTS][TEST_MAX_STRLEN]={
  "CPU_CLK_UNHALTED",                         /* PAPI_TOT_CYC */
  "RETIRED_INSTRUCTIONS",                     /* PAPI_TOT_INS */
  "RETIRED_BRANCH_INSTRUCTIONS",              /* PAPI_BR_INS  */
  "RETIRED_MISPREDICTED_BRANCH_INSTRUCTIONS", /* PAPI_BR_MSP  */
  "L1_ITLB_MISS_AND_L2_ITLB_MISS:ALL",        /* PAPI_TLB_IM  */
  "L1_DTLB_AND_L2_DTLB_MISS",                 /* PAPI_TLB_DM  */
  "INSTRUCTION_CACHE_FETCHES",                /* PAPI_L1_ICA  */
  "INSTRUCTION_CACHE_MISSES",                 /* PAPI_L1_ICM  */
  "DATA_CACHE_ACCESSES",                      /* PAPI_L1_DCA  */
  "DATA_CACHE_MISSES",                        /* PAPI_L1_DCM  */
  "INTERRUPTS_TAKEN",                         /* PAPI_HW_INT  */
  "DISPATCHED_FPU:OPS_MULTIPLY:OPS_ADD",      /* PAPI_FP_OPS  */
  "DECODER_EMPTY",                            /* PAPI_LD_INS  */ /* nope */
  "DISPATCH_STALLS",                          /* PAPI_SR_INS  */ /* nope */
  "REQUESTS_TO_L2:ALL",                       /* PAPI_L2_TCA  */
  "L2_CACHE_MISS:INSTRUCTIONS:DATA",          /* PAPI_L2_TCM  */
};

static char nehalem_events[TEST_MAX_EVENTS][TEST_MAX_STRLEN]={
  "UNHALTED_CORE_CYCLES",         /* PAPI_TOT_CYC */
  "INSTRUCTIONS_RETIRED",         /* PAPI_TOT_INS */
  "BR_INST_EXEC:ANY",             /* PAPI_BR_INS  */
  "BR_MISP_EXEC:ANY",             /* PAPI_BR_MSP  */
  "ITLB_MISSES:ANY",              /* PAPI_TLB_IM  */
  "DTLB_MISSES:ANY",              /* PAPI_TLB_DM  */
  "L1I:READS",                    /* PAPI_L1_ICA  */
  "L1I:MISSES",                   /* PAPI_L1_ICM  */
  "L1D_ALL_REF:ANY",              /* PAPI_L1_DCA  */
  "L1D:REPL",                     /* PAPI_L1_DCM  */
  "HW_INT:RCV",                   /* PAPI_HW_INT  */
  "FP_COMP_OPS_EXE:SSE_FP",       /* PAPI_FP_OPS  */
  "MEM_INST_RETIRED:LOADS",       /* PAPI_LD_INS  */
  "MEM_INST_RETIRED:STORES",      /* PAPI_SR_INS  */
  "L2_RQSTS:REFERENCES",          /* PAPI_L2_TCA  */
  "L2_RQSTS:PREFETCHES",          /* PAPI_L2_TCM  */
};

static char power6_events[TEST_MAX_EVENTS][TEST_MAX_STRLEN]={
  "PM_RUN_CYC",         	/* PAPI_TOT_CYC */
  "PM_INST_CMPL",         	/* PAPI_TOT_INS */
  "PM_BRU_FIN",             	/* PAPI_BR_INS  */
  "PM_BR_MPRED",             	/* PAPI_BR_MSP  */
  "PM_DATA_FROM_L2MISS",        /* PAPI_TLB_IM  */  /* nope */
  "PM_DATA_FROM_L2MISS",        /* PAPI_TLB_DM  */  /* nope */
  "PM_ST_REF_L1",               /* PAPI_L1_ICA  */ /* nope */
  "PM_L1_ICACHE_MISS",          /* PAPI_L1_ICM  */
  "PM_LD_REF_L1",               /* PAPI_L1_DCA  */  /* nope */
  "PM_LD_MISS_L1",              /* PAPI_L1_DCM  */  /* nope */
  "PM_EXT_INT",                 /* PAPI_HW_INT  */
  "PM_FPU_FLOP",       		/* PAPI_FP_OPS  */
  "PM_LD_REF_L1",       	/* PAPI_LD_INS  */
  "PM_ST_REF_L1",      		/* PAPI_SR_INS  */
  "PM_LD_REF_L1",          	/* PAPI_L2_TCA  */ /* nope */
  "PM_DATA_FROM_L2MISS",        /* PAPI_L2_TCM  */ /* nope */
};

static char cortexA9_events[TEST_MAX_EVENTS][TEST_MAX_STRLEN]={
  "CPU_CYCLES",                    /* PAPI_TOT_CYC */
  "INST_OUT_OF_RENAME_STAGE",      /* PAPI_TOT_INS */
  "PC_WRITE",                      /* PAPI_BR_INS  */
  "PC_BRANCH_MIS_PRED",            /* PAPI_BR_MSP  */
  "ITLB_MISS",                     /* PAPI_TLB_IM  */
  "DTLB_REFILL",                   /* PAPI_TLB_DM  */
  "MAIN_UNIT_EXECUTED_INST",       /* PAPI_L1_ICA  */ /* nope */
  "IFETCH_MISS",                   /* PAPI_L1_ICM  */
  "DCACHE_ACCESS",                  /* PAPI_L1_DCA  */
  "DCACHE_REFILL",                 /* PAPI_L1_DCM  */
  "EXT_INTERRUPTS",                /* PAPI_HW_INT  */
  "FP_EXECUTED_INST",              /* PAPI_FP_OPS  */
  "DREAD",                         /* PAPI_LD_INS  */
  "DWRITE",                        /* PAPI_SR_INS  */
  "DCACHE_ACCESS",                 /* PAPI_L2_TCA  */  /* nope */
  "DCACHE_REFILL",                 /* PAPI_L2_TCM  */  /* nope */
};


int copy_events(char *eventset) {

  int processor,processor_notfound=0;

  processor=detect_processor();

  switch(processor) {
  case PROCESSOR_CORE2:
      memcpy(eventset,core2_events,TEST_MAX_EVENTS*TEST_MAX_STRLEN*sizeof(char));
      break;
  case PROCESSOR_NEHALEM:
     memcpy(eventset,nehalem_events,TEST_MAX_EVENTS*TEST_MAX_STRLEN*sizeof(char));
     break;
  case PROCESSOR_ATOM:
     memcpy(eventset,atom_events,TEST_MAX_EVENTS*TEST_MAX_STRLEN*sizeof(char));
     break;
  case PROCESSOR_K7:
  case PROCESSOR_K8:
  case PROCESSOR_AMD_FAM10H:
  case PROCESSOR_AMD_FAM11H:
  case PROCESSOR_AMD_FAM14H:
  case PROCESSOR_AMD_FAM15H:
     memcpy(eventset,amd10h_events,TEST_MAX_EVENTS*TEST_MAX_STRLEN*sizeof(char));
     break;
  case PROCESSOR_POWER6:
     memcpy(eventset,power6_events,TEST_MAX_EVENTS*TEST_MAX_STRLEN*sizeof(char));
     break;
  case PROCESSOR_CORTEX_A8:
     memcpy(eventset,cortexA9_events,TEST_MAX_EVENTS*TEST_MAX_STRLEN*sizeof(char));
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
