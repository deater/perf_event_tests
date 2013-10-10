#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "../include/hw_breakpoint.h"
#include "../include/perf_event.h"
#include "../include/perf_helpers.h"

#include "perf_attr_print.h"

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
			fprintf(fff,"0x%llx",cache_id);
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
			fprintf(fff,"0x%llx",hw_cache_op_id);
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
					fprintf(fff,"0x%llx",config);
					break;
			}
			break;
		case PERF_TYPE_SOFTWARE:
			switch(type) {
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
					fprintf(fff,"0x%llx",config);
			}
			break;
                case PERF_TYPE_HW_CACHE:
			perf_pretty_print_cache(fff,config);
			break;

		case PERF_TYPE_TRACEPOINT:
                case PERF_TYPE_RAW:
                case PERF_TYPE_BREAKPOINT:
		default:
			fprintf(fff,"0x%llx",config);
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
	fprintf(fff,";\n");

	if (pe->freq) {
		if (pe->sample_freq) fprintf(fff,"\tpe[%d].sample_freq=0x%llx;\n",fd,pe->sample_freq);
	}
	else {
		if (pe->sample_period) fprintf(fff,"\tpe[%d].sample_period=0x%llx;\n",fd,pe->sample_period);
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


	if (pe->watermark) {
		fprintf(fff,"\tpe[%d].wakeup_watermark=%d;\n",fd,pe->wakeup_watermark);
	}
	else {
		fprintf(fff,"\tpe[%d].wakeup_events=%d;\n",fd,pe->wakeup_events);
	}

	if (pe->bp_type==HW_BREAKPOINT_EMPTY) {
		fprintf(fff,"\tpe[%d].bp_type=HW_BREAKPOINT_EMPTY;\n",fd);
		if (pe->config1) fprintf(fff,"\tpe[%d].config1=0x%llx;\n",fd,pe->config1);
		if (pe->config2) fprintf(fff,"\tpe[%d].config2=0x%llx;\n",fd,pe->config2);
	}
	else {
		fprintf(fff,"\tpe[%d].bp_type=",fd);
		perf_pretty_print_breakpoint_type(fff,pe->bp_type);
		fprintf(fff,"; /*%d*/\n",pe->bp_type);
		fprintf(fff,"\tpe[%d].bp_addr=0x%llx;\n",fd,pe->bp_addr);
		fprintf(fff,"\tpe[%d].bp_len=0x%llx;\n",fd,pe->bp_len);
	}
	if (pe->branch_sample_type) {
		fprintf(fff,"\tpe[%d].branch_sample_type=",fd);
		perf_pretty_print_branch_sample_type(fff,pe->branch_sample_type);
		fprintf(fff,";\n");
	}

	if (pe->sample_regs_user) fprintf(fff,"\tpe[%d].sample_regs_user=%lld;\n",fd,pe->sample_regs_user);
	if (pe->sample_stack_user) fprintf(fff,"\tpe[%d].sample_stack_user=%d;\n",fd,pe->sample_stack_user);
	fprintf(fff,"\n");
}

void perf_pretty_print_event(FILE *fff, int fd,
				struct perf_event_attr *pe,
				pid_t pid, int cpu,
				int group_fd, unsigned long flags) {

	fprintf(fff,"/* fd = %d */\n",fd);
	perf_pretty_print_attr(fff,pe,fd);

	fprintf(fff,"\tfd[%d]=perf_event_open(&pe[%d],\n",fd,fd);

	fprintf(fff,"\t\t\t\t%d, ",pid);
	if (pid==0) fprintf(fff,"/* current thread */\n");
	else if (pid==-1) fprintf(fff,"/* all processes */\n");
	else fprintf(fff,"/* Only pid %d */\n",pid);

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
