/* This tests Uncore Evets on Nehalem+ CPUs         */
/* These weren't supported until at least Linxu 3.6 */

/* by Vince Weaver, vincent.weaver@maine.edu        */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <dirent.h>

#include <errno.h>

#include "perf_event.h"
#include "perf_helpers.h"
#include "test_utils.h"
#include "matrix_multiply.h"

char test_string[]="Testing uncore events...";

int main(int argc, char **argv) {
   
   int ret,quiet;
  //int cpu;
   struct perf_event_attr pe;
   int fd1;
   long long counts[2];

   int pmu_type,cbox0_type=-1;

   DIR *sys_devices;
   struct dirent *dir;
   FILE *fff;

   char filename[BUFSIZ];
   
   quiet=test_quiet();
   
   //   cpu=detect_processor();
   
   sys_devices=opendir("/sys/devices");
   if (sys_devices==NULL) {
      fprintf(stderr,"Couldn't open /sys/devices\n");
      test_skip(test_string);
   }

   printf("Looking for PMUs:\n");
   while (1) {
     dir=readdir(sys_devices);
     if (dir==NULL) break;

     if (!strncmp(dir->d_name,".",1)) continue;
     if (!strncmp(dir->d_name,"..",2)) continue;

     sprintf(filename,"/sys/devices/%s/type",dir->d_name);
     fff=fopen(filename,"r");
     if (fff==NULL) continue;

     fscanf(fff,"%d",&pmu_type);
     
     printf("\tFound %s = %d\n",dir->d_name, pmu_type);

     fclose(fff);
     if (!strncmp(dir->d_name,"uncore_cbox_0",13)) cbox0_type=pmu_type;

   }

   closedir(sys_devices);

   if (cbox0_type==-1) {
      fprintf(stderr,"Could not find CBOX0\n");
      test_skip(test_string);
   }

   printf("Found cbox0 at type %d\n",cbox0_type);

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=cbox0_type;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=0; /* UNC_C_CLOCKTICKS */
   pe.disabled=1;
   pe.exclude_kernel=1;
   pe.exclude_hv=1;

   fd1=perf_event_open(&pe,0,-1,-1,0);
   if (fd1<0) {
     fprintf(stderr,"Error opening leader %llx, %d : %s\n",
	     pe.config,errno,strerror(errno));
      test_fail(test_string);
   }
      
   ioctl(fd1, PERF_EVENT_IOC_RESET, 0);   

   ret=ioctl(fd1, PERF_EVENT_IOC_ENABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error enabling events\n");
      test_fail(test_string);
   }
   
   naive_matrix_multiply(quiet);

   ret=ioctl(fd1, PERF_EVENT_IOC_DISABLE,0);
   if (ret<0) {
      if (!quiet) fprintf(stderr,"Error disabling events\n");
      test_fail(test_string);
   }

   ret=read(fd1,&counts[0],sizeof(long long));
   if (ret!=sizeof(long long)) {
      if (!quiet) fprintf(stderr,"Unepxected result from read fd1: %d\n",ret);
      test_fail(test_string);      
   }
   
   if (!quiet) {
      printf("Retired instructions: %lld\n",counts[0]);
   }
   if (counts[0]<1) {
      fprintf(stderr,"Retired instruction count too low\n");
      test_fail(test_string);
   }
   
   test_pass(test_string);

   return 0;
}


