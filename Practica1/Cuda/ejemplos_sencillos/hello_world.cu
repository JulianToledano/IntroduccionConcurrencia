#include <stdio.h>

__global__ void mykernel() {
  printf("hola desde kernel");
}

int main(void) {
  mykernel<<<1,1>>>();
  //printf("Hello World!\n");
  return 0;
}
