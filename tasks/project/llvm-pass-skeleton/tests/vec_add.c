#include <stdio.h>
#include <stdlib.h>

#define N 11
#define M 20

int main() {
  int init_val = 30;
  int res[M*N];
  LOOP:
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      res[i*M+j] = init_val;
    }
  }
  printf("Multiplication res: %d; \n", res[0]);
  return 0;
}
