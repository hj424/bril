#include <stdio.h>

void cnt_mul() {
  static int n = 0;
  n += 1;
  printf("Integer multiply detected! Total # of accumulated IMUL is %d. \n", n);
}
