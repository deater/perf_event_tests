#include <stdio.h>

/* FIXME -- this is x86 specific for now */
void get_cpuinfo(char *cpuinfo) {

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

