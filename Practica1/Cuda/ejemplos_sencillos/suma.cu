#include <stdio.h>

// add sera ejecuta en el device
// add será llamada desde el host
// add corre en device asi que a,b y c deben apuntar a memoria del device
__global__ void add(int *a, int *b, int *c){
  *c = *a + *b;
}

int main(void){
  int a, b, c;            // Copias de a b y c en el host
  int *d_a, *d_b, *d_c;   // Copias de a, b y c en el device
  int size = sizeof(int);

  // Obtenemos espacio para las copias de a,b y c en device
  cudaMalloc((void **)&d_a, size);
  cudaMalloc((void **)&d_b, size);
  cudaMalloc((void **)&d_c, size);

  // Valores input
  a = 2;
  b = 7;

  // Copiamos inputs a device
  cudaMemcpy(d_a, &a, size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, &b, size, cudaMemcpyHostToDevice);

  // Lanzamos add() kernen en la GPU
  add<<<1,1>>>(d_a, d_b, d_c);

  // Copiamos el resultado al host
  cudaMemcpy(&c, d_c, size, cudaMemcpyDeviceToHost);

  // Limpiamos
  cudaFree(d_a);
  cudaFree(d_b);
  cudaFree(d_c);

  printf("%d",c);
  
  return 0;
}
