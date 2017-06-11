#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "debug_time.h"

#define NODOS 4

typedef struct TH_T{
	int id;
}TH_T;

typedef struct matriz_t
{
	int filas;
	int columnas;
	int** datos;
} matriz_t;

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


int** crearMatriz(int numFilas, int numColumnas) {
    int** matriz =(int**) malloc(sizeof(int*) * numFilas);
    int i;
    for(i = 0; i < numFilas; i++) {
        matriz[i] = (int*) malloc(sizeof(int) * numColumnas);
    }
    return matriz;
}

int leedato(FILE* fich) {
	//Mejor con memoria dinamica
	char dato[100];
	char datoleido;
	int contador = 0;
	do {
		if(!feof(fich)) {
			fread(&datoleido, sizeof(char), 1, fich);
			if(datoleido != ' ') {
				dato[contador] = datoleido;
				contador++;
			}
		} else datoleido = ' ';
	} while(datoleido != ' ');
	dato[contador] = '\0';
	int datointeger = atoi(dato);
	return datointeger;
}

matriz_t leerMatriz(char* nombreFichero, int traspuesta) {
	matriz_t matriz;
	int numFilas, numColumnas;
	FILE* fich = fopen(nombreFichero, "r");
	if(fich == NULL) {
		printf("Error\n");
		exit(0);
	}
	numFilas = leedato(fich);
	numColumnas = leedato(fich);
	matriz.datos = crearMatriz(numFilas, numColumnas);

	int i, j;
	if(!traspuesta) {
		for(i = 0; i < numFilas; i++) {
	        for(j = 0; j < numColumnas; j++) {
	            matriz.datos[i][j] = leedato(fich);
	        }
    	}
	} else {
		for(i = 0; i < numFilas; i++) {
	        for(j = 0; j < numColumnas; j++) {
	            matriz.datos[j][i] = leedato(fich);
	        }
	    }
	}
	matriz.filas = numFilas;
	matriz.columnas = numColumnas;
	fclose(fich);


    return matriz;
}

void escribirMatriz(int** matriz, int numFilas, int numColumnas, char* fileName) {
    FILE* fich = fopen(fileName, "w");
    if(fich == NULL) {
        printf("Error\n");
        return;
    }

    char charaux[100];

    sprintf(charaux, "%d %d ", numFilas, numColumnas);
    fwrite(charaux, sizeof(char), strlen(charaux), fich);
    int i,j;
    for(i = 0; i < numFilas; i++) {
        for(j = 0; j < numColumnas; j++) {
            sprintf(charaux, "%d ", matriz[i][j]);
            fwrite(charaux, sizeof(char), strlen(charaux), fich);
        }
    }
    fclose(fich);
}

void printMatrix(matriz_t matrix) {
	int i, j;
	for(i = 0; i < matrix.filas; i++) {
        for(j = 0; j < matrix.columnas; j++) {
            printf("%d ", matrix.datos[i][j]);
        }
        printf("\n");
    }
}


int main(int argc, char** argv) {
/// uso programa: multiplicarMatricesSec <matriz1> <matriz2> <matrizresultado>

	DEBUG_TIME_INIT;
	DEBUG_TIME_START;

  int rank;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Leer matrices de disco
  matriz_t m1, m2, mres;
  m1 = leerMatriz(argv[1], 0);
	m2 = leerMatriz(argv[2], 1);
  // reservar resultado
  mres.filas = m1.filas;
  mres.columnas = m2.columnas;
  mres.datos = crearMatriz(mres.filas, mres.columnas);
  // Generar 4 trabajos
  paquete nuevo;
  int division = m1.filas/NODOS;

	// Envio de datos
	{
	DEBUG_TIME_INIT;
	DEBUG_TIME_START;
  for(int i = 0; i < NODOS; i++){
    nuevo.filas_m1 = m1.filas;
    nuevo.columnas_m1 = m1.columnas;
    nuevo.inicio_m1 = division * i;
    nuevo.final_m1 = division * (i+1);

    nuevo.filas_m2 = m2.filas;
    nuevo.columnas_m2 = m2.columnas;
    nuevo.inicio_m2 = 0;
    nuevo.final_m2 = m2.columnas;
    // Enviar la cabecera
    MPI_Send(&nuevo, sizeof(struct paquete_trabajo), MPI_BYTE, i+1, 1, MPI_COMM_WORLD);
    for(int j = nuevo.inicio_m1; j < nuevo.final_m1; j++)
      MPI_Send(m1.datos[j], nuevo.columnas_m1, MPI_INT, i+1, 2, MPI_COMM_WORLD);
    for(int j = nuevo.inicio_m2; j < nuevo.final_m2; j++)
      MPI_Send(m2.datos[j], nuevo.columnas_m2, MPI_INT, i+1, 2, MPI_COMM_WORLD);
  }
	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo envío: ");
	}
	{
	DEBUG_TIME_INIT;
	DEBUG_TIME_START;
  // Como se reciven de forma ordenada no es necesario especificar la fila
	for(int i = 0; i < NODOS; i++)
    for(int j = division*i; j < division*(i+1); j++){
      MPI_Recv(mres.datos[j], nuevo.columnas_m2, MPI_INT, i+1, 3, MPI_COMM_WORLD, &status);
      //for(int k = 0; k < 100; k++) printf("%d ", mres.datos[j][k]);
    //printf("\n" );
  }
	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo recepción: ");
	}

	// Escribir resultado
	escribirMatriz(mres.datos, mres.filas, mres.columnas, argv[3]);
	// Liberar memoria
	for(int i = 0; i < m1.columnas; i++){
		free(m1.datos[i]);free(m2.datos[i]);free(mres.datos[i]);
	}
	free(m1.datos);free(m2.datos);free(mres.datos);

  MPI_Finalize();

	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo Total: ");
  return 0;
}
