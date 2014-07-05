/* Intel Volume 3-B

	17.4.9 -- Debug store area.
		On overflow, the PEBS event *after* the overflow
		records to the DS area the PEBS information
		The Instruction pointer points to one after
		the instruction causing the issue.

	Linux handles this in arch/x86/kernel/cpu/perf_event_intel_ds.c

	PEBS is configured in the DS area setup.  Sets min and max
	values of the memory reason, and what value to reset the
	counter to on interrupt.

	PEBS field contains

	RFLAGS/RIP/RAX/RBX/../R15

	BTS, regular overflow, and PEBS overflow share the same
	interrupt handler.

	Each processor has its own save area.

	18.4.2 repeats above info

	18.4.4 on core2 only 9 events support PEBS, and only
		available on one counter, PMC0
		ENABLE_PEBS bit in IA32_PEBS_ENABLE MSR

	THe PEBStrap bit allows trap-like (record state after instruction)
		of fault-like (record state before instruction)

	18.6.2 Atom Silvermont also supports PEBS

	18.7.1 Nehalem -- PEBS supported in all 4 general purpose counters
		Load latency support added.

		AnyThread, Edge, Invert, CMask must be zero in PEBS event

		Priority:  if simultaneous, event in PMC0 has precedence
		over PMC1, etc.

	18.7.1.2 Latench measure
		MEM_INST_RETIRED event and LATENCY_ABOVE_THRESHOLD umask

		SR_PEBS_LD_LAT_THRESHOLD MSR programmed with the
		latency of interest, only above is counted.
		Minimum value for this is 3.

		Loads are randomly chosen to be tagged to measure
		latency info.  When PEBS is triggered, the most
		recent randomly tagged value is reported.

		Linear Address, Latency, and Source reported

	18.9.4 Sandybridge Support

		Like above but also stores.
		Stores in PMC3 only

		table 18-18

	18.9.4.1 number of supported events increased

	With precise store, once triggered, full latency info
	captured on the next store that completes.

	MEM_TRANS_RETIRED.PRECISE_STORE

	address and status recorded

	18.9.4.4	PDIR
		Precise  Distribution of Instructions Retired

		Can avoid skid.  CPU notices when about to overflow
		and drops to more precise mode.

		Should quiet all other counters when using this(?)

		PDIR applies only to the INST_RETIRED.ALL precise event,
		and must use IA32_PMC1 with PerfEvtSel1 property
		configured and bit 1 in the IA32_PEBS_ENABLE set to 1.
		INST_RETIRED.ALL is a non-architectural performance
		event, it is not supported in prior generations.

	18.11	Haswell

		No precise store.

		Has transaction abort info

		Adds new "Eventing" field that has IP of instruction
		causing the event (as opposed to RIP which has the
		next instruction)

		Precise store replaced by Data Address Profiling.

		MEM_UOPS_RETIRED.STLB_MISS_LOADS
		MEM_UOPS_RETIRED.STLB_MISS_STORES
		MEM_UOPS_RETIRED.LOCK_LOADS
		MEM_UOPS_RETIRED.SPLIT_STORES
		MEM_UOPS_RETIRED.SPLIT_LOADS
		MEM_UOPS_RETIRED.ALL_STORES
		MEM_UOPS_RETIRED.ALL_LOADS
		MEM_LOAD_UOPS_LLC_MISS_RETIRED.LOCAL_DRAM
		MEM_LOAD_UOPS_RETIRED.L1_HIT
		MEM_LOAD_UOPS_RETIRED.L2_HIT
		MEM_LOAD_UOPS_RETIRED.L3_HIT
		MEM_LOAD_UOPS_RETIRED.L1_MISS
		MEM_LOAD_UOPS_RETIRED.L2_MISS
		MEM_LOAD_UOPS_RETIRED.L3_MISS
		MEM_LOAD_UOPS_RETIRED.HIT_LFB
		MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_MISS
		MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_HIT
		MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_HITM
		UOPS_RETIRED.ALL (if load or store is tagged)
		MEM_LOAD_UOPS_LLC_HIT_RETIRED.XSNP_NONE

		When enabled, the latency etc info is stored in the PEBS record

	18.13 = Pentium 4 PEBS

	18.13.7
		only one counter, only three events

***************
Linux interface
***************

	Does not support old 32-bit p4/core PEBS record format
	
	Load latency, precise store, precise store haswell

	pebs_fixup_ip() ?
		makes fake Eventing info?
		pebs.trap?
	


weight field holds pebs->lat

*/

int main(int argc, char **argv) {

	return 0;
}
