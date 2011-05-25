#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <asm/unistd.h>

#include "perf_event.h"
#include "perf_helpers.h"

int perf_event_open(struct perf_event_attr *hw_event_uptr,
		    pid_t pid, int cpu, int group_fd, unsigned long flags) {
   
   return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		  group_fd, flags);
}

#define VENDOR_UNKNOWN -1
#define VENDOR_INTEL    1
#define VENDOR_AMD      2

int detect_processor(void) {

   FILE *fff;
   int vendor=VENDOR_UNKNOWN,cpu_family=0,model=0;
   char string[BUFSIZ];
   
   /* FIXME: x86 only for now */
   
   fff=fopen("/proc/cpuinfo","r");
   if (fff==NULL) {
      fprintf(stderr,"ERROR!  Can't open /proc/cpuinfo\n");
      return PROCESSOR_UNKNOWN;
   }
   
   while(1) {
      if (fgets(string,BUFSIZ,fff)==NULL) break;
      
      /* vendor */
      if (strstr(string,"vendor_id")) {
         if (strstr(string,"GenuineIntel")) {	 
	    vendor=VENDOR_INTEL;
	 }
	 if (strstr(string,"AuthenticAMD")) {
	    vendor=VENDOR_AMD;
	 }
      }
      
      /* family */
      if (strstr(string,"cpu family")) {
	 sscanf(string,"%*s %*s %*s %d",&cpu_family); 
      }
      
      /* model */
      if ((strstr(string,"model")) && (!strstr(string,"model name")) ) {
	 sscanf(string,"%*s %*s %d",&model);
      }
   }
       
   fclose(fff);

   if (vendor==VENDOR_AMD) {
      if (cpu_family==0x6) {
	 return PROCESSOR_K7;
      }
      if (cpu_family==0xf) {
	 return PROCESSOR_K8;
      }
      if (cpu_family==0x10) {
	 return PROCESSOR_AMD_FAM10H;
      }
      if (cpu_family==0x11) {
	 return PROCESSOR_AMD_FAM11H;
      }
      if (cpu_family==0x14) {
	 return PROCESSOR_AMD_FAM14H;
      }
      if (cpu_family==0x15) {
	 return PROCESSOR_AMD_FAM15H;
      }
   }
   
   if (vendor==VENDOR_INTEL) {
    
      if (cpu_family==6) {
	 switch(model) {
	  case 1:
	  case 3:
	    return PROCESSOR_PENTIUM_PRO;
	  case 5:
	  case 6:
	    return PROCESSOR_PENTIUM_II;
	  case 7:
	  case 8:
	  case 10:
	  case 11:
	    return PROCESSOR_PENTIUM_III;
	  case 9:
	  case 13:
	    return PROCESSOR_PENTIUM_M;
	  case 14:
	    return PROCESSOR_COREDUO;
	  case 15:
	  case 22:
	  case 23:
	  case 29:
	    return PROCESSOR_CORE2;	    
	  case 26:
	  case 30:
	  case 31:
	    return PROCESSOR_NEHALEM;
	  case 28:
	  case 38:
	    return PROCESSOR_ATOM;
	  case 37:
	  case 44:
	    return PROCESSOR_WESTMERE;
	  case 42:
	  case 45:
	    return PROCESSOR_SANDYBRIDGE;
	  case 46:
	    return PROCESSOR_NEHALEM_EX;
	    
	 }
      }
      if (cpu_family==15) {
	 return PROCESSOR_PENTIUM_4;
      }
   }
   
   return PROCESSOR_UNKNOWN;
}
