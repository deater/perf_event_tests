int test_quiet(void);
void test_pass(char *string);
void test_warn(char *string);
void test_needtest(char *string);
void test_caution(char *string);
void test_known_issue(char *string);
void test_known_kernel_bug(char *string);
void test_fail(char *string);
void test_fail_kernel(char *string);
void test_kernel_fail(char *string);
void test_kernel_pass(char *string);
void test_skip(char *string);
void test_unexplained(char *string);
void test_unimplemented(char *string);
void test_green_yes(char *string);
void test_yellow_no(char *string);
void test_yellow_yes(char *string);
void test_yellow_old_behavior(char *string);
void test_green_new_behavior(char *string);

int check_paranoid_setting(int desired, int quiet);
int get_paranoid_setting(void);

int check_linux_version_newer(int major, int minor, int subminor);

double display_error(long long average,
		     long long high,
		     long long low,
		     long long expected,
                     int quiet);

#define ALL_OK              0
#define CODE_UNIMPLEMENTED -1
#define ERROR_RESULT       -2
