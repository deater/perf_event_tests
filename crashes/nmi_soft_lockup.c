/* This code quickly causes 3.10-rc7 on core2 to get stuck   */
/* process never ends, lots of garbage in logs, can't reboot */
/* see the error log at end.                                 */

/* log_to_code output from ./bad.trace.bisect54 */
/* by Vince Weaver <vincent.weaver _at_ maine.edu */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "hw_breakpoint.h"
#include "perf_event.h"

int fd[1024];
struct perf_event_attr pe[1024];

int forked_pid;

int perf_event_open(struct perf_event_attr *hw_event_uptr,
	pid_t pid, int cpu, int group_fd, unsigned long flags) {

	return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		group_fd, flags);
}

int main(int argc, char **argv) {
/* 1 */

	memset(&pe[29],0,sizeof(struct perf_event_attr));
	pe[29].type=PERF_TYPE_HW_CACHE;
	pe[29].size=96;
	pe[29].config=PERF_COUNT_HW_CACHE_DTLB | ( PERF_COUNT_HW_CACHE_OP_READ << 8) | ( PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16 );
	pe[29].sample_type=0; /* 0 */
	pe[29].read_format=PERF_FORMAT_GROUP; /* 8 */
	pe[29].pinned=1;
	pe[29].exclude_kernel=1;
	pe[29].exclude_hv=1;
	pe[29].exclude_idle=1;
	pe[29].mmap=1;
	pe[29].freq=1;
	pe[29].inherit_stat=1;
	pe[29].watermark=1;
	pe[29].precise_ip=0; /* arbitrary skid */
	pe[29].wakeup_watermark=0;
	pe[29].bp_type=HW_BREAKPOINT_EMPTY;

	fd[29]=perf_event_open(&pe[29],0,0,-1,PERF_FLAG_FD_NO_GROUP /*1*/ );

	if (fd[29]<0) {
		printf("Open 29 failed\n");
		exit(1);
	}

/* 2 */

	memset(&pe[42],0,sizeof(struct perf_event_attr));
	pe[42].type=PERF_TYPE_SOFTWARE;
	pe[42].size=80;
	pe[42].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[42].sample_type=0; /* 0 */
	pe[42].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID; /* 7 */
	pe[42].inherit=1;
	pe[42].exclusive=1;
	pe[42].exclude_kernel=1;
	pe[42].exclude_idle=1;
	pe[42].freq=1;
	pe[42].task=1;
	pe[42].watermark=1;
	pe[42].precise_ip=3; /* must have zero skid */
	//pe[42].sample_id_all=1;
	//pe[42].exclude_host=1;
	pe[42].wakeup_watermark=0;
	pe[42].bp_type=HW_BREAKPOINT_EMPTY;

	fd[42]=perf_event_open(&pe[42],0,0,-1,0 /*0*/ );

	if (fd[42]<0) {
		printf("Open 42 failed\n");
		exit(1);
	}


/* 3 */
	forked_pid=fork();
	if (forked_pid==0) while(1);
/* 4 */
	prctl(PR_TASK_PERF_EVENTS_DISABLE);
/* 5 */
	kill(forked_pid,SIGKILL);
/* 6 */
	prctl(PR_TASK_PERF_EVENTS_ENABLE);
/* 7 */

	memset(&pe[15],0,sizeof(struct perf_event_attr));
	pe[15].type=PERF_TYPE_SOFTWARE;
	pe[15].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[15].sample_type=0; /* 0 */
	pe[15].read_format=PERF_FORMAT_ID; /* 4 */
	pe[15].disabled=1;
	pe[15].inherit=1;
	pe[15].exclude_user=1;
	pe[15].exclude_hv=1;
	pe[15].enable_on_exec=1;
	pe[15].task=1;
	pe[15].watermark=1;
	pe[15].precise_ip=2; /* request zero skid */
	//pe[15].mmap_data=1;
	//pe[15].sample_id_all=1;
	//pe[15].exclude_guest=1;
	pe[15].wakeup_watermark=0;
	pe[15].bp_type=HW_BREAKPOINT_EMPTY;

	fd[15]=perf_event_open(&pe[15],0,0,fd[29],0 /*0*/ );

	if (fd[15]<0) {
		printf("Open 15 failed\n");
		exit(1);
	}


/* 8 */
	close(fd[29]);
/* 9 */

	memset(&pe[12],0,sizeof(struct perf_event_attr));
	pe[12].type=PERF_TYPE_SOFTWARE;
	pe[12].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[12].sample_type=0; /* 0 */
	pe[12].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_ID; /* 5 */
	pe[12].disabled=1;
	pe[12].inherit=1;
	pe[12].exclusive=1;
	pe[12].mmap=1;
	pe[12].freq=1;
	pe[12].enable_on_exec=1;
	pe[12].watermark=1;
	pe[12].precise_ip=0; /* arbitrary skid */
	//pe[12].mmap_data=1;
	//	pe[12].exclude_guest=1;
	pe[12].wakeup_watermark=0;
	pe[12].bp_type=HW_BREAKPOINT_EMPTY;

	fd[12]=perf_event_open(&pe[12],0,0,-1,PERF_FLAG_FD_NO_GROUP /*1*/ );

	if (fd[12]<0) {
		printf("Open 12 failed\n");
		exit(1);
	}


/* 10 */
	prctl(PR_TASK_PERF_EVENTS_DISABLE);
/* 11 */
	prctl(PR_TASK_PERF_EVENTS_ENABLE);
/* 12 */

	memset(&pe[4],0,sizeof(struct perf_event_attr));
	pe[4].type=PERF_TYPE_SOFTWARE;
	pe[4].size=96;
	pe[4].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[4].sample_type=0; /* 0 */
	pe[4].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID; /* 7 */
	pe[4].pinned=1;
	pe[4].exclude_user=1;
	pe[4].exclude_hv=1;
	pe[4].exclude_idle=1;
	pe[4].mmap=1;
	pe[4].freq=1;
	pe[4].inherit_stat=1;
	pe[4].task=1;
	pe[4].precise_ip=0; /* arbitrary skid */
	pe[4].wakeup_events=0;
	pe[4].bp_type=HW_BREAKPOINT_EMPTY;

	fd[4]=perf_event_open(&pe[4],0,0,-1,0 /*0*/ );

	if (fd[4]<0) {
		printf("Open 4 failed\n");
		exit(1);
	}


/* 13 */

	memset(&pe[7],0,sizeof(struct perf_event_attr));
	pe[7].type=PERF_TYPE_SOFTWARE;
	pe[7].size=96;
	pe[7].config=PERF_COUNT_SW_TASK_CLOCK;
	pe[7].sample_type=0; /* 0 */
	pe[7].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_GROUP; /* 9 */
	pe[7].exclusive=1;
	pe[7].exclude_user=1;
	pe[7].exclude_kernel=1;
	pe[7].exclude_hv=1;
	pe[7].exclude_idle=1;
	pe[7].comm=1;
	pe[7].inherit_stat=1;
	pe[7].precise_ip=1; /* constant skid */
	//pe[7].exclude_host=1;
	pe[7].wakeup_events=0;
	pe[7].bp_type=HW_BREAKPOINT_EMPTY;

	fd[7]=perf_event_open(&pe[7],0,0,-1,PERF_FLAG_FD_NO_GROUP /*1*/ );

	if (fd[7]<0) {
		printf("Open 7 failed\n");
		exit(1);
	}


/* 14 */
	forked_pid=fork();
	if (forked_pid==0) while(1);
/* 15 */
	kill(forked_pid,SIGKILL);
/* 16 */
	forked_pid=fork();
	if (forked_pid==0) while(1);
/* 17 */
	kill(forked_pid,SIGKILL);
/* 18 */
	close(fd[4]);
/* 19 */

	memset(&pe[4],0,sizeof(struct perf_event_attr));
	pe[4].type=PERF_TYPE_HARDWARE;
	pe[4].size=72;
	pe[4].config=PERF_COUNT_HW_BRANCH_INSTRUCTIONS;
	pe[4].sample_type=0; /* 0 */
	pe[4].read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_ID; /* 7 */
	pe[4].exclude_user=1;
	pe[4].exclude_idle=1;
	pe[4].enable_on_exec=1;
	pe[4].precise_ip=0; /* arbitrary skid */
	//pe[4].mmap_data=1;
	//	pe[4].exclude_host=1;
	//	pe[4].exclude_guest=1;
	pe[4].wakeup_events=0;
	pe[4].bp_type=HW_BREAKPOINT_EMPTY;

	fd[4]=perf_event_open(&pe[4],0,0,-1,PERF_FLAG_FD_OUTPUT /*2*/ );

	if (fd[4]<0) {
		printf("Open 4*2 failed\n");
		exit(1);
	}


/* 20 */
	prctl(PR_TASK_PERF_EVENTS_DISABLE);
	/* Replayed 20 syscalls */
	return 0;
}

/*

Message from syslogd@core2 at Jun 28 15:11:34 ...
 kernel:[  140.248002] BUG: soft lockup - CPU#1 stuck for 23s! [out:3049]

Message from syslogd@core2 at Jun 28 15:12:02 ...
 kernel:[  168.248002] BUG: soft lockup - CPU#1 stuck for 22s! [out:3049]

Message from syslogd@core2 at Jun 28 15:12:38 ...
 kernel:[  204.248002] BUG: soft lockup - CPU#1 stuck for 24s! [out:3049]

Message from syslogd@core2 at Jun 28 15:13:06 ...
 kernel:[  232.248002] BUG: soft lockup - CPU#1 stuck for 23s! [out:3049]

Message from syslogd@core2 at Jun 28 15:13:38 ...
 kernel:[  264.248002] BUG: soft lockup - CPU#1 stuck for 22s! [out:3049]


[   82.239623] ------------[ cut here ]------------                
[   82.243537] WARNING: at kernel/events/core.c:2122 task_ctx_sched_out+0x3c/0x)
[   82.243537] Modules linked in: nfsd auth_rpcgss oid_registry nfs_acl nfs locn
[   82.243537] CPU: 0 PID: 3047 Comm: out Not tainted 3.10.0-rc7 #1             
[   82.243537] Hardware name: AOpen   DE7000/nMCP7ALPx-DE R1.06 Oct.19.2012, BI2
[   82.243537]  0000000000000000 ffffffff8102e0f5 0000000000000000 ffff88011fc18
[   82.243537]  ffff88011799b040 0000000000000000 ffff8801177acecc ffffffff810af
[   82.243537]  ffff8801177acec0 ffffffff810af06f 0000000000000009 ffff880117990
[   82.243537] Call Trace:                                                      
[   82.243537]  [<ffffffff8102e0f5>] ? warn_slowpath_common+0x5b/0x70           
[   82.243537]  [<ffffffff810ab2ff>] ? task_ctx_sched_out+0x3c/0x5f             
[   82.243537]  [<ffffffff810af06f>] ? perf_event_exit_task+0xbf/0x194          
[   82.243537]  [<ffffffff8103291b>] ? do_exit+0x3ef/0x90c                      
[   82.243537]  [<ffffffff810ce101>] ? do_wp_page+0x275/0x5a6                   
[   82.243537]  [<ffffffff81032ec2>] ? do_group_exit+0x66/0x98                  
[   82.243537]  [<ffffffff8103dab9>] ? get_signal_to_deliver+0x479/0x4ad        
[   82.243537]  [<ffffffff810cdd50>] ? __pte_alloc+0x83/0xf2                    
[   82.243537]  [<ffffffff8100205d>] ? do_signal+0x3c/0x432                     
[   82.243537]  [<ffffffff813682a0>] ? __do_page_fault+0x31c/0x3ba              
[   82.243537]  [<ffffffff81002473>] ? do_notify_resume+0x20/0x5d               
[   82.243537]  [<ffffffff813656f5>] ? retint_signal+0x3d/0x78                  
[   82.243537] ---[ end trace 6e0d112623f126d5 ]---                             
[  111.664002] INFO: rcu_sched self-detected stall on CPU[  111.676008] INFO: r)
[  111.676018] sending NMI to all CPUs:                                         
[  111.664002] NMI backtrace for cpu 1                                          
[  111.664002] CPU: 1 PID: 3049 Comm: out Tainted: G        W    3.10.0-rc7 #1  
[  111.664002] Hardware name: AOpen   DE7000/nMCP7ALPx-DE R1.06 Oct.19.2012, BI2
[  111.664002] task: ffff880117b62080 ti: ffff880117a6a000 task.ti: ffff880117a0
[  111.664002] RIP: 0010:[<ffffffff811b8deb>]  [<ffffffff811b8deb>] delay_tsc+06
[  111.664002] RSP: 0018:ffff88011fc83ca0  EFLAGS: 00000083                     
[  111.664002] RAX: 000000000dfb29b6 RBX: ffffffff8183c770 RCX: 000000000dfb218c
[  111.664002] RDX: 000000000000082a RSI: 0000000000000001 RDI: 00000000000009d2
[  111.664002] RBP: 000000000000270b R08: 0000000000000001 R09: ffffffff817caa1f
[  111.664002] R10: 0000000000000000 R11: 0000000000000200 R12: 0000000000000020
[  111.664002] R13: ffffffff812465bc R14: 0000000000000039 R15: 0000000000000039
[  111.664002] FS:  00007fa834fef700(0000) GS:ffff88011fc80000(0000) knlGS:00000
[  111.664002] CS:  0010 DS: 0000 ES: 0000 CR0: 000000008005003b                
[  111.664002] CR2: 0000000000600e70 CR3: 00000001171da000 CR4: 00000000000407e0
[  111.664002] DR0: 0000000000000000 DR1: 0000000000000000 DR2: 0000000000000000
[  111.664002] DR3: 0000000000000000 DR6: 00000000ffff0ff0 DR7: 0000000000000400
[  111.664002] Stack:                                                           
[  111.664002]  ffffffff81246578 ffffffff8183c770 000000000000006c ffffffff81830
[  111.664002]  ffffffff812465cd ffffffff817caa41 ffffffff817caa10 ffffffff81245
[  111.664002]  ffffffff8183c770 0000000000000000 0000000000000016 0000000000001
[  111.664002] Call Trace:                                                      
[  111.664002]  <IRQ>  [<ffffffff81246578>] ? wait_for_xmitr+0x3d/0x81          
[  111.664002]  [<ffffffff812465cd>] ? serial8250_console_putchar+0x11/0x1f     
[  111.664002]  [<ffffffff81242285>] ? uart_console_write+0x35/0x47             
[  111.664002]  [<ffffffff81246679>] ? serial8250_console_write+0x9e/0xe9       
[  111.664002]  [<ffffffff8102ea40>] ? call_console_drivers.constprop.22+0x9e/0e
[  111.664002]  [<ffffffff8102f8b0>] ? console_unlock+0x12e/0x2f9               
[  111.664002]  [<ffffffff8102ffa6>] ? vprintk_emit+0x3ab/0x3d6                 
[  111.664002]  [<ffffffff8135fc3f>] ? printk+0x4f/0x51                         
[  111.664002]  [<ffffffff8109273e>] ? rcu_check_callbacks+0x106/0x4bb          
[  111.664002]  [<ffffffff8106b90e>] ? tick_sched_do_timer+0x25/0x25            
[  111.664002]  [<ffffffff8103a92f>] ? update_process_times+0x31/0x5c           
[  111.664002]  [<ffffffff8106b672>] ? tick_sched_handle+0x3e/0x4a              
[  111.664002]  [<ffffffff8106b93e>] ? tick_sched_timer+0x30/0x4c               
[  111.664002]  [<ffffffff8104be54>] ? __run_hrtimer+0xa9/0x14e                 
[  111.664002]  [<ffffffff8104c461>] ? hrtimer_interrupt+0xbd/0x19e             
[  111.664002]  [<ffffffff810a9c9f>] ? perf_remove_from_context+0x89/0x89       
[  111.664002]  [<ffffffff8101bc60>] ? smp_apic_timer_interrupt+0x6d/0x7e       
[  111.664002]  [<ffffffff8136b00a>] ? apic_timer_interrupt+0x6a/0x70           
[  111.664002]  <EOI>  [<ffffffff810aa81a>] ? group_sched_out+0x62/0x62         
[  111.664002]  [<ffffffff810a9d03>] ? perf_event_disable+0x64/0x92             
[  111.664002]  [<ffffffff810a830a>] ? perf_event_for_each_child+0x62/0x83      
[  111.664002]  [<ffffffff810ab3da>] ? perf_event_task_disable+0x3f/0x6d        
[  111.664002]  [<ffffffff8104184c>] ? SyS_prctl+0x16a/0x32f                    
[  111.664002]  [<ffffffff8136a452>] ? system_call_fastpath+0x16/0x1b           
[  111.664002] Code: 3c bf e9 cb ff ff ff 65 8b 34 25 c4 b0 00 00 66 66 90 0f a 
[  111.676334] NMI backtrace for cpu 0                                          
[  111.676339] CPU: 0 PID: 0 Comm: swapper/0 Tainted: G        W    3.10.0-rc7 1
[  111.676341] Hardware name: AOpen   DE7000/nMCP7ALPx-DE R1.06 Oct.19.2012, BI2
[  111.676345] task: ffffffff81610400 ti: ffffffff81600000 task.ti: ffffffff8160
[  111.676348] RIP: 0010:[<ffffffff811b8d9b>]  [<ffffffff811b8d9b>] __const_ude0
[  111.676357] RSP: 0018:ffff88011fc03e60  EFLAGS: 00000046                     
[  111.676360] RAX: 0000000000000000 RBX: 0000000000002710 RCX: 0000000000000040
[  111.676363] RDX: 00000000009aa06c RSI: 0000000000000200 RDI: 0000000000418958
[  111.676365] RBP: ffff88011fc0d9a0 R08: 0000000000000000 R09: ffffffff817b87ec
[  111.676368] R10: 0000000000000000 R11: 0000000000000000 R12: ffffffff81637dc0
[  111.676371] R13: 0000000000000001 R14: ffffffff81600000 R15: ffffffff81637e80
[  111.676374] FS:  0000000000000000(0000) GS:ffff88011fc00000(0000) knlGS:00000
[  111.676377] CS:  0010 DS: 0000 ES: 0000 CR0: 000000008005003b                
[  111.676380] CR2: 00007fa834dd8a60 CR3: 00000001176de000 CR4: 00000000000407f0
[  111.676383] DR0: 0000000000000000 DR1: 0000000000000000 DR2: 0000000000000000
[  111.676385] DR3: 0000000000000000 DR6: 00000000ffff0ff0 DR7: 0000000000000400
[  111.676387] Stack:                                                           
[  111.676388]  ffffffff8101c67f ffffffff81637dc0 ffffffff81092a14 0000000000002
[  111.676394]  0000000000000083 0000000000000000 ffffffff81610400 0000000000000
[  111.676399]  0000000000000000 ffff88011fc0d370 ffffffff8106b90e ffff88011fc00
[  111.676405] Call Trace:                                                      
[  111.676407]  <IRQ>  [<ffffffff8101c67f>] ? arch_trigger_all_cpu_backtrace+0xf
[  111.676416]  [<ffffffff81092a14>] ? rcu_check_callbacks+0x3dc/0x4bb          
[  111.676424]  [<ffffffff8106b90e>] ? tick_sched_do_timer+0x25/0x25            
[  111.676429]  [<ffffffff8103a92f>] ? update_process_times+0x31/0x5c           
[  111.676436]  [<ffffffff8106b672>] ? tick_sched_handle+0x3e/0x4a              
[  111.676441]  [<ffffffff8106b93e>] ? tick_sched_timer+0x30/0x4c               
[  111.676447]  [<ffffffff8104be54>] ? __run_hrtimer+0xa9/0x14e                 
[  111.676452]  [<ffffffff8104c461>] ? hrtimer_interrupt+0xbd/0x19e             
[  111.676458]  [<ffffffff8101bc60>] ? smp_apic_timer_interrupt+0x6d/0x7e       
[  111.676464]  [<ffffffff8136b00a>] ? apic_timer_interrupt+0x6a/0x70           
[  111.676470]  <EOI>  [<ffffffff81008184>] ? default_idle+0x14/0x3a            
[  111.676480]  [<ffffffff810086ec>] ? arch_cpu_idle+0x5/0x14                   
[  111.676486]  [<ffffffff8106465d>] ? cpu_startup_entry+0x103/0x176            
[  111.676493]  [<ffffffff816aacaf>] ? start_kernel+0x3d6/0x3e1                 
[  111.676499]  [<ffffffff816aa6fb>] ? repair_env_string+0x54/0x54              
[  111.676505] Code: eb 0e 66 66 66 66 66 2e 0f 1f 84 00 00 00 00 00 48 ff c8 7 
[  111.664002]  { 1}  (t=5733 jiffies g=359 c=358 q=18)                         
[  111.664002] sending NMI to all CPUs:                                         
[  111.664002] NMI backtrace for cpu 1                                          
[  111.664002] CPU: 1 PID: 3049 Comm: out Tainted: G        W    3.10.0-rc7 #1  
[  111.664002] Hardware name: AOpen   DE7000/nMCP7ALPx-DE R1.06 Oct.19.2012, BI2
[  111.664002] task: ffff880117b62080 ti: ffff880117a6a000 task.ti: ffff880117a0
[  111.664002] RIP: 0010:[<ffffffff811b8ddf>]  [<ffffffff811b8ddf>] delay_tsc+06
[  111.664002] RSP: 0018:ffff88011fc83e60  EFLAGS: 00000847                     
[  111.664002] RAX: 00000000357ac791 RBX: 0000000000002710 RCX: 00000000357ac791
[  111.664002] RDX: 000000000000004f RSI: 0000000000000001 RDI: 0000000000265903
[  111.664002] RBP: ffff88011fc8d9a0 R08: 0000000000000000 R09: 0000000000000000
[  111.664002] R10: 00000000ffffffff R11: ffff880118676260 R12: ffffffff81637dc0
[  111.664002] R13: ffff88011fc8d370 R14: ffff880117a6a000 R15: 0000000000000001
[  111.664002] FS:  00007fa834fef700(0000) GS:ffff88011fc80000(0000) knlGS:00000
[  111.664002] CS:  0010 DS: 0000 ES: 0000 CR0: 000000008005003b                
[  111.664002] CR2: 0000000000600e70 CR3: 00000001171da000 CR4: 00000000000407e0
[  111.664002] DR0: 0000000000000000 DR1: 0000000000000000 DR2: 0000000000000000
[  111.664002] DR3: 0000000000000000 DR6: 00000000ffff0ff0 DR7: 0000000000000400
[  111.664002] Stack:                                                           
[  111.664002]  ffffffff8101c67f ffffffff81637dc0 ffffffff810927dc 0000000000002
[  111.664002]  0000000000353d39 0000000000000001 ffff880117b62080 0000000000000
[  111.664002]  0000000000000001 ffff88011fc8d370 ffffffff8106b90e ffff88011fc80
[  111.664002] Call Trace:                                                      
[  111.664002]  <IRQ>                                                           
[  111.664002]  [<ffffffff8101c67f>] ? arch_trigger_all_cpu_backtrace+0x76/0x7f 
[  111.664002]  [<ffffffff810927dc>] ? rcu_check_callbacks+0x1a4/0x4bb          
[  111.664002]  [<ffffffff8106b90e>] ? tick_sched_do_timer+0x25/0x25            
[  111.664002]  [<ffffffff8103a92f>] ? update_process_times+0x31/0x5c           
[  111.664002]  [<ffffffff8106b672>] ? tick_sched_handle+0x3e/0x4a              
[  111.664002]  [<ffffffff8106b93e>] ? tick_sched_timer+0x30/0x4c               
[  111.664002]  [<ffffffff8104be54>] ? __run_hrtimer+0xa9/0x14e                 
[  111.664002]  [<ffffffff8104c461>] ? hrtimer_interrupt+0xbd/0x19e             
[  111.664002]  [<ffffffff810a9c9f>] ? perf_remove_from_context+0x89/0x89       
[  111.664002]  [<ffffffff8101bc60>] ? smp_apic_timer_interrupt+0x6d/0x7e       
[  111.664002]  [<ffffffff8136b00a>] ? apic_timer_interrupt+0x6a/0x70           
[  111.664002]  <EOI>                                                           
[  111.664002]  [<ffffffff810aa81a>] ? group_sched_out+0x62/0x62                
[  111.664002]  [<ffffffff810a9d03>] ? perf_event_disable+0x64/0x92             
[  111.664002]  [<ffffffff810a830a>] ? perf_event_for_each_child+0x62/0x83      
[  111.664002]  [<ffffffff810ab3da>] ? perf_event_task_disable+0x3f/0x6d        
[  111.664002]  [<ffffffff8104184c>] ? SyS_prctl+0x16a/0x32f                    
[  111.664002]  [<ffffffff8136a452>] ? system_call_fastpath+0x16/0x1b           
[  111.664002] Code: ff c7 10 00 00 e9 d4 ff ff ff 48 8d 3c bf e9 cb ff ff ff 6 
[  113.636004] NMI backtrace for cpu 0                                          
[  113.636004] CPU: 0 PID: 0 Comm: swapper/0 Tainted: G        W    3.10.0-rc7 1
[  113.636004] Hardware name: AOpen   DE7000/nMCP7ALPx-DE R1.06 Oct.19.2012, BI2
[  113.636004] task: ffffffff81610400 ti: ffffffff81600000 task.ti: ffffffff8160
[  113.636004] RIP: 0010:[<ffffffff81008184>]  [<ffffffff81008184>] default_idla
[  113.636004] RSP: 0018:ffffffff81601f98  EFLAGS: 00000246                     
[  113.636004] RAX: 0000000000000000 RBX: ffffffff81601fd8 RCX: 00000000ffffffff
[  113.636004] RDX: 0100000000000000 RSI: 0000000000000000 RDI: ffff88011fc0d81c
[  113.636004] RBP: ffffffff81601fd8 R08: 0000000000000000 R09: 0000000000000000
[  113.636004] R10: 0000000000000001 R11: 0000000000000002 R12: ffffffff817332d0
[  113.636004] R13: ffff88011ffad540 R14: 0000000000000000 R15: 0000000000000000
[  113.636004] FS:  0000000000000000(0000) GS:ffff88011fc00000(0000) knlGS:00000
[  113.636004] CS:  0010 DS: 0000 ES: 0000 CR0: 000000008005003b                
[  113.636004] CR2: 00007fa834dd8a60 CR3: 0000000117988000 CR4: 00000000000407f0
[  113.636004] DR0: 0000000000000000 DR1: 0000000000000000 DR2: 0000000000000000
[  113.636004] DR3: 0000000000000000 DR6: 00000000ffff0ff0 DR7: 0000000000000400
[  113.636004] Stack:                                                           
[  113.636004]  ffffffff8106465d 516a4e0130c2f192 ffffffffffffffff ffffffff81720
[  113.636004]  ffffffff816aacaf ffffffff816aa6fb ffffffff817332d0 0000000000000
[  113.636004]  0000000000000000 0000000000000000 0000000000000000 0000000000000
[  113.636004] Call Trace:                                                      
[  113.636004]  [<ffffffff8106465d>] ? cpu_startup_entry+0x103/0x176            
[  113.636004]  [<ffffffff816aacaf>] ? start_kernel+0x3d6/0x3e1                 
[  113.636004]  [<ffffffff816aa6fb>] ? repair_env_string+0x54/0x54              
[  113.636004] Code: 08 00 48 8b 7b 08 44 89 e2 89 ee ff 13 48 83 c3 10 48 83 3 
[  140.248002] BUG: soft lockup - CPU#1 stuck for 23s! [out:3049]  
[  140.248002] Modules linked in: nfsd auth_rpcgss oid_registry nfs_acl nfs locn
[  140.248002] CPU: 1 PID: 3049 Comm: out Tainted: G        W    3.10.0-rc7 #1  
[  140.248002] Hardware name: AOpen   DE7000/nMCP7ALPx-DE R1.06 Oct.19.2012, BI2
[  140.248002] task: ffff880117b62080 ti: ffff880117a6a000 task.ti: ffff880117a0
[  140.248002] RIP: 0010:[<ffffffff810a9d03>]  [<ffffffff810a9d03>] perf_event_2
[  140.248002] RSP: 0018:ffff880117a6bed0  EFLAGS: 00000282                     
[  140.248002] RAX: 0000000000000001 RBX: 00007fa834b12259 RCX: 0000000000000001
[  140.248002] RDX: 000000000000fe0d RSI: ffffffff810aa81a RDI: ffff8801177ac7cc
[  140.248002] RBP: ffff8801177ac7cc R08: 0000000000000002 R09: 0000000000602000
[  140.248002] R10: 00007fa834b12259 R11: 0000000000000246 R12: 0000000000000002
[  140.248002] R13: ffff8801177ac7cc R14: 00007fa834b12259 R15: 0000000000000246
[  140.248002] FS:  00007fa834fef700(0000) GS:ffff88011fc80000(0000) knlGS:00000
[  140.248002] CS:  0010 DS: 0000 ES: 0000 CR0: 000000008005003b                
[  140.248002] CR2: 0000000000600e70 CR3: 00000001171da000 CR4: 00000000000407e0
[  140.248002] DR0: 0000000000000000 DR1: 0000000000000000 DR2: 0000000000000000
[  140.248002] DR3: 0000000000000000 DR6: 00000000ffff0ff0 DR7: 0000000000000400
[  140.248002] Stack:                                                           
[  140.248002]  ffff880118b96400 ffff880117abf400 ffff880118b965d0 ffffffff810aa
[  140.248002]  ffff880118b96400 ffff880117b62080 ffff880117b62708 0000000000002
[  140.248002]  ffffffff810ab3da 0000000000000000 0000000000000000 ffff880117b60
[  140.248002] Call Trace:                                                      
[  140.248002]  [<ffffffff810a830a>] ? perf_event_for_each_child+0x62/0x83      
[  140.248002]  [<ffffffff810ab3da>] ? perf_event_task_disable+0x3f/0x6d        
[  140.248002]  [<ffffffff8104184c>] ? SyS_prctl+0x16a/0x32f                    
[  140.248002]  [<ffffffff8136a452>] ? system_call_fastpath+0x16/0x1b           
[  140.248002] Code: 48 89 da 48 c7 c6 1a a8 0a 81 e8 cc e4 ff ff 85 c0 74 41 4 

*/
