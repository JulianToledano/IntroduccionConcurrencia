#include <stdio.h>
#define N 510
__global__ void holaMundo() {
  printf("hola desde kernel");
}

int main(void) {

  int *a, *b, *c;               // Copias de a b y c en el host
  int *d_a, *d_b, *d_c;         // Copias de a b y c en el device
  int size = N * sizeof(int);

  // Resevamos memoria para las copias en el device
  cudaMalloc((void**)&d_a, size);
  cudaMalloc((void**)&d_b, size);
  cudaMalloc((void**)&d_c, size);

  // Obtenemos espacio para las copias de a, b y c dentro del host
  a = (int *)malloc(size);
  b = (int *)malloc(size);
  c = (int *)malloc(size);
  for(int i = 0; i < N; i++){
    a[i] = i;
    b[i] = i;
  }

  // Copiamos los inputs en el device
  cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);

  printf("antes de entrar en gpu\n");
  holaMundo<<<1,N>>>();
  printf("despues de gpu!\n");

  // Copiamos el resultado a memoria del host
  cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);
/*
  for(int i = 0; i < N; i++)
    printf("%d, ", c[i]);
*/
  // Liberamos memoria
  free(a); free(b); free(c);
  cudaFree(d_a); cudaFree(d_b); cudaFree(d_c);

  return 0;
}
