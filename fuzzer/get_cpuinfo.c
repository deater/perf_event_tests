#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

void get_cpuinfo_x86(char *cpuinfo) {

	FILE *fff;
	char temp_string[BUFSIZ],temp_string2[BUFSIZ];
	char vendor[BUFSIZ];
	char *result;
	int family,model,stepping;

	fff=fopen("/proc/cpuinfo","r");
	if (fff==NULL) {
		strcpy(cpuinfo,"UNKNOWN");
		return;
	}

	strcpy(vendor,"UNKNOWN ");

	while(1) {
		result=fgets(temp_string,BUFSIZ,fff);
		if (result==NULL) break;

		if (!strncmp(temp_string,"vendor_id",9)) {
			sscanf(temp_string,"%*s%*s%s",temp_string2);

			if (!strncmp(temp_string2,"GenuineIntel",12)) {
				strcpy(vendor,"Intel");
			} else if (!strncmp(temp_string2,"AuthenticAMD",12)) {
				strcpy(vendor,"AMD");
			}
		}

		if (!strncmp(temp_string,"cpu family",10)) {
			sscanf(temp_string,"%*s%*s%*s%d",&family);
		}

 		if (!strncmp(temp_string,"model",5)) {
			sscanf(temp_string,"%*s%s",temp_string2);
			if (temp_string2[0]==':') {
				sscanf(temp_string,"%*s%*s%d",&model);
			}
		}

		if (!strncmp(temp_string,"stepping",8)) {
			sscanf(temp_string,"%*s%*s%d",&stepping);
		}

	}
	fclose(fff);

	sprintf(cpuinfo,"%s %d/%d/%d\n",vendor,family,model,stepping);

}

static void get_cpuinfo_arm(char *cpuinfo) {

	FILE *fff;
	char *result;
	char temp_string[BUFSIZ];
	int architecture,implementer,variant,part,revision;

	fff=fopen("/proc/cpuinfo","r");
	if (fff==NULL) {
		strcpy(cpuinfo,"UNKNOWN");
		return;
	}

	while(1) {
		result=fgets(temp_string,BUFSIZ,fff);
		if (result==NULL) break;

		if (!strncmp(temp_string,"CPU implementer",15)) {
			sscanf(temp_string,"%*s%*s%*s %x",&implementer);
		}
		if (!strncmp(temp_string,"CPU architecture",16)) {
			sscanf(temp_string,"%*s%*s %x",&architecture);
		}
		if (!strncmp(temp_string,"CPU variant",11)) {
			sscanf(temp_string,"%*s%*s%*s %x",&variant);
		}
		if (!strncmp(temp_string,"CPU revision",12)) {
			sscanf(temp_string,"%*s%*s%*s %x",&revision);
		}
		if (!strncmp(temp_string,"CPU part",8)) {
			sscanf(temp_string,"%*s%*s%*s %x",&part);
		}

	}
	fclose(fff);

	sprintf(cpuinfo,"ARMv%d %x %x %x %x\n",
		architecture,implementer,variant,part,revision);

}

void get_cpuinfo(char *cpuinfo) {

	struct utsname uname_buf;

	uname(&uname_buf);

	if (strstr(uname_buf.machine,"86")) {
		return get_cpuinfo_x86(cpuinfo);
	}
	if (strstr(uname_buf.machine,"arm")) {
		return get_cpuinfo_arm(cpuinfo);
	}

	sprintf(cpuinfo,"%s UNKNOWN\n",uname_buf.machine);

}

