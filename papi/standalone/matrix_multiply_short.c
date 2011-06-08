#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_RUNS 3

#define MATRIX_SIZE 512
float a[MATRIX_SIZE][MATRIX_SIZE];
float b[MATRIX_SIZE][MATRIX_SIZE];
float c[MATRIX_SIZE][MATRIX_SIZE];

long long estimated_flops() {

  long long muls,divs,adds;

  /* setup */
  muls=MATRIX_SIZE*MATRIX_SIZE;
  divs=MATRIX_SIZE*MATRIX_SIZE;
  adds=MATRIX_SIZE*MATRIX_SIZE;

  /* multiply */
  muls+=MATRIX_SIZE*MATRIX_SIZE*MATRIX_SIZE;
  adds+=MATRIX_SIZE*MATRIX_SIZE*MATRIX_SIZE;

  /* sum */
  adds+=MATRIX_SIZE*MATRIX_SIZE;

  printf("adds: %lld muls: %lld divs: %lld\n",adds,muls,divs);

  return adds+muls+divs;
}

void matrix_multiply() {

  float s;
  int i,j,k;

  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      a[i][j]=(float)i*(float)j;
      b[i][j]=(float)i/(float)(j+5);
    }
  }

  for(j=0;j<MATRIX_SIZE;j++) {
     for(i=0;i<MATRIX_SIZE;i++) {
        s=0;
        for(k=0;k<MATRIX_SIZE;k++) {
	   s+=a[i][k]*b[k][j];
	}
        c[i][j] = s;
     }
  }

  s=0.0;
  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      s+=c[i][j];
    }
  }

    printf("Validation: s=%f\n",s);

  return;
}

int main(int argc, char **argv) {
   
      matrix_multiply();

   return 0;
}
