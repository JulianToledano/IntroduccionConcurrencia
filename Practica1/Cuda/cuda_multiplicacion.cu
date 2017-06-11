#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cuda.h>
#include "debug_time.h"

#define PROCESADORES 8

typedef struct matriz_t
{
	int filas;
	int columnas;
	int** datos;
} matriz_t;

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

__device__ int multiplicarVector(int lado, int* fila, int* columna) {
	int resultado = 0;
	for (int i = 0; i < lado; i++)
		resultado += fila[i] * columna[i];

	return resultado;
}

__global__ void multiplicarMatrices(int tamano, int** matriz1, int** matriz2, int** resultado) {

	int columna = blockIdx.y * blockDim.x + threadIdx.x;
	int fila = blockIdx.x * blockDim.y + threadIdx.y;
	if((columna >= tamano) || (fila>= tamano)) return;
	resultado[fila][columna] = multiplicarVector(tamano, matriz1[fila], matriz2[columna]);
}

int main(int argc, char** argv) {

	DEBUG_TIME_INIT;
	DEBUG_TIME_START;

	// Leer matrices en el host
	matriz_t m1, m2, mres;
	m1 = leerMatriz(argv[1], 0);
	m2 = leerMatriz(argv[2], 1);
	// Reservar memoria para el resultado
	mres.filas = m1.filas;
	mres.columnas = m2.columnas;
	mres.datos = crearMatriz(mres.filas, mres.columnas);

	// Datos de intercambio entre host y device
	int **i_m1, **i_m2, **i_res;
	// Datos del device
	int **d_m1, **d_m2, **d_res;

	i_m1 = (int**)malloc(m1.columnas * sizeof(int*));
	i_m2 = (int**)malloc(m1.columnas * sizeof(int*));
	i_res = (int**)malloc(m1.columnas * sizeof(int*));

	// Reservamos memoria para las matrices en device
	cudaMalloc((void**)&d_res,sizeof(int*)* m1.columnas);
	cudaMalloc((void**)&d_m1,sizeof(int*)* m1.columnas);
	cudaMalloc((void**)&d_m2,sizeof(int*)* m1.columnas);

	// Reservamos memoria para la zona de intercabio
	for(int i = 0; i < m1.columnas; i++){
		cudaMalloc((void**)&(i_m1[i]),sizeof(int)* m1.columnas);
		cudaMalloc((void**)&(i_m2[i]),sizeof(int)* m1.columnas);
		cudaMalloc((void**)&(i_res[i]),sizeof(int)* m1.columnas);
	}
	// Copiamos los datos a la zona de intercambio
	for(int i = 0; i < m1.columnas;i++){
		cudaMemcpy(i_m1[i], m1.datos[i], m1.columnas * sizeof(int),cudaMemcpyHostToDevice);
		cudaMemcpy(i_m2[i], m2.datos[i], m1.columnas * sizeof(int),cudaMemcpyHostToDevice);
	}
	// Copiamos lso datos de la zona de intercambio a la memoria device
	cudaMemcpy(d_m1, i_m1, m1.columnas * sizeof(int*),cudaMemcpyHostToDevice);
	cudaMemcpy(d_m2, i_m2, m1.columnas * sizeof(int*),cudaMemcpyHostToDevice);
	cudaMemcpy(d_res, i_res, m1.columnas * sizeof(int*),cudaMemcpyHostToDevice);

	//int bloque = 32;

	dim3 grid = dim3((m1.columnas / 32) + 1, (m1.columnas / 32) + 1, 1);
	dim3 block = dim3(32, 32, 1);
	{
	DEBUG_TIME_INIT;
	DEBUG_TIME_START;
	multiplicarMatrices<<<grid, block>>> (m1.columnas, d_m1, d_m2, d_res);
	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo multiplicacion: ");
	}
	for(int i = 0; i < m1.columnas;i++)
		cudaMemcpy(mres.datos[i], i_res[i], m1.columnas * sizeof(int), cudaMemcpyDeviceToHost);

	// Escribir resultado
	escribirMatriz(mres.datos, mres.filas, mres.columnas, argv[3]);

	//free

	for(int i = 0; i < m1.columnas; i++){
	//	free(matriz1_CPU[i]);
	//	free(matriz2_CPU[i]);
	//	free(resultadoFinal_CPU[i]);

		cudaFree(i_m1[i]);
		cudaFree(i_m2[i]);
		cudaFree(i_res[i]);

	}

	//free(matriz1_CPU);
	//free(matriz2_CPU);
	//free(resultadoFinal_CPU);
	free(i_m1);
	free(i_m2);
	free(i_res);


	cudaFree(d_m1);
	cudaFree(d_m2);
	cudaFree(d_res);

	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo Total: ");

	return 0;
}
