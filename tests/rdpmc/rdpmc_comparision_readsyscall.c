/* Copyright (c) 2015, Linaro Limited
 * All rights reserved.
 *
 * SPDX-License-Identifier:	BSD-3-Clause
 *
 * This program is used to read perf cycle counter using
 * perf_event_open/read syscall
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

static int fddev = -1;
static int overflow;

static void sigio_handler(int signo, siginfo_t *info, void *uc)
{
	overflow = overflow + 1;
}

static int  counter_init(unsigned int counter, unsigned long sample)
{
	static struct perf_event_attr attr;
	static struct sigaction sigio;

	attr.type = PERF_TYPE_HARDWARE;

	if (counter < 0 || counter > PERF_COUNT_HW_MAX)
		return -1;

	/* setup SIGIO signal handler */
	memset(&sigio, 0, sizeof(struct sigaction));
	sigio.sa_sigaction = (void *)sigio_handler;
	sigio.sa_flags = SA_SIGINFO;

	if (sigaction(SIGIO, &sigio, NULL) == -1)
		printf("Error: SIGIO signal handler %s\n", strerror(errno));

	attr.sample_period = sample;
	attr.sample_type = PERF_SAMPLE_IP;
	attr.wakeup_events = 1;
	attr.config = counter;
	fddev = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
	if (fddev == -1) {
		fprintf(stderr,
			"Error opening perf_event_open for counter %llx\n"
			, attr.config);
		return -1;
	}

	fcntl(fddev, F_SETFL, O_RDWR|O_NONBLOCK|O_ASYNC);
	fcntl(fddev, F_SETSIG, SIGIO);
	fcntl(fddev, F_SETOWN, getpid());
	return 0;
}

static inline long long cpucycles(void)
{
	long long result = 0;

	if (read(fddev, &result, sizeof(result)) < sizeof(result))
		return 0;

	return result;
}

/* loop body to keep things interested. Make sure it gets inlined. */
static inline int
loop(int *__restrict__ a, int *__restrict__ b, int n)
{
	unsigned sum = 0;
	int i = 0;

	for (i = 0; i < n; ++i)
		if (a[i] > b[i])
			sum += a[i] + 5;

	return sum;
}

void print_usage(char *argv)
{
	printf("Usage: %s -c perf_hw_counter -n length\n"
	"%s - read perf hardware counters from userspace\n\n"
	"-c\t perf_hw_counter is enum value of hw counter for platform\n"
	"Refer to 'enum perf_hw_id'[file <linux/perf_event.h>] for hw\n"
	"counter enum value in range of 0 to PERF_COUNT_HW_MAX\n"
	"-n\t Length size array will implement busyloop\n"
	"-s\t Sample period [> 1000000 samples]\n"
	"e.g.\n"
	"\t%s -c 0 -n 64 -s 1000000\n", argv, argv, argv);
}

int main(int ac, char *argv[])
{
	long long time_start = 0;
	long long time_end   = 0;
	int option = 0;
	int len = -1, cnt = -1;
	int *a  = NULL;
	int *b  = NULL;
	int i, pre_loop_res, post_loop_res;
	unsigned long s_period = 1000000;
#ifdef MAX_DELAY
	unsigned long tmp_value = 0;
	unsigned long max_value = 0;
#endif

	while ((option = getopt(ac, argv, "c:n:s:")) != -1) {
		switch (option) {
		case 'c':
			cnt = atoi(optarg);
			break;
		case 'n':
			len = atoi(optarg);
			break;
		case 's':
			s_period = atoi(optarg);
			break;
		default:
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	if (len == -1 || cnt == -1) {
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	if (counter_init(cnt, s_period) < 0) {
		printf("Error: Counter Invalid\n");
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	a = malloc(len*sizeof(*a));
	b = malloc(len*sizeof(*b));
	ioctl(fddev, PERF_EVENT_IOC_RESET, 0);
	ioctl(fddev, PERF_EVENT_IOC_ENABLE, 0);

	for (i = 0; i < len; ++i) {
		a[i] = i+128;
		b[i] = i+64;
	}

	time_start = time_end = 0;
	/* --------------------Critical section-------------- */
	time_start = cpucycles();
	for (i = 0; i < 1000; i++) {
		loop(a, b, len);
#ifndef MAX_DELAY
		cpucycles();
#else
		tmp_value = cpucycles();
		if (max_value < tmp_value)
			max_value = tmp_value;
#endif
	}

	time_end = cpucycles();
	/* ---------------------------------- */
	pre_loop_res = abs(time_end-time_start)/1000;
	time_start = time_end = 0;
	ioctl(fddev, PERF_EVENT_IOC_RESET, 0);
	/* --------------------Critical section-------------- */
	time_start = cpucycles();
	for (i = 0; i < 1000; i++)
		loop(a, b, len);

	time_end = cpucycles();
	/* ---------------------------------- */
	ioctl(fddev, PERF_EVENT_IOC_DISABLE, 0);
	post_loop_res = abs(time_end-time_start)/1000;
#ifndef MAX_DELAY
	printf("\t%s Counter[%d] avg delay[cpucycles]=%d overflows=%d\n",
		argv[0], cnt, abs(post_loop_res - pre_loop_res), overflow);
#else
	printf("\t%s Counter[%d] max delay[cpucycles]=%ld avg delay[cpucycles]"
	"=%d Overflows=%d\n",
	argv[0], cnt, max_value, abs(post_loop_res - pre_loop_res), overflow);
#endif
	free(a);
	free(b);
	close(fddev);
	return 0;
}
