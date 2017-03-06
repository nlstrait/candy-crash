#include "array2d.h"
#include <stdio.h>

int main(int argc, char** argv) {
  array2d a2d = array2d_new(2, 2);
  int num1 = 10;
  array2d_update(a2d, num1, 0, 0);
  printf("%d\n", num1);
  int num2 = 20;
  array2d_get(a2d, &num2, 0, 0);
  printf("%d, %d", num1, num2);
  return 0;
}
