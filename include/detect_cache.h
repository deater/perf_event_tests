#define CACHE_L1 0
#define CACHE_L2 1
#define CACHE_L3 2
#define CACHE_L4 3
#define CACHE_L5 4
#define MAX_CACHE_LEVEL 5

#define CACHE_I 0
#define CACHE_D 1
#define CACHE_U 2
#define CACHE_T 3
#define MAX_CACHE_TYPE 4

#define CACHE_UNKNOWN 0

struct cache_info_t {
  int exists;
  int level;
  int type;
  int wpolicy;
  int replace;
  int size;
  int lines;
  int ways;
  int linesize;
};

extern struct cache_info_t cache_info[MAX_CACHE_LEVEL][MAX_CACHE_TYPE];

int gather_cache_info(int quiet, char *test_string);
int cache_get_max_levels(int quiet, char *test_string);
void print_cache_info(int quiet, struct cache_info_t *cache_entry);
