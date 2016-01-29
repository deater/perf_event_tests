#if 0
 Performance counter stats for './papi_tot_ins':

   100,000,594,516      instructions:u                                              

      12.995436061 seconds time elapsed
#endif

#include <stdio.h>

#define RUNS 30

static unsigned long long remove_commas(char *string) {

	unsigned long long result=0;
	int i=0;

	while(1) {
		if ((string[i]==0) || (string[i]==' ')) break;

		if (string[i]!=',') {
			result*=10;
			result+=(string[i]-'0');
		}

		i++;
	}

	return result;
}

int main(int argc, char **argv) {

	int i;
	char filename[BUFSIZ];
	char string[BUFSIZ],insn_string[BUFSIZ];
	FILE *fff;
	long long instructions=0;
	double time;
	unsigned long long instructions_max=0,instructions_min=0x7fffffffffffffffULL;
	unsigned long long instructions_total=0,instructions_average=0;
	double time_min=10000.0,time_max=0.0,time_total=0,time_average=0;

	for(i=1;i<=RUNS;i++ ){
		sprintf(filename,"out.%d",i);
		fff=fopen(filename,"r");
		if (fff==NULL) {
			fprintf(stderr,"Couldn't open %s\n",filename);
		}
		else {
			fgets(string,BUFSIZ,fff);
			fgets(string,BUFSIZ,fff);
			fgets(string,BUFSIZ,fff);
			fgets(string,BUFSIZ,fff);
			/* instructions */

			sscanf(string,"%s",insn_string);
			instructions=remove_commas(insn_string);

			fgets(string,BUFSIZ,fff);
			fgets(string,BUFSIZ,fff);
			/* time */
			sscanf(string,"%lf",&time);


			printf("%lf\t%lld\n",time,instructions);

			if (time<time_min) time_min=time;
			if (time>time_max) time_max=time;
			time_total+=time;

			if (instructions<instructions_min) instructions_min=instructions;
			if (instructions>instructions_max) instructions_max=instructions;
			instructions_total+=instructions;

			fclose(fff);
		}

	}

	time_average=time_total/RUNS;
	instructions_average=instructions_total/RUNS;

	printf("Average time: %lf (Max %lf Min %lf)\n",
		time_average,time_max,time_min);
	printf("Average instructions: %lld (Max %lld Min %lld)\n",
		instructions_average,instructions_max,instructions_min);

	return 0;
}
