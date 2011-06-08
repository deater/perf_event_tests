#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define NUM_RUNS 3

#define MATRIX_SIZE 512
double a[MATRIX_SIZE][MATRIX_SIZE];
double b[MATRIX_SIZE][MATRIX_SIZE];
double c[MATRIX_SIZE][MATRIX_SIZE];

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

  double s;
  int i,j,k;

  for(i=0;i<MATRIX_SIZE;i++) {
    for(j=0;j<MATRIX_SIZE;j++) {
      a[i][j]=(double)i*(double)j;
      b[i][j]=(double)i/(double)(j+5);
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

    printf("Validation: s=%lf\n",s);

  return;
}

int main(int argc, char **argv) {
   
      matrix_multiply();

   return 0;
}
