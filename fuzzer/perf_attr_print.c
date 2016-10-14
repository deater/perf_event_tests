#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>

#include "hw_breakpoint.h"
#include "perf_event.h"
#include "perf_helpers.h"

#include "perf_attr_print.h"

/* forward declarations */
void perf_pretty_print_tracepoint(FILE *fff, int id);

void perf_pretty_print_mmap_prot(int prot) {

	if (prot==PROT_NONE) {
		printf("PROT_NONE");
		return;
	}
	if (prot&PROT_READ) printf("PROT_READ ");
	if (prot&PROT_WRITE) printf("PROT_WRITE ");
	if (prot&PROT_EXEC) printf("PROT_EXEC ");
}

void perf_pretty_print_mmap_flags(int flags) {

	if (flags&MAP_SHARED) printf("MAP_SHARED ");
	if (flags&MAP_PRIVATE) printf("MAP_PRIVATE ");
#ifdef MAP_32BIT
	if (flags&MAP_32BIT) printf("MAP_32BIT ");
#endif
	if (flags&MAP_ANONYMOUS) printf("MAP_ANONYMOUS ");
	if (flags&MAP_DENYWRITE) printf("MAP_DENYWRITE ");
	if (flags&MAP_EXECUTABLE) printf("MAP_EXECUTABLE ");
	if (flags&MAP_FILE) printf("MAP_FILE ");
	if (flags&MAP_FIXED) printf("MAP_FIXED ");
	if (flags&MAP_GROWSDOWN) printf("MAP_GROWSDOWN ");
#ifdef MAP_HUGETLB
	if (flags&MAP_HUGETLB) printf("MAP_HUGETLB ");
#endif
	if (flags&MAP_LOCKED) printf("MAP_LOCKED ");
	if (flags&MAP_NONBLOCK) printf("MAP_NONBLOCK ");
	if (flags&MAP_NORESERVE) printf("MAP_NORESERVE ");
	if (flags&MAP_POPULATE) printf("MAP_POPULATE ");
#ifdef MAP_STACK
	if (flags&MAP_STACK) printf("MAP_STACK ");
#endif
}




void perf_pretty_print_flags(FILE *fff,long int flags) {

	long int our_flags=flags;

	if (our_flags==0) {
		fprintf(fff,"0");
		return;
	}

	if (our_flags&PERF_FLAG_FD_NO_GROUP) {
		fprintf(fff,"PERF_FLAG_FD_NO_GROUP");
		our_flags&=~PERF_FLAG_FD_NO_GROUP;
		if (our_flags==0) return;
		fprintf(fff,"|");
	}

	if (our_flags&PERF_FLAG_FD_OUTPUT) {
		fprintf(fff,"PERF_FLAG_FD_OUTPUT");
		our_flags&=~PERF_FLAG_FD_OUTPUT;
		if (our_flags==0) return;
		fprintf(fff,"|");
	}

	if (flags&PERF_FLAG_PID_CGROUP) {
		fprintf(fff,"PERF_FLAG_PID_CGROUP");
		our_flags&=~PERF_FLAG_PID_CGROUP;
		if (our_flags==0) return;
		fprintf(fff,"|");
	}

	if (flags&PERF_FLAG_FD_CLOEXEC) {
		fprintf(fff,"PERF_FLAG_FD_CLOEXEC");
		our_flags&=~PERF_FLAG_FD_CLOEXEC;
		if (our_flags==0) return;
		fprintf(fff,"|");
	}

	fprintf(fff,"0x%lxULL",our_flags);
}

static void perf_pretty_print_type(FILE *fff,int type) {

	switch(type) {
		case PERF_TYPE_HARDWARE:
			fprintf(fff,"PERF_TYPE_HARDWARE");
			break;
		case PERF_TYPE_SOFTWARE:
			fprintf(fff,"PERF_TYPE_SOFTWARE");
			break;
		case PERF_TYPE_TRACEPOINT:
			fprintf(fff,"PERF_TYPE_TRACEPOINT");
			break;
		case PERF_TYPE_HW_CACHE:
			fprintf(fff,"PERF_TYPE_HW_CACHE");
			break;
		case PERF_TYPE_RAW:
			fprintf(fff,"PERF_TYPE_RAW");
			break;
		case PERF_TYPE_BREAKPOINT:
			fprintf(fff,"PERF_TYPE_BREAKPOINT");
			break;
		default:
			fprintf(fff,"%d",type);
			break;
	}
}

static void perf_pretty_print_cache(FILE *fff, long long config) {


	long long cache_id,hw_cache_op_id,hw_cache_op_result_id;

	cache_id=config&0xff;

	switch(cache_id) {

		case PERF_COUNT_HW_CACHE_L1D:
			fprintf(fff,"PERF_COUNT_HW_CACHE_L1D");
			break;
		case PERF_COUNT_HW_CACHE_L1I:
			fprintf(fff,"PERF_COUNT_HW_CACHE_L1I");
			break;
		case PERF_COUNT_HW_CACHE_LL:
			fprintf(fff,"PERF_COUNT_HW_CACHE_LL");
			break;
		case PERF_COUNT_HW_CACHE_DTLB:
			fprintf(fff,"PERF_COUNT_HW_CACHE_DTLB");
			break;
		case PERF_COUNT_HW_CACHE_ITLB:
			fprintf(fff,"PERF_COUNT_HW_CACHE_ITLB");
			break;
		case PERF_COUNT_HW_CACHE_BPU:
			fprintf(fff,"PERF_COUNT_HW_CACHE_BPU");
			break;
		case PERF_COUNT_HW_CACHE_NODE:
			fprintf(fff,"PERF_COUNT_HW_CACHE_NODE");
			break;
		default:
			fprintf(fff,"0x%llxULL",cache_id);
			break;
	}

	fprintf(fff," | ( ");

	hw_cache_op_id=(config>>8)&0xff;

	switch(hw_cache_op_id) {
		case PERF_COUNT_HW_CACHE_OP_READ:
			fprintf(fff,"PERF_COUNT_HW_CACHE_OP_READ");
			break;
		case PERF_COUNT_HW_CACHE_OP_WRITE:
			fprintf(fff,"PERF_COUNT_HW_CACHE_OP_WRITE");
			break;
		case PERF_COUNT_HW_CACHE_OP_PREFETCH:
			fprintf(fff,"PERF_COUNT_HW_CACHE_OP_PREFETCH");
			break;
		default:
			fprintf(fff,"0x%llxULL",hw_cache_op_id);
			break;
	}
	fprintf(fff," << 8) | ( ");

	hw_cache_op_result_id=(config>>16);
	switch(hw_cache_op_result_id) {
		case PERF_COUNT_HW_CACHE_RESULT_ACCESS:
			fprintf(fff,"PERF_COUNT_HW_CACHE_RESULT_ACCESS");
			break;
		case PERF_COUNT_HW_CACHE_RESULT_MISS:
			fprintf(fff,"PERF_COUNT_HW_CACHE_RESULT_MISS");
			break;
		default:
			fprintf(fff,"0x%llxULL",hw_cache_op_result_id);
			break;
	}

	fprintf(fff," << 16 )");


}

static void perf_pretty_print_config(FILE *fff, long long type, long long config) {

	switch (type) {
		case PERF_TYPE_HARDWARE:
                        switch(config) {
				case PERF_COUNT_HW_CPU_CYCLES:
					fprintf(fff,"PERF_COUNT_HW_CPU_CYCLES");
					break;
				case PERF_COUNT_HW_INSTRUCTIONS:
					fprintf(fff,"PERF_COUNT_HW_INSTRUCTIONS");
					break;
				case PERF_COUNT_HW_CACHE_REFERENCES:
					fprintf(fff,"PERF_COUNT_HW_CACHE_REFERENCES");
					break;
				case PERF_COUNT_HW_CACHE_MISSES:
					fprintf(fff,"PERF_COUNT_HW_CACHE_MISSES");
					break;
				case PERF_COUNT_HW_BRANCH_INSTRUCTIONS:
					fprintf(fff,"PERF_COUNT_HW_BRANCH_INSTRUCTIONS");
					break;
				case PERF_COUNT_HW_BRANCH_MISSES:
					fprintf(fff,"PERF_COUNT_HW_BRANCH_MISSES");
					break;
				case PERF_COUNT_HW_BUS_CYCLES:
					fprintf(fff,"PERF_COUNT_HW_BUS_CYCLES");
					break;
                                case PERF_COUNT_HW_STALLED_CYCLES_FRONTEND:
					fprintf(fff,"PERF_COUNT_HW_STALLED_CYCLES_FRONTEND");
					break;
				case PERF_COUNT_HW_STALLED_CYCLES_BACKEND:
					fprintf(fff,"PERF_COUNT_HW_STALLED_CYCLES_BACKEND");
					break;
				case PERF_COUNT_HW_REF_CPU_CYCLES:
					fprintf(fff,"PERF_COUNT_HW_REF_CPU_CYCLES");
					break;
				default:
					fprintf(fff,"0x%llxULL",config);
					break;
			}
			printf(";");
			break;
		case PERF_TYPE_SOFTWARE:
			switch(config) {
				case PERF_COUNT_SW_CPU_CLOCK:
					fprintf(fff,"PERF_COUNT_SW_CPU_CLOCK");
					break;
				case PERF_COUNT_SW_TASK_CLOCK:
					fprintf(fff,"PERF_COUNT_SW_TASK_CLOCK");
					break;
				case PERF_COUNT_SW_PAGE_FAULTS:
					fprintf(fff,"PERF_COUNT_SW_PAGE_FAULTS");
					break;
				case PERF_COUNT_SW_CONTEXT_SWITCHES:
					fprintf(fff,"PERF_COUNT_SW_CONTEXT_SWITCHES");
					break;
				case PERF_COUNT_SW_CPU_MIGRATIONS:
					fprintf(fff,"PERF_COUNT_SW_CPU_MIGRATIONS");
					break;
				case PERF_COUNT_SW_PAGE_FAULTS_MIN:
					fprintf(fff,"PERF_COUNT_SW_PAGE_FAULTS_MIN");
					break;
				case PERF_COUNT_SW_PAGE_FAULTS_MAJ:
					fprintf(fff,"PERF_COUNT_SW_PAGE_FAULTS_MAJ");
					break;
				case PERF_COUNT_SW_ALIGNMENT_FAULTS:
					fprintf(fff,"PERF_COUNT_SW_ALIGNMENT_FAULTS");
					break;
				case PERF_COUNT_SW_EMULATION_FAULTS:
					fprintf(fff,"PERF_COUNT_SW_EMULATION_FAULTS");
					break;
				case PERF_COUNT_SW_DUMMY:
					fprintf(fff,"PERF_COUNT_SW_DUMMY");
					break;
				default:
					fprintf(fff,"0x%llxULL",config);
			}
			printf(";");
			break;

                case PERF_TYPE_HW_CACHE:
			perf_pretty_print_cache(fff,config);
			printf(";");
			break;

		case PERF_TYPE_TRACEPOINT:
			fprintf(fff,"0x%llxULL; /* %lld ",config,config&0xffffffff);
			perf_pretty_print_tracepoint(fff,config);
			fprintf(fff," */");
			break;

                case PERF_TYPE_RAW:
                case PERF_TYPE_BREAKPOINT:
		default:
			fprintf(fff,"0x%llxULL;",config);
			break;
	}


}

static void perf_pretty_print_sample_type(FILE *fff, unsigned long long sample_type) {

	unsigned long long our_type=sample_type;

	if (!our_type) {
		fprintf(fff,"0");
		return;
	}

	if (our_type&PERF_SAMPLE_IP) {
		fprintf(fff,"PERF_SAMPLE_IP");
		our_type&=~PERF_SAMPLE_IP;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_TID) {
		fprintf(fff,"PERF_SAMPLE_TID");
		our_type&=~PERF_SAMPLE_TID;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_TIME) {
		fprintf(fff,"PERF_SAMPLE_TIME");
		our_type&=~PERF_SAMPLE_TIME;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_ADDR) {
		fprintf(fff,"PERF_SAMPLE_ADDR");
		our_type&=~PERF_SAMPLE_ADDR;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_READ) {
		fprintf(fff,"PERF_SAMPLE_READ");
		our_type&=~PERF_SAMPLE_READ;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_CALLCHAIN) {
		fprintf(fff,"PERF_SAMPLE_CALLCHAIN");
		our_type&=~PERF_SAMPLE_CALLCHAIN;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_ID) {
		fprintf(fff,"PERF_SAMPLE_ID");
		our_type&=~PERF_SAMPLE_ID;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_CPU) {
		fprintf(fff,"PERF_SAMPLE_CPU");
		our_type&=~PERF_SAMPLE_CPU;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_PERIOD) {
		fprintf(fff,"PERF_SAMPLE_PERIOD");
		our_type&=~PERF_SAMPLE_PERIOD;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_STREAM_ID) {
		fprintf(fff,"PERF_SAMPLE_STREAM_ID");
		our_type&=~PERF_SAMPLE_STREAM_ID;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_RAW) {
		fprintf(fff,"PERF_SAMPLE_RAW");
		our_type&=~PERF_SAMPLE_RAW;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_STACK) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_STACK");
		our_type&=~PERF_SAMPLE_BRANCH_STACK;
		if (!our_type) return;
		fprintf(fff,"|");
	}
	if (our_type&PERF_SAMPLE_REGS_USER) {
		fprintf(fff,"PERF_SAMPLE_REGS_USER");
		our_type&=~PERF_SAMPLE_REGS_USER;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_STACK_USER) {
		fprintf(fff,"PERF_SAMPLE_STACK_USER");
		our_type&=~PERF_SAMPLE_STACK_USER;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_WEIGHT) {
		fprintf(fff,"PERF_SAMPLE_WEIGHT");
		our_type&=~PERF_SAMPLE_WEIGHT;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_DATA_SRC) {
		fprintf(fff,"PERF_SAMPLE_DATA_SRC");
		our_type&=~PERF_SAMPLE_DATA_SRC;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_IDENTIFIER) {
		fprintf(fff,"PERF_SAMPLE_IDENTIFIER");
		our_type&=~PERF_SAMPLE_IDENTIFIER;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_TRANSACTION) {
		fprintf(fff,"PERF_SAMPLE_TRANSACTION");
		our_type&=~PERF_SAMPLE_TRANSACTION;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	fprintf(fff,"0x%llxULL",our_type);


}


static void perf_pretty_print_branch_sample_type(FILE *fff, unsigned long long branch_sample_type) {

	unsigned long long our_type=branch_sample_type;

	if (!our_type) {
		fprintf(fff,"0");
		return;
	}

	if (our_type&PERF_SAMPLE_BRANCH_USER) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_USER");
		our_type&=~PERF_SAMPLE_BRANCH_USER;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_KERNEL) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_KERNEL");
		our_type&=~PERF_SAMPLE_BRANCH_KERNEL;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_HV) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_HV");
		our_type&=~PERF_SAMPLE_BRANCH_HV;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_ANY) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_ANY");
		our_type&=~PERF_SAMPLE_BRANCH_ANY;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_ANY_CALL) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_ANY_CALL");
		our_type&=~PERF_SAMPLE_BRANCH_ANY_CALL;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_ANY_RETURN) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_ANY_RETURN");
		our_type&=~PERF_SAMPLE_BRANCH_ANY_RETURN;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_IND_CALL) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_IND_CALL");
		our_type&=~PERF_SAMPLE_BRANCH_IND_CALL;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_IND_JUMP) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_IND_JUMP");
		our_type&=~PERF_SAMPLE_BRANCH_IND_JUMP;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_ABORT_TX) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_ABORT_TX");
		our_type&=~PERF_SAMPLE_BRANCH_ABORT_TX;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_IN_TX) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_IN_TX");
		our_type&=~PERF_SAMPLE_BRANCH_IN_TX;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_NO_TX) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_NO_TX");
		our_type&=~PERF_SAMPLE_BRANCH_NO_TX;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_COND) {
		fprintf(fff,"PERF_SAMPLE_BRANCH_COND");
		our_type&=~PERF_SAMPLE_BRANCH_COND;
		if (!our_type) return;
		fprintf(fff,"|");
	}

	fprintf(fff,"0x%llxULL",our_type);


}


static void perf_pretty_print_read_format(FILE *fff, unsigned long long read_format) {

	unsigned long long our_format=read_format;

	if (our_format&PERF_FORMAT_TOTAL_TIME_ENABLED) {
		fprintf(fff,"PERF_FORMAT_TOTAL_TIME_ENABLED");
		our_format&=~PERF_FORMAT_TOTAL_TIME_ENABLED;
		if (!our_format) return;
		fprintf(fff,"|");
	}

	if (our_format&PERF_FORMAT_TOTAL_TIME_RUNNING) {
		fprintf(fff,"PERF_FORMAT_TOTAL_TIME_RUNNING");
		our_format&=~PERF_FORMAT_TOTAL_TIME_RUNNING;
		if (!our_format) return;
		fprintf(fff,"|");
	}

	if (our_format&PERF_FORMAT_ID) {
		fprintf(fff,"PERF_FORMAT_ID");
		our_format&=~PERF_FORMAT_ID;
		if (!our_format) return;
		fprintf(fff,"|");
	}

	if (our_format&PERF_FORMAT_GROUP) {
		fprintf(fff,"PERF_FORMAT_GROUP");
		our_format&=~PERF_FORMAT_GROUP;
		if (!our_format) return;
		fprintf(fff,"|");
	}
	fprintf(fff,"0x%llxULL",our_format);

}

static void perf_pretty_print_breakpoint_type(FILE *fff,int bp_type) {

	int our_bp=bp_type;

	if (our_bp==HW_BREAKPOINT_EMPTY) {
		fprintf(fff,"HW_BREAKPOINT_EMPTY");
		return;
	}

	if (our_bp&HW_BREAKPOINT_R) {
		fprintf(fff,"HW_BREAKPOINT_R");
		our_bp&=~HW_BREAKPOINT_R;
		if (!our_bp) return;
		fprintf(fff,"|");
	}

	if (our_bp&HW_BREAKPOINT_W) {
		fprintf(fff,"HW_BREAKPOINT_W");
		our_bp&=~HW_BREAKPOINT_W;
		if (!our_bp) return;
		fprintf(fff,"|");
	}

	if (our_bp&HW_BREAKPOINT_X) {
		fprintf(fff,"HW_BREAKPOINT_X");
		our_bp&=~HW_BREAKPOINT_X;
		if (!our_bp) return;
		fprintf(fff,"|");
	}
	fprintf(fff,"0x%x",our_bp);
}

void perf_pretty_print_attr(FILE *fff, struct perf_event_attr *pe, int fd) {

	fprintf(fff,"\n");
	fprintf(fff,"\tmemset(&pe[%d],0,sizeof(struct perf_event_attr));\n",
		fd);
	fprintf(fff,"\tpe[%d].type=",fd);
	perf_pretty_print_type(fff,pe->type);
	fprintf(fff,";\n");

	if (pe->size) fprintf(fff,"\tpe[%d].size=%d;\n",fd,pe->size);

	fprintf(fff,"\tpe[%d].config=",fd);
	perf_pretty_print_config(fff,pe->type,pe->config);
	fprintf(fff,"\n");

	if (pe->freq) {
		if (pe->sample_freq) fprintf(fff,"\tpe[%d].sample_freq=0x%llxULL;\n",fd,pe->sample_freq);
	}
	else {
		if (pe->sample_period) fprintf(fff,"\tpe[%d].sample_period=0x%llxULL;\n",fd,pe->sample_period);
	}

	fprintf(fff,"\tpe[%d].sample_type=",fd);
	perf_pretty_print_sample_type(fff,pe->sample_type);
	fprintf(fff,"; /* %llx */\n",pe->sample_type);

	fprintf(fff,"\tpe[%d].read_format=",fd);
	perf_pretty_print_read_format(fff,pe->read_format);
	fprintf(fff,"; /* %llx */\n",pe->read_format);

	if (pe->disabled) fprintf(fff,"\tpe[%d].disabled=%d;\n",fd,pe->disabled);
	if (pe->inherit) fprintf(fff,"\tpe[%d].inherit=%d;\n",fd,pe->inherit);
	if (pe->pinned) fprintf(fff,"\tpe[%d].pinned=%d;\n",fd,pe->pinned);
	if (pe->exclusive) fprintf(fff,"\tpe[%d].exclusive=%d;\n",fd,pe->exclusive);
	if (pe->exclude_user) fprintf(fff,"\tpe[%d].exclude_user=%d;\n",fd,pe->exclude_user);
	if (pe->exclude_kernel) fprintf(fff,"\tpe[%d].exclude_kernel=%d;\n",fd,pe->exclude_kernel);
	if (pe->exclude_hv) fprintf(fff,"\tpe[%d].exclude_hv=%d;\n",fd,pe->exclude_hv);
	if (pe->exclude_idle) fprintf(fff,"\tpe[%d].exclude_idle=%d;\n",fd,pe->exclude_idle);
	if (pe->mmap) fprintf(fff,"\tpe[%d].mmap=%d;\n",fd,pe->mmap);
	if (pe->comm) fprintf(fff,"\tpe[%d].comm=%d;\n",fd,pe->comm);
	if (pe->freq) fprintf(fff,"\tpe[%d].freq=%d;\n",fd,pe->freq);
	if (pe->inherit_stat) fprintf(fff,"\tpe[%d].inherit_stat=%d;\n",fd,pe->inherit_stat);
	if (pe->enable_on_exec) fprintf(fff,"\tpe[%d].enable_on_exec=%d;\n",fd,pe->enable_on_exec);
	if (pe->task) fprintf(fff,"\tpe[%d].task=%d;\n",fd,pe->task);
	if (pe->watermark) fprintf(fff,"\tpe[%d].watermark=%d;\n",fd,pe->watermark);

	fprintf(fff,"\tpe[%d].precise_ip=",fd);
	switch(pe->precise_ip) {
		case 0:	fprintf(fff,"0; /* arbitrary skid */\n");
			break;
		case 1: fprintf(fff,"1; /* constant skid */\n");
			break;
		case 2: fprintf(fff,"2; /* request zero skid */\n");
			break;
		case 3: fprintf(fff,"3; /* must have zero skid */\n");
			break;
	}

	if (pe->mmap_data) fprintf(fff,"\tpe[%d].mmap_data=%d;\n",fd,pe->mmap_data);
	if (pe->sample_id_all) fprintf(fff,"\tpe[%d].sample_id_all=%d;\n",fd,pe->sample_id_all);
	if (pe->exclude_host) fprintf(fff,"\tpe[%d].exclude_host=%d;\n",fd,pe->exclude_host);
	if (pe->exclude_guest) fprintf(fff,"\tpe[%d].exclude_guest=%d;\n",fd,pe->exclude_guest);
	if (pe->exclude_callchain_kernel) fprintf(fff,"\tpe[%d].exclude_callchain_kernel=%d;\n",fd,pe->exclude_callchain_kernel);
	if (pe->exclude_callchain_user) fprintf(fff,"\tpe[%d].exclude_callchain_user=%d;\n",fd,pe->exclude_callchain_user);
	if (pe->mmap2) fprintf(fff,"\tpe[%d].mmap2=%d;\n",fd,pe->mmap2);
	if (pe->comm_exec) fprintf(fff,"\tpe[%d].comm_exec=%d;\n",fd,pe->comm_exec);

	if (pe->watermark) {
		fprintf(fff,"\tpe[%d].wakeup_watermark=%d;\n",fd,pe->wakeup_watermark);
	}
	else {
		fprintf(fff,"\tpe[%d].wakeup_events=%d;\n",fd,pe->wakeup_events);
	}

	if (pe->bp_type==HW_BREAKPOINT_EMPTY) {
		fprintf(fff,"\tpe[%d].bp_type=HW_BREAKPOINT_EMPTY;\n",fd);
		if (pe->config1) fprintf(fff,"\tpe[%d].config1=0x%llxULL;\n",fd,pe->config1);
		if (pe->config2) fprintf(fff,"\tpe[%d].config2=0x%llxULL;\n",fd,pe->config2);
	}
	else {
		fprintf(fff,"\tpe[%d].bp_type=",fd);
		perf_pretty_print_breakpoint_type(fff,pe->bp_type);
		fprintf(fff,"; /*%d*/\n",pe->bp_type);
		fprintf(fff,"\tpe[%d].bp_addr=0x%llxULL;\n",fd,pe->bp_addr);
		fprintf(fff,"\tpe[%d].bp_len=0x%llxULL;\n",fd,pe->bp_len);
	}
	if (pe->branch_sample_type) {
		fprintf(fff,"\tpe[%d].branch_sample_type=",fd);
		perf_pretty_print_branch_sample_type(fff,pe->branch_sample_type);
		fprintf(fff,";\n");
	}

	if (pe->sample_regs_user) fprintf(fff,"\tpe[%d].sample_regs_user=%lldULL;\n",fd,pe->sample_regs_user);
	if (pe->sample_stack_user) fprintf(fff,"\tpe[%d].sample_stack_user=%d;\n",fd,pe->sample_stack_user);
	fprintf(fff,"\n");
}

void perf_pretty_print_event(FILE *fff, int fd, int original_pid,
				struct perf_event_attr *pe,
				pid_t pid, int cpu,
				int group_fd, unsigned long flags) {

	fprintf(fff,"/* fd = %d */\n",fd);
	perf_pretty_print_attr(fff,pe,fd);

	fprintf(fff,"\tfd[%d]=perf_event_open(&pe[%d],\n",fd,fd);

	if (pid==original_pid) fprintf(fff,"\t\t\t\tgetpid(), /* current thread */\n");
	else {
		fprintf(fff,"\t\t\t\t%d, ",pid);

		if (pid==0) fprintf(fff,"/* current thread */\n");
		else if (pid==-1) fprintf(fff,"/* all processes */\n");
		else fprintf(fff,"/* Only pid %d */\n",pid);
	}

	fprintf(fff,"\t\t\t\t%d, ",cpu);
	if (cpu>=0) fprintf(fff,"/* Only cpu %d */\n",cpu);
	else if (cpu==-1) fprintf(fff,"/* all cpus */\n");
	else fprintf(fff,"/* Unknown setting? */\n");

	if (group_fd==-1) {
		fprintf(fff,"\t\t\t\t-1, /* New Group Leader */\n");
	}
	else {
		fprintf(fff,"\t\t\t\tfd[%d], /* %d is group leader */\n",
			group_fd,group_fd);
	}

	fprintf(fff,"\t\t\t\t");
	perf_pretty_print_flags(fff,flags);
	fprintf(fff," /*%lx*/ ",flags);

	fprintf(fff,");\n");


	fprintf(fff,"\n");

}

void perf_pretty_print_event_short(FILE *fff, int fd, int original_pid,
				struct perf_event_attr *pe,
				pid_t pid, int cpu,
				int group_fd, unsigned long flags) {

	fprintf(fff,"%d:\t",fd);
	perf_pretty_print_type(fff,pe->type);
	fprintf(fff,", ");
	if (pe->type==PERF_TYPE_BREAKPOINT) {
		perf_pretty_print_breakpoint_type(fff,pe->bp_type);
		fprintf(fff,", addr=0x%llx",pe->bp_addr);
		fprintf(fff,", len=0x%llx\n",pe->bp_len);
	}
	else {
		perf_pretty_print_config(fff,pe->type,pe->config);
		fprintf(fff,"\n");

	}
}

void perf_pretty_print_tracepoint(FILE *fff, int id) {

	switch(id&0xffffffff) {
		case 440:	fprintf(fff,"block_bio_backmerge"); break;
		case 442:	fprintf(fff,"block_bio_bounce"); break;
		case 441:	fprintf(fff,"block_bio_complete"); break;
		case 439:	fprintf(fff,"block_bio_frontmerge"); break;
		case 438:	fprintf(fff,"block_bio_queue"); break;
		case 432:	fprintf(fff,"block/block_bio_remap"); break;
		case 448:	fprintf(fff,"block/block_dirty_buffer"); break;
		case 437:	fprintf(fff,"block/block_getrq"); break;
		case 435:	fprintf(fff,"block/block_plug"); break;
		case 447:	fprintf(fff,"block/block_rq_abort"); break;
		case 445:	fprintf(fff,"block/block_rq_complete"); break;
		case 444:	fprintf(fff,"block/block_rq_insert"); break;
		case 443:	fprintf(fff,"block/block_rq_issue"); break;
		case 431:	fprintf(fff,"block/block_rq_remap"); break;
		case 446:	fprintf(fff,"block/block_rq_requeue"); break;
		case 436:	fprintf(fff,"block/block_sleeprq"); break;
		case 433:	fprintf(fff,"block/block_split"); break;
		case 449:	fprintf(fff,"block/block_touch_buffer"); break;
		case 434:	fprintf(fff,"block/block_unplug"); break;
		case 192:	fprintf(fff,"ext3/ext3_allocate_blocks"); break;
		case 208:	fprintf(fff,"ext3/ext3_allocate_inode"); break;
		case 185:	fprintf(fff,"ext3/ext3_alloc_new_reservation"); break;
		case 181:	fprintf(fff,"ext3/ext3_direct_IO_enter"); break;
		case 180:	fprintf(fff,"ext3/ext3_direct_IO_exit"); break;
		case 194:	fprintf(fff,"ext3/ext3_discard_blocks"); break;
		case 186:	fprintf(fff,"ext3/ext3_discard_reservation"); break;
		case 206:	fprintf(fff,"ext3/ext3_drop_inode"); break;
		case 207:	fprintf(fff,"ext3/ext3_evict_inode"); break;
		case 183:	fprintf(fff,"ext3/ext3_forget"); break;
		case 191:	fprintf(fff,"ext3/ext3_free_blocks"); break;
		case 210:	fprintf(fff,"ext3/ext3_free_inode"); break;
		case 175:	fprintf(fff,"ext3/ext3_get_blocks_enter"); break;
		case 174:	fprintf(fff,"ext3/ext3_get_blocks_exit"); break;
		case 195:	fprintf(fff,"ext3/ext3_invalidatepage"); break;
		case 201:	fprintf(fff,"ext3/ext3_journalled_write_end"); break;
		case 198:	fprintf(fff,"ext3/ext3_journalled_writepage"); break;
		case 173:	fprintf(fff,"ext3/ext3_load_inode"); break;
		case 205:	fprintf(fff,"ext3/ext3_mark_inode_dirty"); break;
		case 203:	fprintf(fff,"ext3/ext3_ordered_write_end"); break;
		case 200:	fprintf(fff,"ext3/ext3_ordered_writepage"); break;
		case 182:	fprintf(fff,"ext3/ext3_read_block_bitmap"); break;
		case 197:	fprintf(fff,"ext3/ext3_readpage"); break;
		case 196:	fprintf(fff,"ext3/ext3_releasepage"); break;
		case 193:	fprintf(fff,"ext3/ext3_request_blocks"); break;
		case 209:	fprintf(fff,"ext3/ext3_request_inode"); break;
		case 184:	fprintf(fff,"ext3/ext3_reserved"); break;
		case 187:	fprintf(fff,"ext3/ext3_rsv_window_add"); break;
		case 190:	fprintf(fff,"ext3/ext3_sync_file_enter"); break;
		case 189:	fprintf(fff,"ext3/ext3_sync_file_exit"); break;
		case 188:	fprintf(fff,"ext3/ext3_sync_fs"); break;
		case 177:	fprintf(fff,"ext3/ext3_truncate_enter"); break;
		case 176:	fprintf(fff,"ext3/ext3_truncate_exit"); break;
		case 179:	fprintf(fff,"ext3/ext3_unlink_enter"); break;
		case 178:	fprintf(fff,"ext3/ext3_unlink_exit"); break;
		case 202:	fprintf(fff,"ext3/ext3_writeback_write_end"); break;
		case 199:	fprintf(fff,"ext3/ext3_writeback_writepage"); break;
		case 204:	fprintf(fff,"ext3/ext3_write_begin"); break;
		case 270:	fprintf(fff,"ext4/ext4_allocate_blocks"); break;
		case 297:	fprintf(fff,"ext4/ext4_allocate_inode"); break;
#if 0
case :fprintf(fff,"ext4/ext4_alloc_da_blocks"); break;265
case :fprintf(fff,"ext4/ext4_begin_ordered_truncate"); break;293
case :fprintf(fff,"ext4/ext4_da_release_space"); break;257
case :fprintf(fff,"ext4/ext4_da_reserve_space"); break;258
case :fprintf(fff,"ext4/ext4_da_update_reserve_space"); break;259
case :fprintf(fff,"ext4/ext4_da_write_begin"); break;291
case :fprintf(fff,"ext4/ext4_da_write_end"); break;288
case :fprintf(fff,"ext4/ext4_da_write_pages_extent"); break;285
case :fprintf(fff,"ext4/ext4_da_write_pages"); break;286
case :fprintf(fff,"ext4/ext4_direct_IO_enter"); break;252
case :fprintf(fff,"ext4/ext4_direct_IO_exit"); break;251
case :fprintf(fff,"ext4/ext4_discard_blocks"); break;278
case :fprintf(fff,"ext4/ext4_discard_preallocations"); break;273
case :fprintf(fff,"ext4/ext4_drop_inode"); break;295
case :fprintf(fff,"ext4/ext4_es_cache_extent"); break;218
case :fprintf(fff,"ext4/ext4_es_find_delayed_extent_range_enter"); break;216
#endif
		case 215:	fprintf(fff,"ext4/ext4_es_find_delayed_extent_range_exit"); break;
#if 0
case :fprintf(fff,"ext4/ext4_es_insert_extent"); break;219
case :fprintf(fff,"ext4/ext4_es_lookup_extent_enter"); break;214
case :fprintf(fff,"ext4/ext4_es_lookup_extent_exit"); break;213
case :fprintf(fff,"ext4/ext4_es_remove_extent"); break;217
case :fprintf(fff,"ext4/ext4_es_shrink_enter"); break;212
case :fprintf(fff,"ext4/ext4_es_shrink_exit"); break;211
case :fprintf(fff,"ext4/ext4_evict_inode"); break;296
case :fprintf(fff,"ext4/ext4_ext_convert_to_initialized_enter"); break;243
case :fprintf(fff,"ext4/ext4_ext_convert_to_initialized_fastpath"); break;242
case :fprintf(fff,"ext4/ext4_ext_handle_uninitialized_extents"); break;231
	#endif
		case 228:	fprintf(fff,"ext4/ext4_ext_in_cache"); break;
		case 237:	fprintf(fff,"ext4/ext4_ext_load_extent"); break;
		case 241:	fprintf(fff,"ext4/ext4_ext_map_blocks_enter"); break;
		case 239:	fprintf(fff,"ext4/ext4_ext_map_blocks_exit"); break;
		case 229:	fprintf(fff,"ext4/ext4_ext_put_in_cache"); break;
		case 220:	fprintf(fff,"ext4/ext4_ext_remove_space_done"); break;
		case 221:	fprintf(fff,"ext4/ext4_ext_remove_space"); break;
		case 222:	fprintf(fff,"ext4/ext4_ext_rm_idx"); break;
		case 223:	fprintf(fff,"ext4/ext4_ext_rm_leaf"); break;
		case 225:	fprintf(fff,"ext4/ext4_ext_show_extent"); break;
		case 250:	fprintf(fff,"ext4/ext4_fallocate_enter"); break;
		case 249:	fprintf(fff,"ext4/ext4_fallocate_exit"); break;
		case 227:	fprintf(fff,"ext4/ext4_find_delalloc_range"); break;
		case 260:	fprintf(fff,"ext4/ext4_forget"); break;
		case 269:	fprintf(fff,"ext4/ext4_free_blocks"); break;
		case 299:	fprintf(fff,"ext4/ext4_free_inode"); break;
		case 230:	fprintf(fff,"ext4/ext4_get_implied_cluster_alloc_exit"); break;
		case 226:	fprintf(fff,"ext4/ext4_get_reserved_cluster_alloc"); break;
		case 240:	fprintf(fff,"ext4/ext4_ind_map_blocks_enter"); break;
		case 238:	fprintf(fff,"ext4/ext4_ind_map_blocks_exit"); break;
		case 280:	fprintf(fff,"ext4/ext4_invalidatepage"); break;
		case 279:	fprintf(fff,"ext4/ext4_journalled_invalidatepage"); break;
		case 289:	fprintf(fff,"ext4/ext4_journalled_write_end"); break;
		case 235:	fprintf(fff,"ext4/ext4_journal_start"); break;
		case 234:	fprintf(fff,"ext4/ext4_journal_start_reserved"); break;
		case 253:	fprintf(fff,"ext4/ext4_load_inode_bitmap"); break;
		case 236:	fprintf(fff,"ext4/ext4_load_inode"); break;
		case 294:	fprintf(fff,"ext4/ext4_mark_inode_dirty"); break;
		case 264:	fprintf(fff,"ext4/ext4_mballoc_alloc"); break;
		case 262:	fprintf(fff,"ext4/ext4_mballoc_discard"); break;
		case 261:	fprintf(fff,"ext4/ext4_mballoc_free"); break;
		case 263:	fprintf(fff,"ext4/ext4_mballoc_prealloc"); break;
		case 256:	fprintf(fff,"ext4/ext4_mb_bitmap_load"); break;
		case 255:	fprintf(fff,"ext4/ext4_mb_buddy_bitmap_load"); break;
		case 272:	fprintf(fff,"ext4/ext4_mb_discard_preallocations"); break;
		case 276:	fprintf(fff,"ext4/ext4_mb_new_group_pa"); break;
		case 277:	fprintf(fff,"ext4/ext4_mb_new_inode_pa"); break;
		case 274:	fprintf(fff,"ext4/ext4_mb_release_group_pa"); break;
		case 275:	fprintf(fff,"ext4/ext4_mb_release_inode_pa"); break;
		case 248:	fprintf(fff,"ext4/ext4_punch_hole"); break;
		case 254:	fprintf(fff,"ext4/ext4_read_block_bitmap_load"); break;
	#if 0
case :fprintf(fff,"ext4/ext4_readpage"); break;282
case :fprintf(fff,"ext4/ext4_releasepage"); break;281
case :fprintf(fff,"ext4/ext4_remove_blocks"); break;224
case :fprintf(fff,"ext4/ext4_request_blocks"); break;271
case :fprintf(fff,"ext4/ext4_request_inode"); break;298
case :fprintf(fff,"ext4/ext4_sync_file_enter"); break;268
case :fprintf(fff,"ext4/ext4_sync_file_exit"); break;267
case :fprintf(fff,"ext4/ext4_sync_fs"); break;266
case :fprintf(fff,"ext4/ext4_trim_all_free"); break;232
case :fprintf(fff,"ext4/ext4_trim_extent"); break;233
case :fprintf(fff,"ext4/ext4_truncate_enter"); break;245
	#endif
		case 244:	fprintf(fff,"ext4/ext4_truncate_exit"); break;
		case 247:	fprintf(fff,"ext4/ext4_unlink_enter"); break;
		case 246:	fprintf(fff,"ext4/ext4_unlink_exit"); break;
		case 292:	fprintf(fff,"ext4/ext4_write_begin"); break;
		case 290:	fprintf(fff,"ext4/ext4_write_end"); break;
		case 283:	fprintf(fff,"ext4/ext4_writepage"); break;
		case 287:	fprintf(fff,"ext4/ext4_writepages"); break;
		case 284:	fprintf(fff,"ext4/ext4_writepages_result"); break;
		case 115:	fprintf(fff,"filemap/mm_filemap_add_to_page_cache"); break;
		case 116:	fprintf(fff,"filemap/mm_filemap_delete_from_page_cache"); break;
		case 1:		fprintf(fff,"ftrace/function"); break;
		case 451:	fprintf(fff,"gpio/gpio_direction"); break;
		case 450:	fprintf(fff,"gpio/gpio_value"); break;
		case 47:	fprintf(fff,"irq/irq_handler_entry"); break;
		case 46:	fprintf(fff,"irq/irq_handler_exit"); break;
		case 45:	fprintf(fff,"irq/softirq_entry"); break;
		case 44:	fprintf(fff,"irq/softirq_exit"); break;
		case 43:	fprintf(fff,"irq/softirq_raise"); break;
		case 23:	fprintf(fff,"irq_vectors/call_function_entry"); break;
		case 22:	fprintf(fff,"irq_vectors/call_function_exit"); break;
		case 21:	fprintf(fff,"irq_vectors/call_function_single_entry"); break;
		case 20:	fprintf(fff,"irq_vectors/call_function_single_exit"); break;
		case 29:	fprintf(fff,"irq_vectors/error_apic_entry"); break;
		case 28:	fprintf(fff,"irq_vectors/error_apic_exit"); break;
		case 25:	fprintf(fff,"irq_vectors/irq_work_entry"); break;
		case 24:	fprintf(fff,"irq_vectors/irq_work_exit"); break;
		case 35:	fprintf(fff,"irq_vectors/local_timer_entry"); break;
		case 34:	fprintf(fff,"irq_vectors/local_timer_exit"); break;
		case 33:	fprintf(fff,"irq_vectors/reschedule_entry"); break;
		case 32:	fprintf(fff,"irq_vectors/reschedule_exit"); break;
		case 31:	fprintf(fff,"irq_vectors/spurious_apic_entry"); break;
		case 30:	fprintf(fff,"irq_vectors/spurious_apic_exit"); break;
		case 17:	fprintf(fff,"irq_vectors/thermal_apic_entry"); break;
		case 16:	fprintf(fff,"irq_vectors/thermal_apic_exit"); break;
		case 19:	fprintf(fff,"irq_vectors/threshold_apic_entry"); break;
		case 18:	fprintf(fff,"irq_vectors/threshold_apic_exit"); break;
		case 27:	fprintf(fff,"irq_vectors/x86_platform_ipi_entry"); break;
		case 26:	fprintf(fff,"irq_vectors/x86_platform_ipi_exit"); break;
#if 0
case :fprintf(fff,"jbd2/jbd2_checkpoint"); break;325
case :fprintf(fff,"jbd2/jbd2_checkpoint_stats"); break;313
case :fprintf(fff,"jbd2/jbd2_commit_flushing"); break;322
case :fprintf(fff,"jbd2/jbd2_commit_locking"); break;323
case :fprintf(fff,"jbd2/jbd2_commit_logging"); break;321
case :fprintf(fff,"jbd2/jbd2_drop_transaction"); break;320
case :fprintf(fff,"jbd2/jbd2_end_commit"); break;319
case :fprintf(fff,"jbd2/jbd2_handle_extend"); break;316
case :fprintf(fff,"jbd2/jbd2_handle_start"); break;317
case :fprintf(fff,"jbd2/jbd2_handle_stats"); break;315
case :fprintf(fff,"jbd2/jbd2_lock_buffer_stall"); break;310
case :fprintf(fff,"jbd2/jbd2_run_stats"); break;314
case :fprintf(fff,"jbd2/jbd2_start_commit"); break;324
case :fprintf(fff,"jbd2/jbd2_submit_inode_data"); break;318
case :fprintf(fff,"jbd2/jbd2_update_log_tail"); break;312
case :fprintf(fff,"jbd2/jbd2_write_superblock"); break;311
case :fprintf(fff,"jbd/jbd_checkpoint"); break;309
case :fprintf(fff,"jbd/jbd_cleanup_journal_tail"); break;301
case :fprintf(fff,"jbd/jbd_commit_flushing"); break;306
case :fprintf(fff,"jbd/jbd_commit_locking"); break;307
case :fprintf(fff,"jbd/jbd_commit_logging"); break;305
case :fprintf(fff,"jbd/jbd_do_submit_data"); break;302
case :fprintf(fff,"jbd/jbd_drop_transaction"); break;304
case :fprintf(fff,"jbd/jbd_end_commit"); break;303
case :fprintf(fff,"jbd/jbd_start_commit"); break;308
case :fprintf(fff,"jbd/journal_write_superblock"); break;300
	#endif
		case 142:	fprintf(fff,"kmem/kfree"); break;
		case 146:	fprintf(fff,"kmem/kmalloc"); break;
		case 144:	fprintf(fff,"kmem/kmalloc_node"); break;
		case 145:	fprintf(fff,"kmem/kmem_cache_alloc"); break;
		case 143:	fprintf(fff,"kmem/kmem_cache_alloc_node"); break;
		case 141:	fprintf(fff,"kmem/kmem_cache_free"); break;
		case 135:	fprintf(fff,"kmem/mm_page_alloc_extfrag"); break;
		case 138:	fprintf(fff,"kmem/mm_page_alloc"); break;
		case 137:	fprintf(fff,"kmem/mm_page_alloc_zone_locked"); break;
		case 139:	fprintf(fff,"kmem/mm_page_free_batched"); break;
		case 140:	fprintf(fff,"kmem/mm_page_free"); break;
		case 136:	fprintf(fff,"kmem/mm_page_pcpu_drain"); break;
		case 40:	fprintf(fff,"mce/mce_record"); break;
		case 147:	fprintf(fff,"migrate/mm_migrate_pages"); break;
	#if 0
case :fprintf(fff,"module/module_free"); break;90
case :fprintf(fff,"module/module_get"); break;89
case :fprintf(fff,"module/module_load"); break;91
case :fprintf(fff,"module/module_put"); break;88
case :fprintf(fff,"module/module_request"); break;87
case :fprintf(fff,"napi/napi_poll"); break;489
case :fprintf(fff,"net/net_dev_queue"); break;492
case :fprintf(fff,"net/net_dev_xmit"); break;493
case :fprintf(fff,"net/netif_receive_skb"); break;491
case :fprintf(fff,"net/netif_rx"); break;490
case :fprintf(fff,"nfs4/nfs4_access"); break;392
case :fprintf(fff,"nfs4/nfs4_bind_conn_to_session"); break;422
case :fprintf(fff,"nfs4/nfs4_cb_sequence"); break;417
case :fprintf(fff,"nfs4/nfs4_close"); break;413
case :fprintf(fff,"nfs4/nfs4_commit"); break;374
	#endif
		case 425:	fprintf(fff,"nfs4/nfs4_create_session"); break;
		case 405:	fprintf(fff,"nfs4/nfs4_delegreturn_exit"); break;
		case 386:	fprintf(fff,"nfs4/nfs4_delegreturn"); break;
	#if 0
case :fprintf(fff,"nfs4/nfs4_destroy_clientid"); break;423
case :fprintf(fff,"nfs4/nfs4_destroy_session"); break;424
case :fprintf(fff,"nfs4/nfs4_exchange_id"); break;426
case :fprintf(fff,"nfs4/nfs4_fsinfo"); break;383
case :fprintf(fff,"nfs4/nfs4_get_acl"); break;389
case :fprintf(fff,"nfs4/nfs4_getattr"); break;385
case :fprintf(fff,"nfs4/nfs4_get_fs_locations"); break;396
case :fprintf(fff,"nfs4/nfs4_get_lock"); break;412
case :fprintf(fff,"nfs4/nfs4_layoutcommit"); break;371
case :fprintf(fff,"nfs4/nfs4_layoutget"); break;372
case :fprintf(fff,"nfs4/nfs4_layoutreturn"); break;370
case :fprintf(fff,"nfs4/nfs4_lock_expired"); break;409
case :fprintf(fff,"nfs4/nfs4_lock_reclaim"); break;410
case :fprintf(fff,"nfs4/nfs4_lookup"); break;401
	#endif
		case 384:	fprintf(fff,"nfs4/nfs4_lookup_root"); break;
	#if 0
case :fprintf(fff,"nfs4/nfs4_map_gid_to_group"); break;379
case :fprintf(fff,"nfs4/nfs4_map_group_to_gid"); break;381
case :fprintf(fff,"nfs4/nfs4_map_name_to_uid"); break;382
case :fprintf(fff,"nfs4/nfs4_map_uid_to_name"); break;380
case :fprintf(fff,"nfs4/nfs4_mkdir"); break;399
case :fprintf(fff,"nfs4/nfs4_mknod"); break;398
case :fprintf(fff,"nfs4/nfs4_open_expired"); break;415
case :fprintf(fff,"nfs4/nfs4_open_file"); break;414
case :fprintf(fff,"nfs4/nfs4_open_reclaim"); break;416
case :fprintf(fff,"nfs4/nfs4_pnfs_commit_ds"); break;373
case :fprintf(fff,"nfs4/nfs4_pnfs_read"); break;377
case :fprintf(fff,"nfs4/nfs4_pnfs_write"); break;375
case :fprintf(fff,"nfs4/nfs4_readdir"); break;390
case :fprintf(fff,"nfs4/nfs4_read"); break;378
case :fprintf(fff,"nfs4/nfs4_readlink"); break;391
case :fprintf(fff,"nfs4/nfs4_recall_delegation"); break;387
case :fprintf(fff,"nfs4/nfs4_reclaim_complete"); break;420
case :fprintf(fff,"nfs4/nfs4_reclaim_delegation"); break;406
case :fprintf(fff,"nfs4/nfs4_remove"); break;397
case :fprintf(fff,"nfs4/nfs4_rename"); break;394
case :fprintf(fff,"nfs4/nfs4_renew_async"); break;427
	#endif
		case 428:	fprintf(fff,"nfs4/nfs4_renew"); break;
	#if 0
case :fprintf(fff,"nfs4/nfs4_secinfo"); break;395
case :fprintf(fff,"nfs4/nfs4_sequence_done"); break;418
case :fprintf(fff,"nfs4/nfs4_sequence"); break;421
case :fprintf(fff,"nfs4/nfs4_set_acl"); break;388
case :fprintf(fff,"nfs4/nfs4_setattr"); break;393
case :fprintf(fff,"nfs4/nfs4_setclientid_confirm"); break;429
case :fprintf(fff,"nfs4/nfs4_setclientid"); break;430
case :fprintf(fff,"nfs4/nfs4_set_delegation"); break;407
case :fprintf(fff,"nfs4/nfs4_set_lock"); break;411
case :fprintf(fff,"nfs4/nfs4_setup_sequence"); break;419
case :fprintf(fff,"nfs4/nfs4_symlink"); break;400
case :fprintf(fff,"nfs4/nfs4_test_delegation_stateid"); break;404
case :fprintf(fff,"nfs4/nfs4_test_lock_stateid"); break;402
case :fprintf(fff,"nfs4/nfs4_test_open_stateid"); break;403
case :fprintf(fff,"nfs4/nfs4_unlock"); break;408
case :fprintf(fff,"nfs4/nfs4_write"); break;376
case :fprintf(fff,"nfs/nfs_access_enter"); break;353
case :fprintf(fff,"nfs/nfs_access_exit"); break;352
	#endif
		case 347:	fprintf(fff,"nfs/nfs_atomic_open_enter"); break;
	#if 0
case :fprintf(fff,"nfs/nfs_atomic_open_exit"); break;346
case :fprintf(fff,"nfs/nfs_create_enter"); break;345
case :fprintf(fff,"nfs/nfs_create_exit"); break;344
case :fprintf(fff,"nfs/nfs_fsync_enter"); break;355
case :fprintf(fff,"nfs/nfs_fsync_exit"); break;354
case :fprintf(fff,"nfs/nfs_getattr_enter"); break;363
case :fprintf(fff,"nfs/nfs_getattr_exit"); break;362
case :fprintf(fff,"nfs/nfs_invalidate_mapping_enter"); break;365
case :fprintf(fff,"nfs/nfs_invalidate_mapping_exit"); break;364
case :fprintf(fff,"nfs/nfs_link_enter"); break;331
case :fprintf(fff,"nfs/nfs_link_exit"); break;330
case :fprintf(fff,"nfs/nfs_lookup_enter"); break;351
case :fprintf(fff,"nfs/nfs_lookup_exit"); break;350
case :fprintf(fff,"nfs/nfs_lookup_revalidate_enter"); break;349
case :fprintf(fff,"nfs/nfs_lookup_revalidate_exit"); break;348
case :fprintf(fff,"nfs/nfs_mkdir_enter"); break;341
case :fprintf(fff,"nfs/nfs_mkdir_exit"); break;340
case :fprintf(fff,"nfs/nfs_mknod_enter"); break;343
case :fprintf(fff,"nfs/nfs_mknod_exit"); break;342
	#endif
	case 369:	fprintf(fff,"nfs/nfs_refresh_inode_enter"); break;
	case 368:	fprintf(fff,"nfs/nfs_refresh_inode_exit"); break;
	case 337:	fprintf(fff,"nfs/nfs_remove_enter"); break;
	case 336:	fprintf(fff,"nfs/nfs_remove_exit"); break;
	#if 0
case :fprintf(fff,"nfs/nfs_rename_enter"); break;329
case :fprintf(fff,"nfs/nfs_rename_exit"); break;328
case :fprintf(fff,"nfs/nfs_revalidate_inode_enter"); break;367
case :fprintf(fff,"nfs/nfs_revalidate_inode_exit"); break;366
case :fprintf(fff,"nfs/nfs_rmdir_enter"); break;339
case :fprintf(fff,"nfs/nfs_rmdir_exit"); break;338
case :fprintf(fff,"nfs/nfs_setattr_enter"); break;361
case :fprintf(fff,"nfs/nfs_setattr_exit"); break;360
case :fprintf(fff,"nfs/nfs_sillyrename_rename"); break;327
case :fprintf(fff,"nfs/nfs_sillyrename_unlink"); break;326
case :fprintf(fff,"nfs/nfs_symlink_enter"); break;333
case :fprintf(fff,"nfs/nfs_symlink_exit"); break;332
case :fprintf(fff,"nfs/nfs_unlink_enter"); break;335
case :fprintf(fff,"nfs/nfs_unlink_exit"); break;334
case :fprintf(fff,"nfs/nfs_writeback_inode_enter"); break;357
case :fprintf(fff,"nfs/nfs_writeback_inode_exit"); break;356
case :fprintf(fff,"nfs/nfs_writeback_page_enter"); break;359
case :fprintf(fff,"nfs/nfs_writeback_page_exit"); break;358
	#endif
		case 36:	fprintf(fff,"nmi/nmi_handler"); break;
		case 117:	fprintf(fff,"oom/oom_score_adj_update"); break;
		case 118:	fprintf(fff,"pagemap/mm_lru_activate"); break;
		case 119:	fprintf(fff,"pagemap/mm_lru_insertion"); break;
		case 103:	fprintf(fff,"power/clock_disable"); break;
		case 104:	fprintf(fff,"power/clock_enable"); break;
		case 102:	fprintf(fff,"power/clock_set_rate"); break;
		case 109:	fprintf(fff,"power/cpu_frequency"); break;
		case 110:	fprintf(fff,"power/cpu_idle"); break;
		case 107:	fprintf(fff,"power/device_pm_report_time"); break;
		case 94:	fprintf(fff,"power/dev_pm_qos_add_request"); break;
		case 92:	fprintf(fff,"power/dev_pm_qos_remove_request"); break;
		case 93:	fprintf(fff,"power/dev_pm_qos_update_request"); break;
		case 108:	fprintf(fff,"power/machine_suspend"); break;
		case 100:	fprintf(fff,"power/pm_qos_add_request"); break;
		case 98:	fprintf(fff,"power/pm_qos_remove_request"); break;
		case 95:	fprintf(fff,"power/pm_qos_update_flags"); break;
		case 99:	fprintf(fff,"power/pm_qos_update_request"); break;
		case 97:	fprintf(fff,"power/pm_qos_update_request_timeout"); break;
		case 96:	fprintf(fff,"power/pm_qos_update_target"); break;
		case 101:	fprintf(fff,"power/power_domain_target"); break;
		case 106:	fprintf(fff,"power/wakeup_source_activate"); break;
		case 105:	fprintf(fff,"power/wakeup_source_deactivate"); break;
		case 86:	fprintf(fff,"printk/console"); break;
		case 463:	fprintf(fff,"random/credit_entropy_bits"); break;
		case 461:	fprintf(fff,"random/extract_entropy"); break;
		case 460:	fprintf(fff,"random/extract_entropy_user"); break;
		case 462:	fprintf(fff,"random/get_random_bytes"); break;
		case 465:	fprintf(fff,"random/mix_pool_bytes"); break;
		case 464:	fprintf(fff,"random/mix_pool_bytes_nolock"); break;
		case 452:	fprintf(fff,"ras/aer_event"); break;
		case 39:	fprintf(fff,"raw_syscalls/sys_enter"); break;
		case 38:	fprintf(fff,"raw_syscalls/sys_exit"); break;
		case 67:	fprintf(fff,"rcu/rcu_utilization"); break;
#if 0
case :fprintf(fff,"regmap/regcache_drop_region"); break;466
case :fprintf(fff,"regmap/regcache_sync"); break;473
case :fprintf(fff,"regmap/regmap_async_complete_done"); break;467
case :fprintf(fff,"regmap/regmap_async_complete_start"); break;468
case :fprintf(fff,"regmap/regmap_async_io_complete"); break;469
case :fprintf(fff,"regmap/regmap_async_write_start"); break;470
case :fprintf(fff,"regmap/regmap_cache_bypass"); break;471
case :fprintf(fff,"regmap/regmap_cache_only"); break;472
case :fprintf(fff,"regmap/regmap_hw_read_done"); break;476
case :fprintf(fff,"regmap/regmap_hw_read_start"); break;477
case :fprintf(fff,"regmap/regmap_hw_write_done"); break;474
case :fprintf(fff,"regmap/regmap_hw_write_start"); break;475
case :fprintf(fff,"regmap/regmap_reg_read_cache"); break;478
case :fprintf(fff,"regmap/regmap_reg_read"); break;479
case :fprintf(fff,"regmap/regmap_reg_write"); break;480
#endif
		case 455:	fprintf(fff,"regulator/regulator_disable_complete"); break;
		case 456:	fprintf(fff,"regulator/regulator_disable"); break;
		case 457:	fprintf(fff,"regulator/regulator_enable_complete"); break;
		case 458:	fprintf(fff,"regulator/regulator_enable_delay"); break;
		case 459:	fprintf(fff,"regulator/regulator_enable"); break;
		case 453:	fprintf(fff,"regulator/regulator_set_voltage_complete"); break;
		case 454:	fprintf(fff,"regulator/regulator_set_voltage"); break;
		case 112:	fprintf(fff,"rpm/rpm_idle"); break;
		case 113:	fprintf(fff,"rpm/rpm_resume"); break;
		case 111:	fprintf(fff,"rpm/rpm_return_int"); break;
		case 114:	fprintf(fff,"rpm/rpm_suspend"); break;
#if 0
case :fprintf(fff,"sched/sched_kthread_stop"); break;85
case :fprintf(fff,"sched/sched_kthread_stop_ret"); break;84
case :fprintf(fff,"sched/sched_migrate_task"); break;80
case :fprintf(fff,"sched/sched_pi_setprio"); break;68
case :fprintf(fff,"sched/sched_process_exec"); break;74
case :fprintf(fff,"sched/sched_process_exit"); break;78
case :fprintf(fff,"sched/sched_process_fork"); break;75
case :fprintf(fff,"sched/sched_process_free"); break;79
case :fprintf(fff,"sched/sched_process_wait"); break;76
case :fprintf(fff,"sched/sched_stat_blocked"); break;70
case :fprintf(fff,"sched/sched_stat_iowait"); break;71
case :fprintf(fff,"sched/sched_stat_runtime"); break;69
#endif
		case 72:	fprintf(fff,"sched/sched_stat_sleep"); break;
		case 73:	fprintf(fff,"sched/sched_stat_wait"); break;
		case 81:	fprintf(fff,"sched/sched_switch"); break;
		case 77:	fprintf(fff,"sched/sched_wait_task"); break;
		case 83:	fprintf(fff,"sched/sched_wakeup"); break;
		case 82:	fprintf(fff,"sched/sched_wakeup_new"); break;
		case 483:	fprintf(fff,"scsi/scsi_dispatch_cmd_done"); break;
		case 484:	fprintf(fff,"scsi/scsi_dispatch_cmd_error"); break;
		case 485:	fprintf(fff,"scsi/scsi_dispatch_cmd_start"); break;
		case 482:	fprintf(fff,"scsi/scsi_dispatch_cmd_timeout"); break;
		case 481:	fprintf(fff,"scsi/scsi_eh_wakeup"); break;
		case 61:	fprintf(fff,"signal/signal_deliver"); break;
		case 62:	fprintf(fff,"signal/signal_generate"); break;
#if 0
case :fprintf(fff,"skb/consume_skb"); break;495
case :fprintf(fff,"skb/kfree_skb"); break;496
case :fprintf(fff,"skb/skb_copy_datagram_iovec"); break;494
case :fprintf(fff,"sock/sock_exceed_buf_limit"); break;487
case :fprintf(fff,"sock/sock_rcvqueue_full"); break;488
#endif
		case 508:	fprintf(fff,"sunrpc/rpc_bind_status"); break;
		case 509:	fprintf(fff,"sunrpc/rpc_call_status"); break;
		case 507:	fprintf(fff,"sunrpc/rpc_connect_status"); break;
		case 498:	fprintf(fff,"sunrpc/rpc_socket_close"); break;
		case 500:	fprintf(fff,"sunrpc/rpc_socket_connect"); break;
		case 499:	fprintf(fff,"sunrpc/rpc_socket_reset_connection"); break;
		case 497:	fprintf(fff,"sunrpc/rpc_socket_shutdown"); break;
		case 501:	fprintf(fff,"sunrpc/rpc_socket_state_change"); break;
		case 506:	fprintf(fff,"sunrpc/rpc_task_begin"); break;
		case 504:	fprintf(fff,"sunrpc/rpc_task_complete"); break;
		case 505:	fprintf(fff,"sunrpc/rpc_task_run_action"); break;
		case 503:	fprintf(fff,"sunrpc/rpc_task_sleep"); break;
		case 502:	fprintf(fff,"sunrpc/rpc_task_wakeup"); break;
		case 42:	fprintf(fff,"task/task_newtask"); break;
		case 41:	fprintf(fff,"task/task_rename"); break;
		case 51:	fprintf(fff,"timer/hrtimer_cancel"); break;
		case 53:	fprintf(fff,"timer/hrtimer_expire_entry"); break;
		case 52:	fprintf(fff,"timer/hrtimer_expire_exit"); break;
		case 55:	fprintf(fff,"timer/hrtimer_init"); break;
		case 54:	fprintf(fff,"timer/hrtimer_start"); break;
		case 49:	fprintf(fff,"timer/itimer_expire"); break;
		case 50:	fprintf(fff,"timer/itimer_state"); break;
		case 48:	fprintf(fff,"timer/tick_stop"); break;
		case 56:	fprintf(fff,"timer/timer_cancel"); break;
		case 58:	fprintf(fff,"timer/timer_expire_entry"); break;
		case 57:	fprintf(fff,"timer/timer_expire_exit"); break;
		case 60:	fprintf(fff,"timer/timer_init"); break;
		case 59:	fprintf(fff,"timer/timer_start"); break;
		case 486:	fprintf(fff,"udp/udp_fail_queue_rcv_skb"); break;
		case 124:	fprintf(fff,"vmscan/mm_shrink_slab_end"); break;
		case 125:	fprintf(fff,"vmscan/mm_shrink_slab_start"); break;
		case 131:	fprintf(fff,"vmscan/mm_vmscan_direct_reclaim_begin"); break;
		case 128:	fprintf(fff,"vmscan/mm_vmscan_direct_reclaim_end"); break;
		case 134:	fprintf(fff,"vmscan/mm_vmscan_kswapd_sleep"); break;
		case 133:	fprintf(fff,"vmscan/mm_vmscan_kswapd_wake"); break;
		case 123:	fprintf(fff,"vmscan/mm_vmscan_lru_isolate"); break;
		case 120:	fprintf(fff,"vmscan/mm_vmscan_lru_shrink_inactive"); break;
		case 122:	fprintf(fff,"vmscan/mm_vmscan_memcg_isolate"); break;
		case 130:	fprintf(fff,"vmscan/mm_vmscan_memcg_reclaim_begin"); break;
		case 127:	fprintf(fff,"vmscan/mm_vmscan_memcg_reclaim_end"); break;
		case 129:	fprintf(fff,"vmscan/mm_vmscan_memcg_softlimit_reclaim_begin"); break;
		case 126:	fprintf(fff,"vmscan/mm_vmscan_memcg_softlimit_reclaim_end"); break;
		case 132:	fprintf(fff,"vmscan/mm_vmscan_wakeup_kswapd"); break;
		case 121:	fprintf(fff,"vmscan/mm_vmscan_writepage"); break;
		case 37:	fprintf(fff,"vsyscall/emulate_vsyscall"); break;
		case 65:	fprintf(fff,"workqueue/workqueue_activate_work"); break;
		case 63:	fprintf(fff,"workqueue/workqueue_execute_end"); break;
		case 64:	fprintf(fff,"workqueue/workqueue_execute_start"); break;
		case 66:	fprintf(fff,"workqueue/workqueue_queue_work"); break;
		case 153:	fprintf(fff,"writeback/balance_dirty_pages"); break;
		case 154:	fprintf(fff,"writeback/bdi_dirty_ratelimit"); break;
		case 155:	fprintf(fff,"writeback/global_dirty_state"); break;
		case 157:	fprintf(fff,"writeback/wbc_writepage"); break;
		case 159:	fprintf(fff,"writeback/writeback_bdi_register"); break;
		case 158:	fprintf(fff,"writeback/writeback_bdi_unregister"); break;
		case 151:	fprintf(fff,"writeback/writeback_congestion_wait"); break;
		case 170:	fprintf(fff,"writeback/writeback_dirty_inode"); break;
#if 0
case :fprintf(fff,"writeback/writeback_dirty_inode_start"); break;171
case :fprintf(fff,"writeback/writeback_dirty_page"); break;172
case :fprintf(fff,"writeback/writeback_exec"); break;166
case :fprintf(fff,"writeback/writeback_nowork"); break;161
case :fprintf(fff,"writeback/writeback_pages_written"); break;162
case :fprintf(fff,"writeback/writeback_queue"); break;167
case :fprintf(fff,"writeback/writeback_queue_io"); break;156
case :fprintf(fff,"writeback/writeback_sb_inodes_requeue"); break;152
case :fprintf(fff,"writeback/writeback_single_inode"); break;148
case :fprintf(fff,"writeback/writeback_single_inode_start"); break;149
case :fprintf(fff,"writeback/writeback_start"); break;165
case :fprintf(fff,"writeback/writeback_wait"); break;163
case :fprintf(fff,"writeback/writeback_wait_iff_congested"); break;150
case :fprintf(fff,"writeback/writeback_wake_background"); break;160
case :fprintf(fff,"writeback/writeback_write_inode"); break;168
#endif
		case 169:	fprintf(fff,"writeback/writeback_write_inode_start"); break;
		case 164:	fprintf(fff,"writeback/writeback_written"); break;
		default:	fprintf(fff,"Unknown");	break;
	}
}

