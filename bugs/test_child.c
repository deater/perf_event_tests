#include <stdio.h>

double busywork(int count) {
 
  int i;
  double sum=0.0012;
   
  for(i=0;i<count;i++) {
    sum+=0.01;
  }
  return sum;
   
}


int main(int argc, char** argv) {
   
  double result;

   result=busywork(10000000);

   printf("test_child, result=%lf\n",result);

   return 0;
}
