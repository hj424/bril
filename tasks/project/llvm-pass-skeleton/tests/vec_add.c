#include <stdio.h>
#include <stdlib.h>

int main() {
  int n = 10;
  int m = 10;
  int res[m][n];
  LOOP:
  for (int i = 0; i < 10; i++) {
    for (int j = 0; j < 10; j++) {
      res[i][j] = m;
    }
  }
  printf("Multiplication res: %d; \n", res[0][0]);
  return 0;
}
