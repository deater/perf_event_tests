#include <stdio.h>

int main(int argc, char **argv) {

	char string[BUFSIZ];
	char *result;
	FILE *fff;

	if (argc<3) {
		fprintf(stderr,"\nUsage: %s logfile char\n\n",argv[0]);
		return -1;
	}

	fff=fopen(argv[1],"r");
	if (fff==NULL) {
		fprintf(stderr,"Error opening file %s\n",argv[1]);
		return -1;
	}

	while(1) {
		result=fgets(string,BUFSIZ,fff);
		if (result==NULL) break;
		if (string[0]!=argv[2][0]) printf("%s",string);
	}

	fclose(fff);

	return 0;

}
