#include <stdio.h>
#include <unistd.h>
#include "perf_event.h"

/* FIXME: error handling? */
int parse_open_event(char *line,
	int *orig_fd, pid_t *pid, int *cpu, int *group_fd, long int *flags,
	struct perf_event_attr *pe) {

	/* I hate bitfields */
	int disabled,inherit,pinned,exclusive;
	int exclude_user,exclude_kernel,exclude_hv,exclude_idle;
	int mmap,comm,freq,inherit_stat;
	int enable_on_exec,task,watermark,precise_ip;
	int mmap_data,sample_id_all,exclude_host,exclude_guest;
	int exclude_callchain_kernel,exclude_callchain_user;
	int mmap2;
	int comm_exec,use_clockid,context_switch,write_backward;
	int namespaces,ksymbol,bpf_event,aux_output;
	int cgroup,text_poke,build_id,inherit_thread,remove_on_exec;
	int sigtrap;

	sscanf(line,
		"%*c %d %d %d %d %lx "
		"%x %x "
		"%llx %llx %llx %llx "

		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d %d "
		"%d %d %d "
		"%d %d %d %d %d %d "

		"%d %d "
		"%llx %llx %lld %lld"
		"%d %d %lld "
		"%d %hd %d",
		orig_fd,pid,cpu,group_fd,flags,
		&pe->type,&pe->size,
		&pe->config,&pe->sample_period,&pe->sample_type,&pe->read_format,

		&disabled,&inherit,&pinned,&exclusive,
		&exclude_user,&exclude_kernel,&exclude_hv,&exclude_idle,
		&mmap,&comm,&freq,&inherit_stat,
		&enable_on_exec,&task,&watermark,&precise_ip,
		&mmap_data,&sample_id_all,&exclude_host,&exclude_guest,
		&exclude_callchain_kernel,&exclude_callchain_user,
		&mmap2,&comm_exec,&use_clockid,&context_switch,
		&write_backward,&namespaces,&ksymbol,&bpf_event,&aux_output,
		&cgroup,&text_poke,&build_id,&inherit_thread,&remove_on_exec,
		&sigtrap,

		&pe->wakeup_events,&pe->bp_type,
		&pe->config1,&pe->config2,&pe->branch_sample_type,
		&pe->sample_regs_user,&pe->sample_stack_user,
		&pe->clockid,&pe->sample_regs_intr,&pe->aux_watermark,
		&pe->sample_max_stack,&pe->aux_sample_size);

	/* re-populate bitfields */
	/* can't sscanf into them */
	pe->disabled=disabled;
	pe->inherit=inherit;
	pe->pinned=pinned;
	pe->exclusive=exclusive;
	pe->exclude_user=exclude_user;
	pe->exclude_kernel=exclude_kernel;
	pe->exclude_hv=exclude_hv;
	pe->exclude_idle=exclude_idle;
	pe->mmap=mmap;
	pe->comm=comm;
	pe->freq=freq;
	pe->inherit_stat=inherit_stat;
	pe->enable_on_exec=enable_on_exec;
	pe->task=task;
	pe->watermark=watermark;
	pe->precise_ip=precise_ip;
	pe->mmap_data=mmap_data;
	pe->sample_id_all=sample_id_all;
	pe->exclude_host=exclude_host;
	pe->exclude_guest=exclude_guest;
	pe->exclude_callchain_user=exclude_callchain_user;
	pe->exclude_callchain_kernel=exclude_callchain_kernel;
	pe->mmap2=mmap2;
	pe->comm_exec=comm_exec;
	pe->use_clockid=use_clockid;
	pe->context_switch=context_switch;
	pe->write_backward=write_backward;
	pe->namespaces=namespaces;
	pe->ksymbol=ksymbol;
	pe->bpf_event=bpf_event;
	pe->aux_output=aux_output;
	pe->cgroup=cgroup;
	pe->text_poke=text_poke;
	pe->build_id=build_id;
	pe->inherit_thread=inherit_thread;
	pe->remove_on_exec=remove_on_exec;
	pe->sigtrap=sigtrap;

	return 0;

}
