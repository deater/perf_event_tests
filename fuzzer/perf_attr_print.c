#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "../include/hw_breakpoint.h"
#include "../include/perf_event.h"
#include "../include/perf_helpers.h"

#include "perf_attr_print.h"

void perf_pretty_print_flags(long int flags) {

	long int our_flags=flags;

	if (our_flags==0) {
		printf("0");
		return;
	}

	if (our_flags&PERF_FLAG_FD_NO_GROUP) {
		printf("PERF_FLAG_FD_NO_GROUP");
		our_flags&=~PERF_FLAG_FD_NO_GROUP;
		if (our_flags==0) return;
		printf("|");
	}

	if (our_flags&PERF_FLAG_FD_OUTPUT) {
		printf("PERF_FLAG_FD_OUTPUT");
		our_flags&=~PERF_FLAG_FD_OUTPUT;
		if (our_flags==0) return;
		printf("|");
	}

	if (flags&PERF_FLAG_PID_CGROUP) {
		printf("PERF_FLAG_PID_CGROUP");
		our_flags&=~PERF_FLAG_PID_CGROUP;
		if (our_flags==0) return;
		printf("|");
	}
	printf("0x%lxULL",our_flags);
}

static void perf_pretty_print_type(int type) {

	switch(type) {
		case PERF_TYPE_HARDWARE:
			printf("PERF_TYPE_HARDWARE");
			break;
		case PERF_TYPE_SOFTWARE:
			printf("PERF_TYPE_SOFTWARE");
			break;
		case PERF_TYPE_TRACEPOINT:
			printf("PERF_TYPE_TRACEPOINT");
			break;
		case PERF_TYPE_HW_CACHE:
			printf("PERF_TYPE_HW_CACHE");
			break;
		case PERF_TYPE_RAW:
			printf("PERF_TYPE_RAW");
			break;
		case PERF_TYPE_BREAKPOINT:
			printf("PERF_TYPE_BREAKPOINT");
			break;
		default:
			printf("%d",type);
			break;
	}
}

static void perf_pretty_print_cache(long long config) {


	long long cache_id,hw_cache_op_id,hw_cache_op_result_id;

	cache_id=config&0xff;

	switch(cache_id) {

		case PERF_COUNT_HW_CACHE_L1D:
			printf("PERF_COUNT_HW_CACHE_L1D");
			break;
		case PERF_COUNT_HW_CACHE_L1I:
			printf("PERF_COUNT_HW_CACHE_L1I");
			break;
		case PERF_COUNT_HW_CACHE_LL:
			printf("PERF_COUNT_HW_CACHE_LL");
			break;
		case PERF_COUNT_HW_CACHE_DTLB:
			printf("PERF_COUNT_HW_CACHE_DTLB");
			break;
		case PERF_COUNT_HW_CACHE_ITLB:
			printf("PERF_COUNT_HW_CACHE_ITLB");
			break;
		case PERF_COUNT_HW_CACHE_BPU:
			printf("PERF_COUNT_HW_CACHE_BPU");
			break;
		case PERF_COUNT_HW_CACHE_NODE:
			printf("PERF_COUNT_HW_CACHE_NODE");
			break;
		default:
			printf("0x%llx",cache_id);
			break;
	}

	printf(" | ( ");

	hw_cache_op_id=(config>>8)&0xff;

	switch(hw_cache_op_id) {
		case PERF_COUNT_HW_CACHE_OP_READ:
			printf("PERF_COUNT_HW_CACHE_OP_READ");
			break;
		case PERF_COUNT_HW_CACHE_OP_WRITE:
			printf("PERF_COUNT_HW_CACHE_OP_WRITE");
			break;
		case PERF_COUNT_HW_CACHE_OP_PREFETCH:
			printf("PERF_COUNT_HW_CACHE_OP_PREFETCH");
			break;
		default:
			printf("0x%llx",hw_cache_op_id);
			break;
	}
	printf(" << 8) | ( ");

	hw_cache_op_result_id=(config>>16);
	switch(hw_cache_op_result_id) {
		case PERF_COUNT_HW_CACHE_RESULT_ACCESS:
			printf("PERF_COUNT_HW_CACHE_RESULT_ACCESS");
			break;
		case PERF_COUNT_HW_CACHE_RESULT_MISS:
			printf("PERF_COUNT_HW_CACHE_RESULT_MISS");
			break;
		default:
			printf("0x%llxULL",hw_cache_op_result_id);
			break;
	}

	printf(" << 16 )");


}

static void perf_pretty_print_config(long long type, long long config) {

	switch (type) {
		case PERF_TYPE_HARDWARE:
                        switch(config) {
				case PERF_COUNT_HW_CPU_CYCLES:
					printf("PERF_COUNT_HW_CPU_CYCLES");
                                        break;
				case PERF_COUNT_HW_INSTRUCTIONS:
					printf("PERF_COUNT_HW_INSTRUCTIONS");
					break;
				case PERF_COUNT_HW_CACHE_REFERENCES:
					printf("PERF_COUNT_HW_CACHE_REFERENCES");
					break;
				case PERF_COUNT_HW_CACHE_MISSES:
					printf("PERF_COUNT_HW_CACHE_MISSES");
					break;
				case PERF_COUNT_HW_BRANCH_INSTRUCTIONS:
					printf("PERF_COUNT_HW_BRANCH_INSTRUCTIONS");
					break;
				case PERF_COUNT_HW_BRANCH_MISSES:
					printf("PERF_COUNT_HW_BRANCH_MISSES");
					break;
				case PERF_COUNT_HW_BUS_CYCLES:
					printf("PERF_COUNT_HW_BUS_CYCLES");
					break;
                                case PERF_COUNT_HW_STALLED_CYCLES_FRONTEND:
					printf("PERF_COUNT_HW_STALLED_CYCLES_FRONTEND");
					break;
				case PERF_COUNT_HW_STALLED_CYCLES_BACKEND:
					printf("PERF_COUNT_HW_STALLED_CYCLES_BACKEND");
					break;
				case PERF_COUNT_HW_REF_CPU_CYCLES:
					printf("PERF_COUNT_HW_REF_CPU_CYCLES");
					break;
				default:
					printf("0x%llx",config);
					break;
			}
			break;
		case PERF_TYPE_SOFTWARE:
			switch(type) {
				case PERF_COUNT_SW_CPU_CLOCK:
					printf("PERF_COUNT_SW_CPU_CLOCK");
					break;
				case PERF_COUNT_SW_TASK_CLOCK:
					printf("PERF_COUNT_SW_TASK_CLOCK");
					break;
				case PERF_COUNT_SW_PAGE_FAULTS:
					printf("PERF_COUNT_SW_PAGE_FAULTS");
					break;
				case PERF_COUNT_SW_CONTEXT_SWITCHES:
					printf("PERF_COUNT_SW_CONTEXT_SWITCHES");
					break;
				case PERF_COUNT_SW_CPU_MIGRATIONS:
					printf("PERF_COUNT_SW_CPU_MIGRATIONS");
					break;
				case PERF_COUNT_SW_PAGE_FAULTS_MIN:
					printf("PERF_COUNT_SW_PAGE_FAULTS_MIN");
					break;
				case PERF_COUNT_SW_PAGE_FAULTS_MAJ:
					printf("PERF_COUNT_SW_PAGE_FAULTS_MAJ");
					break;
				case PERF_COUNT_SW_ALIGNMENT_FAULTS:
					printf("PERF_COUNT_SW_ALIGNMENT_FAULTS");
					break;
				case PERF_COUNT_SW_EMULATION_FAULTS:
					printf("PERF_COUNT_SW_EMULATION_FAULTS");
					break;
				default:
					printf("0x%llx",config);
			}
			break;
                case PERF_TYPE_HW_CACHE:
			perf_pretty_print_cache(config);
			break;

		case PERF_TYPE_TRACEPOINT:
                case PERF_TYPE_RAW:
                case PERF_TYPE_BREAKPOINT:
		default:
			printf("0x%llx",config);
			break;
	}


}

static void perf_pretty_print_sample_type(unsigned long long sample_type) {

	unsigned long long our_type=sample_type;

	if (!our_type) {
		printf("0");
		return;
	}

	if (our_type&PERF_SAMPLE_IP) {
		printf("PERF_SAMPLE_IP");
		our_type&=~PERF_SAMPLE_IP;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_TID) {
		printf("PERF_SAMPLE_TID");
		our_type&=~PERF_SAMPLE_TID;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_TIME) {
		printf("PERF_SAMPLE_TIME");
		our_type&=~PERF_SAMPLE_TIME;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_ADDR) {
		printf("PERF_SAMPLE_ADDR");
		our_type&=~PERF_SAMPLE_ADDR;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_READ) {
		printf("PERF_SAMPLE_READ");
		our_type&=~PERF_SAMPLE_READ;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_CALLCHAIN) {
		printf("PERF_SAMPLE_CALLCHAIN");
		our_type&=~PERF_SAMPLE_CALLCHAIN;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_ID) {
		printf("PERF_SAMPLE_ID");
		our_type&=~PERF_SAMPLE_ID;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_CPU) {
		printf("PERF_SAMPLE_CPU");
		our_type&=~PERF_SAMPLE_CPU;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_PERIOD) {
		printf("PERF_SAMPLE_PERIOD");
		our_type&=~PERF_SAMPLE_PERIOD;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_STREAM_ID) {
		printf("PERF_SAMPLE_STREAM_ID");
		our_type&=~PERF_SAMPLE_STREAM_ID;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_RAW) {
		printf("PERF_SAMPLE_RAW");
		our_type&=~PERF_SAMPLE_RAW;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_BRANCH_STACK) {
		printf("PERF_SAMPLE_BRANCH_STACK");
		our_type&=~PERF_SAMPLE_BRANCH_STACK;
		if (!our_type) return;
		printf("|");
	}
	if (our_type&PERF_SAMPLE_REGS_USER) {
		printf("PERF_SAMPLE_REGS_USER");
		our_type&=~PERF_SAMPLE_REGS_USER;
		if (!our_type) return;
		printf("|");
	}

	if (our_type&PERF_SAMPLE_STACK_USER) {
		printf("PERF_SAMPLE_STACK_USER");
		our_type&=~PERF_SAMPLE_STACK_USER;
		if (!our_type) return;
		printf("|");
	}
	printf("0x%llxULL",our_type);


}

static void perf_pretty_print_read_format(unsigned long long read_format) {

	unsigned long long our_format=read_format;

	if (our_format&PERF_FORMAT_TOTAL_TIME_ENABLED) {
		printf("PERF_FORMAT_TOTAL_TIME_ENABLED");
		our_format&=~PERF_FORMAT_TOTAL_TIME_ENABLED;
		if (!our_format) return;
		printf("|");
	}

	if (our_format&PERF_FORMAT_TOTAL_TIME_RUNNING) {
		printf("PERF_FORMAT_TOTAL_TIME_RUNNING");
		our_format&=~PERF_FORMAT_TOTAL_TIME_RUNNING;
		if (!our_format) return;
		printf("|");
	}

	if (our_format&PERF_FORMAT_ID) {
		printf("PERF_FORMAT_ID");
		our_format&=~PERF_FORMAT_ID;
		if (!our_format) return;
		printf("|");
	}

	if (our_format&PERF_FORMAT_GROUP) {
		printf("PERF_FORMAT_GROUP");
		our_format&=~PERF_FORMAT_GROUP;
		if (!our_format) return;
		printf("|");
	}
	printf("0x%llxULL",our_format);

}

static void perf_pretty_print_breakpoint_type(int bp_type) {

	int our_bp=bp_type;

	if (our_bp==HW_BREAKPOINT_EMPTY) {
		printf("HW_BREAKPOINT_EMPTY");
		return;
	}

	if (our_bp&HW_BREAKPOINT_R) {
		printf("HW_BREAKPOINT_R");
		our_bp&=~HW_BREAKPOINT_R;
		if (!our_bp) return;
		printf("|");
	}

	if (our_bp&HW_BREAKPOINT_W) {
		printf("HW_BREAKPOINT_W");
		our_bp&=~HW_BREAKPOINT_W;
		if (!our_bp) return;
		printf("|");
	}

	if (our_bp&HW_BREAKPOINT_X) {
		printf("HW_BREAKPOINT_X");
		our_bp&=~HW_BREAKPOINT_X;
		if (!our_bp) return;
		printf("|");
	}
	printf("0x%x",our_bp);
}

void perf_pretty_print_attr(struct perf_event_attr *pe, int fd) {

	printf("\n");
	printf("\tmemset(&pe[%d],0,sizeof(struct perf_event_attr));\n",
		fd);
	printf("\tpe[%d].type=",fd);
	perf_pretty_print_type(pe->type);
	printf(";\n");

	if (pe->size) printf("\tpe[%d].size=%d;\n",fd,pe->size);

	printf("\tpe[%d].config=",fd);
	perf_pretty_print_config(pe->type,pe->config);
	printf(";\n");

	if (pe->freq) {
		if (pe->sample_freq) printf("\tpe[%d].sample_freq=0x%llx;\n",fd,pe->sample_freq);
	}
	else {
		if (pe->sample_period) printf("\tpe[%d].sample_period=0x%llx;\n",fd,pe->sample_period);
	}

	printf("\tpe[%d].sample_type=",fd);
	perf_pretty_print_sample_type(pe->sample_type);
	printf("; /* %llx */\n",pe->sample_type);

	printf("\tpe[%d].read_format=",fd);
	perf_pretty_print_read_format(pe->read_format);
	printf("; /* %llx */\n",pe->read_format);

	if (pe->disabled) printf("\tpe[%d].disabled=%d;\n",fd,pe->disabled);
	if (pe->inherit) printf("\tpe[%d].inherit=%d;\n",fd,pe->inherit);
	if (pe->pinned) printf("\tpe[%d].pinned=%d;\n",fd,pe->pinned);
	if (pe->exclusive) printf("\tpe[%d].exclusive=%d;\n",fd,pe->exclusive);
	if (pe->exclude_user) printf("\tpe[%d].exclude_user=%d;\n",fd,pe->exclude_user);
	if (pe->exclude_kernel) printf("\tpe[%d].exclude_kernel=%d;\n",fd,pe->exclude_kernel);
	if (pe->exclude_hv) printf("\tpe[%d].exclude_hv=%d;\n",fd,pe->exclude_hv);
	if (pe->exclude_idle) printf("\tpe[%d].exclude_idle=%d;\n",fd,pe->exclude_idle);
	if (pe->mmap) printf("\tpe[%d].mmap=%d;\n",fd,pe->mmap);
	if (pe->comm) printf("\tpe[%d].comm=%d;\n",fd,pe->comm);
	if (pe->freq) printf("\tpe[%d].freq=%d;\n",fd,pe->freq);
	if (pe->inherit_stat) printf("\tpe[%d].inherit_stat=%d;\n",fd,pe->inherit_stat);
	if (pe->enable_on_exec) printf("\tpe[%d].enable_on_exec=%d;\n",fd,pe->enable_on_exec);
	if (pe->task) printf("\tpe[%d].task=%d;\n",fd,pe->task);
	if (pe->watermark) printf("\tpe[%d].watermark=%d;\n",fd,pe->watermark);

	printf("\tpe[%d].precise_ip=",fd);
	switch(pe->precise_ip) {
		case 0:	printf("0; /* arbitrary skid */\n");
			break;
		case 1: printf("1; /* constant skid */\n");
			break;
		case 2: printf("2; /* request zero skid */\n");
			break;
		case 3: printf("3; /* must have zero skid */\n");
			break;
	}

	if (pe->mmap_data) printf("\tpe[%d].mmap_data=%d;\n",fd,pe->mmap_data);
	if (pe->sample_id_all) printf("\tpe[%d].sample_id_all=%d;\n",fd,pe->sample_id_all);
	if (pe->exclude_host) printf("\tpe[%d].exclude_host=%d;\n",fd,pe->exclude_host);
	if (pe->exclude_guest) printf("\tpe[%d].exclude_guest=%d;\n",fd,pe->exclude_guest);
	/* callchain kernel */
	/* callchain user */

	if (pe->watermark) {
		printf("\tpe[%d].wakeup_watermark=%d;\n",fd,pe->wakeup_watermark);
	}
	else {
		printf("\tpe[%d].wakeup_events=%d;\n",fd,pe->wakeup_events);
	}

	if (pe->bp_type==HW_BREAKPOINT_EMPTY) {
		printf("\tpe[%d].bp_type=HW_BREAKPOINT_EMPTY;\n",fd);
		if (pe->config1) printf("\tpe[%d].config1=0x%llx;\n",fd,pe->config1);
		if (pe->config2) printf("\tpe[%d].config2=0x%llx;\n",fd,pe->config2);
	}
	else {
		printf("\tpe[%d].bp_type=",fd);
		perf_pretty_print_breakpoint_type(pe->bp_type);
		printf("; /*%d*/\n",pe->bp_type);
		printf("\tpe[%d].bp_addr=0x%llx;\n",fd,pe->bp_addr);
		printf("\tpe[%d].bp_len=0x%llx;\n",fd,pe->bp_len);
	}
	if (pe->branch_sample_type) printf("\tpe[%d].branch_sample_type=%lld;\n",fd,pe->branch_sample_type);
	/* sample regs user */
	/* sample stack user */
	printf("\n");
}

void perf_pretty_print_event(int fd,
				struct perf_event_attr *pe,
				pid_t pid, int cpu,
				int group_fd, unsigned long flags) {

	printf("/* fd = %d */\n",fd);
	perf_pretty_print_attr(pe,fd);

	printf("\tfd[%d]=perf_event_open(&pe[%d],%d,%d,",
		fd,fd,pid,cpu);
	if (group_fd==-1) {
		printf("-1,");
	}
	else {
		printf("fd[%d],",group_fd);
	}

	perf_pretty_print_flags(flags);
	printf(" /*%lx*/ ",flags);

	printf(");\n");


	printf("\n");

}
