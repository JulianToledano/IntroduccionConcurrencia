#include <stdio.h>

#define N 512

/****************************************************************************
*                             TERMINOLOGÍA                                  *
*  cada invocación paralela de add se llama block                           *
*  el conjunto de blocks se llama grid                                      *
*  cada invocación puede referirse al indice de si bloque con blockIdx.x    *
*****************************************************************************/
__global__ void add(int*a, int*b, int*c) {
  c[blockIdx.x] = a[blockIdx.x] + b[blockIdx.x];
}

int main(){
  int *a, *b, *c;           // Copias de a b y c en el host
  int *d_a, *d_b, *d_c;     // Copias de a b y c en el device
  int size = N * sizeof(int);

  // Obtenemos espacio para las copias de a,b y c en device
  cudaMalloc((void**) &d_a, size);
  cudaMalloc((void**) &d_b, size);
  cudaMalloc((void**) &d_c, size);

  // Obtenemos espacio para las copias de a, b y c dentro del host
  a = (int *)malloc(size);
  b = (int *)malloc(size);
  for(int i = 0; i < N; i++){
    a[i] = i;
    b[i] = i;
  }


  c = (int *)malloc(size);

  // Copiamos los imput en device
  cudaMemcpy(d_a, a, size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_b, b, size, cudaMemcpyHostToDevice);

  // Lanzamos el kernel add dentro de la GPU
  add<<<N,1>>>(d_a, d_b, d_c);

  // Copiamos los resultados de nuevo en el host
  cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);

  for(int i = 0; i < N; i++)
    printf("%d, ",c[i]);

  // Liberamos memoria
  free(a);free(b);free(c);
  cudaFree(d_a);cudaFree(d_b);cudaFree(d_c);


}
