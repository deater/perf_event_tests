#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int main(int argc, char **argv) {

  char name[BUFSIZ],lowername[BUFSIZ];
  char description[BUFSIZ];
  char filename[BUFSIZ];

  int length,i;

  FILE *code,*makefile,*makefile2,*test;

  makefile=fopen("Makefile.inc","w");
  if (makefile==NULL) {
    printf("Error opening Makefile.inc!\n");
    exit(1);
  }

  makefile2=fopen("Makefile2.inc","w");
  if (makefile2==NULL) {
    printf("Error opening Makefile2.inc!\n");
    exit(1);
  }

  test=fopen("test.sh","w");
  if (test==NULL) {
    printf("Error opening test.sh\n");
    exit(1);
  }

  while(1) {

    if (fgets(name,BUFSIZ,stdin)==NULL) break;
    length=strlen(name);
    name[length-1]='\0';  /* strip off ending linefeed */

    for(i=0;i<length;i++) {
      lowername[i]=tolower(name[i]);
    }
    fprintf(makefile,"\t%s \\\n",lowername); 
    fprintf(test,"./papi/%s\n",lowername); 

    fprintf(makefile2,"###\n\n");
    fprintf(makefile2,"%s:\t%s.o ../lib/test_utils.o\n",lowername,lowername);
    fprintf(makefile2,"\t$(CC) $(LFLAGS) -o %s %s.o \\\n",lowername,lowername);
    fprintf(makefile2,"\t\t../lib/test_utils.o $(PAPI_LIB)\n\n");
    fprintf(makefile2,"%s.o:  %s.c\n",lowername,lowername);
    fprintf(makefile2,"\t$(CC) $(CFLAGS) $(PAPI_INCLUDE) -c %s.c\n\n",lowername);

    if (fgets(description,BUFSIZ,stdin)==NULL) break;
    length=strlen(description);
    description[length-1]='\0';  /* strip off ending linefeed */

    sprintf(filename,"%s.c",lowername);
    code=fopen(filename,"w");
    if (code==NULL) {
      printf("Error!  Coult not open %s\n",filename);
      exit(1);
    }

  fprintf(code,"/* This code runs some sanity checks on the %s */\n",name);
  fprintf(code,"/*   (%s) events.                   */\n",description);
  fprintf(code,"\n");
  fprintf(code,"/* by Vince Weaver, vweaver1 _at_ eecs.utk.edu           */\n");
  fprintf(code,"\n");
  fprintf(code,"#include <stdio.h>\n");
  fprintf(code,"#include <stdlib.h>\n");
  fprintf(code,"#include <unistd.h>\n");
  fprintf(code,"\n");
  fprintf(code,"#include \"papi.h\"\n");
  fprintf(code,"\n");
  fprintf(code,"#include \"test_utils.h\"\n");
  fprintf(code,"\n");
  fprintf(code,"int main(int argc, char **argv) {\n");
  fprintf(code,"\n");
  fprintf(code,"\tint events[1];\n");
  fprintf(code,"\tlong long counts[1];\n");
  fprintf(code,"\n");
  fprintf(code,"\tint retval,quiet;\n");
  fprintf(code,"\n");
  fprintf(code,"\tchar test_string[]=\"Testing %s predefined event...\";\n",name);
  fprintf(code,"\n");
  fprintf(code,"\tquiet=test_quiet();\n");
  fprintf(code,"\n");
  fprintf(code,"\tretval = PAPI_library_init(PAPI_VER_CURRENT);\n");
  fprintf(code,"\tif (retval != PAPI_VER_CURRENT) {\n");
  fprintf(code,"\t\tif (!quiet) printf(\"Error! PAPI_library_init %%d\\n\",retval);\n");
  fprintf(code,"\t\ttest_fail(test_string);\n");
  fprintf(code,"\t}\n");
  fprintf(code,"\n");
  fprintf(code,"\tretval = PAPI_query_event(%s);\n",name);
  fprintf(code,"\tif (retval != PAPI_OK) {\n");
  fprintf(code,"\t\tif (!quiet) printf(\"%s not available\\n\");\n",name);
  fprintf(code,"\t\ttest_skip(test_string);\n");
  fprintf(code,"\t}\n");
  fprintf(code,"\n");
  fprintf(code,"\tevents[0]=%s;\n",name);
  fprintf(code,"\n");
  fprintf(code,"\tPAPI_start_counters(events,1);\n");
  fprintf(code,"\n");
  fprintf(code,"\tPAPI_stop_counters(counts,1);\n");
  fprintf(code,"\n");
  fprintf(code,"\tif (counts[0]<1) {\n");
  fprintf(code,"\t\tif (!quiet) printf(\"Error! Count too low\\n\");\n");
  fprintf(code,"\t\ttest_fail(test_string);\n");
  fprintf(code,"\t}\n");
  fprintf(code,"\n");
  fprintf(code,"\tPAPI_shutdown();\n");
  fprintf(code,"\n");
  fprintf(code,"\ttest_unimplemented(test_string);\n");
  fprintf(code,"\n");
  fprintf(code,"\treturn 0;\n");
  fprintf(code,"}\n");

     fclose(code);
  }

  fclose(makefile);
  fclose(makefile2);
  fclose(test);

  return 0;
}
