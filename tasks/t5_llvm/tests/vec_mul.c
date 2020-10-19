#include <stdio.h>
#include <stdlib.h>

int main() {
  int n = 10;
  int* res = (int*) malloc (n*sizeof(int));
  for (int i = 0; i < n; i++) {
    res[i] = i * i;
  }
  return 0;
}
