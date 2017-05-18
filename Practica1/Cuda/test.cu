#include <stdio.h>

#define N 512

/***************************************************************
*                        TERMINOLOGÍA                          *
*  Un block puede ser dividido en distintos threads paralelos  *
*  Usamos  threadId.x en vez de blockIdx.x                     *
****************************************************************/

__global__ void add(/*int *a, int *b, int *c*/){
  //c[threadIdx.x] = a[threadIdx.x] + b[threadIdx.x];
  printf("hola desde el kernel");
}

int main(){

  int *a, *b, *c;               // Copias de a b y c en el host
  int *d_a, *d_b, *d_c;         // Copias de a b y c en el device
  int size = N * sizeof(int);

  // Resevamos memoria para las copias en el device
  cudaMalloc((void**)&d_a, size);
  cudaMalloc((void**)&d_b, size);
//  cudaMalloc((void**)&d_c, size);
/*
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
*/
  // Lanzamos add kernel con N threads
  add<<<1,N>>>(/*d_a, d_b, d_c*/);

  // Copiamos el resultado a memoria del host
  cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);
/*
  for(int i = 0; i < N; i++)
    printf("%d, ", c[i]);*/

  /*// Liberamos memoria
  free(a); free(b); free(c);
  cudaFree(d_a); cudaFree(d_b); cudaFree(d_c);
*/
  return 0;
}
