// Mezclar threads y bloques
#include <stdio.h>

#define N (2048 * 2048)
#define THREADS_PER_BLOCK 512

__global__ void add(int*a, int*b, int*c, int n) {
  int index = threadIdx.x + blockIdx.x * blockDim.x;
  if(index < n)
    c[index] = a[index] + b[index];
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
  add<<<(N+THREADS_PER_BLOCK-1)/THREADS_PER_BLOCK,THREADS_PER_BLOCK>>>(d_a, d_b, d_c,N);
  cudaMemcpy(c, d_c, size, cudaMemcpyDeviceToHost);

  for(int i = 0; i < N; i++)
    printf("%d, ",c[i]);

  // Liberamos memoria
  free(a);free(b);free(c);
  cudaFree(d_a);cudaFree(d_b);cudaFree(d_c);


}
