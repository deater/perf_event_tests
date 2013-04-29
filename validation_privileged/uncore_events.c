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
   struct perf_event_attr pe;
   int fd1;
   long long counts[2];

   int pmu_type,uncore_type=-1;

   DIR *sys_devices;
   struct dirent *dir;
   FILE *fff;

   char filename[BUFSIZ];
   
   char uncore_box_name[BUFSIZ];
   char uncore_event_name[BUFSIZ];
   int uncore_event;

   int processor_type;

   quiet=test_quiet();
   
   sys_devices=opendir("/sys/bus/event_source/devices/");
   if (sys_devices==NULL) {
      fprintf(stderr,"Couldn't open /sys/bus_event/source/devices/\n");
      test_skip(test_string);
   }

   processor_type=detect_processor();

   switch(processor_type) {
      case PROCESSOR_SANDYBRIDGE_EP:
           strcpy(uncore_box_name,"uncore_imc_0");
           strcpy(uncore_event_name,"uncore_imc0::UNC_M_CLOCKTICKS");
           uncore_event=0xff;
           break;
      case PROCESSOR_SANDYBRIDGE:
      case PROCESSOR_IVYBRIDGE:
           strcpy(uncore_box_name,"uncore_cbox_0");
           strcpy(uncore_event_name,"snb_unc_cbo0::UNC_CLOCKTICKS");
           uncore_event=0xff;
           break;
      default:
	   if (!quiet) fprintf(stderr,"Unsupported processor\n");
	   test_skip(test_string);
	   break;
   }

   //printf("Looking for PMUs:\n");
   while (1) {
     dir=readdir(sys_devices);
     if (dir==NULL) break;

     if (!strncmp(dir->d_name,".",1)) continue;
     if (!strncmp(dir->d_name,"..",2)) continue;

     sprintf(filename,"/sys/devices/%s/type",dir->d_name);
     fff=fopen(filename,"r");
     if (fff==NULL) continue;

     fscanf(fff,"%d",&pmu_type);
     
     //printf("\tFound %s = %d\n",dir->d_name, pmu_type);

     fclose(fff);
     if (!strcmp(dir->d_name,uncore_box_name)) uncore_type=pmu_type;

   }

   closedir(sys_devices);

   if (uncore_type==-1) {
      if (!quiet) fprintf(stderr,"Could not find %s\n",uncore_box_name);
      test_skip(test_string);
   }

   if (!quiet) {
      printf("Found %s at type %d\n",uncore_box_name,uncore_type);
   }

   memset(&pe,0,sizeof(struct perf_event_attr));
   pe.type=uncore_type;
   pe.size=sizeof(struct perf_event_attr);
   pe.config=uncore_event;
   pe.disabled=1;
   pe.exclude_user=0;
   pe.exclude_kernel=0;
   pe.exclude_hv=0;

   fd1=perf_event_open(&pe,-1,0,-1,0);
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
      printf("%s %lld\n",uncore_event_name,counts[0]);
   }
   if (counts[0]<1) {
      fprintf(stderr,"Retired instruction count too low\n");
      test_fail(test_string);
   }
   
   test_pass(test_string);

   return 0;
}
