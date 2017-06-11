#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <list>

#define PROCESADORES 8

typedef struct matriz_t
{
	int filas;
	int columnas;
	int** datos;
} matriz_t;

typedef struct paquete_nodo{
	int filas_m1;
  int columnas_m1;
  int inicio_m1;
  int final_m1;

  int filas_m2;
  int columnas_m2;
  int inicio_m2;
  int final_m2;
}paquete_nodo;

typedef struct paquete_trabajo{
	int **matriz1_datos;
	int matriz1_inicial;
	int matriz1_final;

	int **matriz2_datos;
	int matriz2_inicial;
	int matriz2_final;
}paquete;

matriz_t mres;
std::list<paquete> lista;
pthread_mutex_t cerrojo;

int** crearMatriz(int numFilas, int numColumnas) {
    int** matriz =(int**) malloc(sizeof(int*) * numFilas);
    int i;
    for(i = 0; i < numFilas; i++) {
        matriz[i] = (int*) malloc(sizeof(int) * numColumnas);
    }
    return matriz;
}

void rellenar_lista(int paquetes, matriz_t m1, matriz_t m2, std::list<paquete> *lista){

	int division = m1.filas / paquetes;
	paquete nuevo;

	for(int i = 0; i < paquetes; i++){
		nuevo.matriz1_inicial = i * division;
		nuevo.matriz1_final = (i + 1) * division - 1;
		nuevo.matriz1_datos = crearMatriz(division, m1.columnas);
		// Rellenamos los datos
		for(int j = 0; j < division; j++)
			for(int k = 0; k < m1.columnas; k++)
				nuevo.matriz1_datos[j][k] = m1.datos[nuevo.matriz1_inicial + j][k];

		nuevo.matriz2_inicial = 0;
		nuevo.matriz2_final = m2.columnas - 1;
		nuevo.matriz2_datos = crearMatriz(m2.filas, m2.columnas);
		//Rellenamos los datos
		for(int j = 0; j < m2.filas; j++)
			for(int k = 0; k < m2.columnas; k++)
				nuevo.matriz2_datos[j][k] = m2.datos[j][k];

		// Introdicimos en la lista
		lista->push_back(nuevo);
	}
}

void multiplicar_matrices(paquete pa){
	int resultado = 0;
	for(int i = 0; i <= pa.matriz1_final - pa.matriz1_inicial; i++)
		for(int  j = 0; j <= pa.matriz2_final; j++){
				mres.datos[pa.matriz1_inicial + i][j] = 0;
				for( int k = 0; k <= pa.matriz2_final; k++)
					mres.datos[pa.matriz1_inicial + i][j] += pa.matriz1_datos[i][k] * pa.matriz2_datos[j][k];
		}
}

void obtener_paquete_y_multiplicar(/*void *param*/){
	while(!lista.empty()){
		pthread_mutex_lock(&cerrojo);
		paquete test = lista.front();
		lista.pop_front();
		pthread_mutex_unlock(&cerrojo);
		multiplicar_matrices(test);
		}
}

int main(){
  int rank;
  MPI_Status status;
  MPI_Init(NULL, NULL);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Recibir cabecera
  paquete_nodo trabajo;
  MPI_Recv(&trabajo, sizeof(paquete_nodo), MPI_BYTE, 0, 1, MPI_COMM_WORLD, &status);

  // Reservar espacio en memoria
  matriz_t matriz_1;
  matriz_t matriz_2;

  matriz_1.datos = (int**)malloc(sizeof(int*)*(trabajo.final_m1 - trabajo.inicio_m1));
  matriz_2.datos = (int**)malloc(sizeof(int*)*(trabajo.final_m2 - trabajo.inicio_m2));
  mres.datos = (int**)malloc(sizeof(int*)*(trabajo.final_m1 - trabajo.inicio_m1));

  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++){
    matriz_1.datos[i] = (int*)malloc(sizeof(int) * trabajo.columnas_m1);
    mres.datos[i] = (int*)malloc(sizeof(int) * trabajo.columnas_m1);
  }
  for(int i = 0; i < trabajo.final_m2 - trabajo.inicio_m2; i++)
    matriz_2.datos[i] = (int*)malloc(sizeof(int) * trabajo.columnas_m2);

  // Recivir datos matriz 1
  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++)
    MPI_Recv(matriz_1.datos[i], trabajo.columnas_m1, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);
  // Reciver datos matriz 2
  for(int i = 0; i < trabajo.final_m2 - trabajo.inicio_m2; i++)
    MPI_Recv(matriz_2.datos[i], trabajo.columnas_m2, MPI_INT, 0, 2, MPI_COMM_WORLD, &status);


	rellenar_lista(10, matriz_1, matriz_2, &lista);

	pthread_mutex_init(&cerrojo,NULL);
	pthread_t th[PROCESADORES];
	for(int i = 0; i < PROCESADORES; i++)
		pthread_create (&th[i],NULL, (void * (*)(void *))&obtener_paquete_y_multiplicar, NULL);

	for(int i = 0; i < PROCESADORES; i++)
		pthread_join(th[i], NULL);
	// Destruimos cerrojo
	pthread_mutex_destroy(&cerrojo);


  // Enviamos resultado
  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++)
    MPI_Send(mres.datos[i],trabajo.columnas_m2,MPI_INT,0,3, MPI_COMM_WORLD);
/******************
  // Liberar memoria
  for(int i = 0; i < trabajo.final_m1 - trabajo.inicio_m1; i++){
    free(matriz_1.datos[i]); free(mres.datos[i]);
  }
  for(int i = 0; i < trabajo.final_m2 - trabajo.inicio_m2; i++)
    free(matriz_2.datos[i]);
  free(matriz_1.datos); free(matriz_2.datos); free(mres.datos);
*/
  MPI_Finalize();
  return 0;
}
