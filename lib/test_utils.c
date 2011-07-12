#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

void test_needtest(char *string) {

  if (isatty(fileno(stdout))) {
     fprintf( stdout, "%-*s %sNEED TESTCASE%s\n", 60, string, 
	   YELLOW, NORMAL );
  }
  else {
    fprintf( stdout, "%-*s NEED TESTCASE\n", 60, string);
  }
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

void test_skip(char *string) {

  if (isatty(fileno(stdout))) {
     fprintf( stdout, "%-*s %sSKIPPED%s\n", 60, string, 
	   WHITE, NORMAL );
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

      printf("   ( note, a small value above %lld may be expected due\n",
	  expected);
      printf("     to overhead and interrupt noise, among other reasons)\n");

      printf("   Average Error = %.2f%%\n",error);
   }

   return error;

}
