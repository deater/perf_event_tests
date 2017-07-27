#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/utsname.h>

#include "test_utils.h"

int test_quiet(void) {

	if (getenv("TESTS_QUIET")!=NULL) return 1;
	return 0;
}

#define RED    "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN  "\033[1;32m"
#define BLUE   "\033[1;34m"
#define WHITE  "\033[1m"
#define NORMAL "\033[0m"


void test_pass(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sPASSED%s\n", 60, string,
			GREEN, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s PASSED\n", 60, string);
	}
}

void test_warn(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sWARNING%s\n", 60, string,
			YELLOW, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s WARNING\n", 60, string);
	}
	exit(1);
}

void test_needtest(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sNEED TESTCASE%s\n", 60, string,
			YELLOW, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s NEED TESTCASE\n", 60, string);
	}
	exit(1);
}

void test_caution(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sUSE CAUTION%s\n", 60, string,
			YELLOW, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s USE CAUTION\n", 60, string);
	}
	exit(1);
}

void test_known_issue(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sKNOWN ISSUE%s\n", 60, string,
			YELLOW, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s KNOWN ISSUE\n", 60, string);
	}
	exit(1);
}

void test_skip(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sSKIPPED%s\n", 60, string, WHITE,
			NORMAL );
	}
	else {
		fprintf( stdout, "%-*s SKIPPED\n", 60, string);
	}
	exit(1);
}

void test_fail(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sFAILED%s\n", 60, string,
			RED, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s FAILED\n", 60, string);
	}

	exit(1);
}

void test_known_kernel_bug(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sKNOWN KERNEL BUG%s\n",
			60, string, RED, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s KNOWN KERNEL BUG\n", 60, string);
	}

	exit(1);
}

void test_fail_kernel(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sFAILED (KERNEL TOO OLD)%s\n",
			60, string, RED, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s FAILED (KERNEL TOO OLD)\n", 60, string);
	}

	exit(1);
}

void test_kernel_fail(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sKERNEL FAILED%s\n", 60, string,
			RED, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s KERNEL FAILED\n", 60, string);
	}

	exit(1);
}

void test_kernel_pass(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sKERNEL OK%s\n", 60, string,
			GREEN, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s KERNEL OK\n", 60, string);
	}

	exit(1);
}

void test_unexplained(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sUNEXPLAINED%s\n", 60, string,
			BLUE, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s UNEXPLAINED\n", 60, string);
	}

	exit(1);
}

void test_unimplemented(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sUNIMPLEMENTED%s\n", 60, string,
			BLUE, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s UNIMPLEMENTED\n", 60, string);
	}

	exit(1);
}

void test_yellow_no(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sNO%s\n", 60, string,
			YELLOW, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s NO\n", 60, string);
	}
	exit(1);
}

void test_green_yes(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sYES%s\n", 60, string,
			GREEN, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s YES\n", 60, string);
	}

	exit(1);
}

void test_yellow_yes(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sYES%s\n", 60, string,
			YELLOW, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s YES\n", 60, string);
	}

	exit(1);
}

void test_yellow_old_behavior(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sOLD BEHAVIOR%s\n", 60, string,
			YELLOW, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s OLD BEHAVIOR\n", 60, string);
	}

	exit(1);
}

void test_green_new_behavior(char *string) {

	if (isatty(fileno(stdout))) {
		fprintf( stdout, "%-*s %sNEW BEHAVIOR%s\n", 60, string,
			GREEN, NORMAL );
	}
	else {
		fprintf( stdout, "%-*s NEW BEHAVIOR\n", 60, string);
	}

	exit(1);
}





double display_error(long long average,
		     long long high,
		     long long low,
		     long long expected,
                     int quiet) {

	double error;

	error=(((double)average-expected)/expected)*100.0;

	if (!quiet) {
		printf("   Expected: %lld\n", expected);
		printf("   High: %lld   Low:  %lld   Average:  %lld\n",
			high,low,average);

		printf("   ( note, a small value above %lld "
			"may be expected due\n", expected);
		printf("     to overhead and interrupt noise, "
			"among other reasons)\n");

		printf("   Average Error = %.2f%%\n",error);
	}

	return error;

}

int get_paranoid_setting(void) {

	FILE *fff;
	int paranoid;

	fff=fopen("/proc/sys/kernel/perf_event_paranoid","r");
	if (fff==NULL) {
		return -1;
	}

	fscanf(fff,"%d",&paranoid);

	fclose(fff);

	return paranoid;

}


int check_paranoid_setting(int desired, int quiet) {


	int paranoid;

	paranoid=get_paranoid_setting();

	if (paranoid>desired) {
		if (!quiet) {
			fprintf(stderr,"Value of %d in /proc/sys/kernel/perf_event_paranoid too high for this test to run!\n",
				paranoid);
		}
		return -1;
	}

	return 0;

}

int check_linux_version_newer(int major, int minor, int subminor) {

	int newer=0;
	int check_version,current_version;
	struct utsname uname_buffer;
	char *ptr;
	int current_major=0,current_minor=0,current_sub=0;

	/* Get the kernel info */
	uname(&uname_buffer);

	ptr=strtok(uname_buffer.release,".");
	if (ptr!=NULL) current_major=atoi(ptr);

	ptr=strtok(NULL,".");
	if (ptr!=NULL) current_minor=atoi(ptr);

	ptr=strtok(NULL,".");
	if (ptr!=NULL) current_sub=atoi(ptr);


	check_version = ( ((major&0xff)<<24) | ((minor&0xff)<<16) |
			((subminor&0xff) << 8));

	current_version = ( ((current_major&0xff)<<24) | ((current_minor&0xff)<<16) |
			((current_sub&0xff) << 8));

	if (current_version>=check_version) newer=1;

	return newer;
}
