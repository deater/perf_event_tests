#include <stdio.h>

#include "papi_cache_info.h"

#include "papi.h"

#include "test_utils.h"


static const PAPI_hw_info_t *hw_info=NULL;

struct cache_info_t {
       int wpolicy;
       int replace;
       int size;
       int entries;
       int ways;
       int linesize;
};

static struct cache_info_t cache_info[MAX_CACHE];


static void check_if_cache_info_available(int quiet, char *test_string) {

   int cache_type,i,j;

   hw_info=PAPI_get_hardware_info();
   if (hw_info==NULL) {
      if (!quiet) printf("hardware cache info not available\n");
      test_fail(test_string);
   }

   for(i=0;i<hw_info->mem_hierarchy.levels;i++) {
      for(j=0;j<2;j++) {
         cache_type=PAPI_MH_CACHE_TYPE(
                    hw_info->mem_hierarchy.level[i].cache[j].type);
         if (cache_type!=PAPI_MH_TYPE_EMPTY) {
	    if (i==0) {
	       if (cache_type==PAPI_MH_TYPE_DATA) {
	          cache_info[L1D_CACHE].size=hw_info->mem_hierarchy.level[i].cache[j].size;
               	  cache_info[L1D_CACHE].linesize=hw_info->mem_hierarchy.level[i].cache[j].line_size;
	          cache_info[L1D_CACHE].ways=hw_info->mem_hierarchy.level[i].cache[j].associativity;
	          cache_info[L1D_CACHE].entries=cache_info[L1D_CACHE].size/
                              cache_info[L1D_CACHE].linesize;
                  cache_info[L1D_CACHE].wpolicy=PAPI_MH_CACHE_WRITE_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
	          cache_info[L1D_CACHE].replace=PAPI_MH_CACHE_REPLACEMENT_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
               }
               else if (cache_type==PAPI_MH_TYPE_INST) {
	          cache_info[L1I_CACHE].size=hw_info->mem_hierarchy.level[i].cache[j].size;
               	  cache_info[L1I_CACHE].linesize=hw_info->mem_hierarchy.level[i].cache[j].line_size;
	          cache_info[L1I_CACHE].ways=hw_info->mem_hierarchy.level[i].cache[j].associativity;
	          cache_info[L1I_CACHE].entries=cache_info[L1I_CACHE].size/
                              cache_info[L1I_CACHE].linesize;
                  cache_info[L1I_CACHE].wpolicy=PAPI_MH_CACHE_WRITE_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
	          cache_info[L1I_CACHE].replace=PAPI_MH_CACHE_REPLACEMENT_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
               }
	    }
            else if (i==1) {
	       cache_info[L2_CACHE].size=hw_info->mem_hierarchy.level[i].cache[j].size;
               cache_info[L2_CACHE].linesize=hw_info->mem_hierarchy.level[i].cache[j].line_size;
	       cache_info[L2_CACHE].ways=hw_info->mem_hierarchy.level[i].cache[j].associativity;
	       cache_info[L2_CACHE].entries=cache_info[L2_CACHE].size/
                              cache_info[L2_CACHE].linesize;
               cache_info[L2_CACHE].wpolicy=PAPI_MH_CACHE_WRITE_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
	       cache_info[L2_CACHE].replace=PAPI_MH_CACHE_REPLACEMENT_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
            }
            else if (i==2) {
	       cache_info[L3_CACHE].size=hw_info->mem_hierarchy.level[i].cache[j].size;
               cache_info[L3_CACHE].linesize=hw_info->mem_hierarchy.level[i].cache[j].line_size;
	       cache_info[L3_CACHE].ways=hw_info->mem_hierarchy.level[i].cache[j].associativity;
	       cache_info[L3_CACHE].entries=cache_info[L3_CACHE].size/
                              cache_info[L3_CACHE].linesize;
               cache_info[L3_CACHE].wpolicy=PAPI_MH_CACHE_WRITE_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
	       cache_info[L3_CACHE].replace=PAPI_MH_CACHE_REPLACEMENT_POLICY(hw_info->mem_hierarchy.level[i].cache[j].type);
            }
	 }
      }
   }
}

long long get_cachesize(int type, int quiet, char *test_string) {

   check_if_cache_info_available(quiet,test_string);

   if (type>MAX_CACHE) {
      if (!quiet) printf("Errror cache %d not available\n",type);
      test_fail(test_string);
   }

   return cache_info[type].size;
}


long long get_entries(int type, int quiet, char *test_string) {

   check_if_cache_info_available(quiet,test_string);

   if (type>MAX_CACHE) {
      if (!quiet) printf("Errror cache %d not available\n",type);
      test_fail(test_string);
   }

   return cache_info[type].entries;
}


long long get_linesize(int type,int quiet,char *test_string) {

   check_if_cache_info_available(quiet,test_string);

   if (type>MAX_CACHE) {
      if (!quiet) printf("Errror cache %d not available\n",type);
      test_fail(test_string);
   }

   return cache_info[type].linesize;
}
