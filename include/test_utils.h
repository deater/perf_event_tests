int test_quiet(void);
void test_pass(char *string);
void test_needtest(char *string);
void test_caution(char *string);
void test_fail(char *string);
void test_kernel_fail(char *string);
void test_kernel_pass(char *string);
void test_skip(char *string);
void test_unexplained(char *string);
void test_unimplemented(char *string);
double display_error(long long average,
		     long long high,
		     long long low,
		     long long expected,
                     int quiet);

#define ALL_OK              0
#define CODE_UNIMPLEMENTED -1
#define ERROR_RESULT       -2
