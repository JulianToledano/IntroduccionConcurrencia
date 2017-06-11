#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct paquete_trabajo{
	int filas_m1;
  int columnas_m1;
  int inicio_m1;
  int final_m1;

  int filas_m2;
  int columnas_m2;
  int inicio_m2;
  int final_m2;
}paquete;



int main(){
  int rank;
  MPI_Status status;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Recibir cabecera
  paquete trabajo;
  MPI_Recv(&trabajo, sizeof(paquete_trabajo), MPI_BYTE, 0, 1, MPI_COMM_WORLD, &status);

  // Reservar espacio en memoria
  int **matriz_1;
  int **matriz_2;
  int **resultado;

  matriz_1 = (int**)malloc(sizeof(int*)*(trabajo.final_m1 - trabajo.inicio_m1));
  matriz_2 = (int**)malloc(sizeof(int*)*(trabajo.final_m2 - trabajo.inicio_m2));
  resultado = (int**)malloc(sizeof(int*)*(trabajo.final_m1 - trabajo.inicio_m1));

  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++){
    matriz_1[i] = (int*)malloc(sizeof(int) * trabajo.columnas_m1);
    resultado[i] = (int*)malloc(sizeof(int) * trabajo.columnas_m1);
  }
  for(int i = 0; i < trabajo.final_m2 - trabajo.inicio_m2; i++)
    matriz_2[i] = (int*)malloc(sizeof(int) * trabajo.columnas_m2);

  // Recivir datos matriz 1
  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++)
    MPI_Recv(matriz_1[i], trabajo.columnas_m1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
  // Reciver datos matriz 2
  for(int i = 0; i < trabajo.final_m2 - trabajo.inicio_m2; i++)
    MPI_Recv(matriz_2[i], trabajo.columnas_m2, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);

  // Multiplicar matrices
  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++)
    for(int j = 0; j < trabajo.filas_m2; j++){
      resultado[i][j] = 0;
      for(int k = 0; k < trabajo.columnas_m2; k++)
        resultado[i][j] += matriz_1[i][k] * matriz_2[j][k];
    }

  // Enviamos resultado
  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++)
    MPI_Send(resultado[i],trabajo.columnas_m2,MPI_INT,0,3, MPI_COMM_WORLD);
    
  // Liberar memoria
  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++){
    free(matriz_1[i]); free(resultado[i]);
  }
  for(int i = 0; i < trabajo.final_m2 - trabajo.inicio_m2; i++)
    free(matriz_2[i]);
  free(matriz_1); free(matriz_2); free(resultado);

  MPI_Finalize();
  return 0;
}
