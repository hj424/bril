#include <stdlib.h> 
#include <cstdio>
#include <ctime>

#define N 1000000000

int main() {
  std::clock_t start;
  double runtime;
  
  int* res = (int*) malloc (N * sizeof(int)); 
  start = std::clock();
  //#pragma unroll 16
  for (int i = 0; i < N; i++) {
    res[i] = i*i;
  }
  runtime = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
  printf("Multiplication res[2]: %d; \n", res[2]);
  printf("Runtime: %lf \n", runtime);
  return 0;
}
