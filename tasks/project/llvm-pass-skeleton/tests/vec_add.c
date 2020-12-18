#include <stdio.h>
#include <stdlib.h>

int main() {
  int n = 10;
  int m = 10;
  int res[m][n];
  OUTER:
  for (int i = 0; i < n; i++) {
  INNER:
    for (int j = 0; j < m; j++) {
      res[i][j] = m;
    }
  }
  printf("Multiplication res: %d; \n", res[0][0]);
  return 0;
}
