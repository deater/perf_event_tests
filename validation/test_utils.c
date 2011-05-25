#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int test_quiet(void) {

  if (getenv("TESTS_QUIET")!=NULL) return 1;
  return 0;

}

#define RED    "\033[1;31m"
#define YELLOW "\033[1;33m"
#define GREEN  "\033[1;32m"
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
