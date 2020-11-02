#include <stdio.h>
#include <stdlib.h>

int main() {
  int n = 10;
  int tmp = 20;
  int res_tmp = 0;
  int* res = (int*) malloc (n*sizeof(int));
  for (int i = 0; i < n; i++) {
    res[i] = i + i;
    res_tmp = tmp * tmp;
  }
  printf("Multiplication res: %d; \n", res_tmp);
  return 0;
}
