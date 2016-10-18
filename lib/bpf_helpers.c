#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/syscall.h>
#include <unistd.h>
#include <asm/unistd.h>


#include "bpf.h"

#ifndef __NR_bpf

#if defined(__i386__)
#define __NR_bpf		357
#elif defined(__x86_64__)
#define __NR_bpf		321
#elif defined(__powerpc__)
#define __NR_bpf		361
#elif defined(__arm__) || defined(__aarch64__)
#define __NR_bpf		386
#elif defined(__sparc__)
#define __NR_bpf		349
#endif

#endif


long sys_bpf(int cmd, union bpf_attr *attr, unsigned long size) {

	return syscall(__NR_bpf, cmd, attr, size);
}

