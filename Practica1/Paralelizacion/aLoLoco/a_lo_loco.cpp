#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "debug_time.h"
#include <unistd.h>

#define PROCESOS 8

typedef struct matriz_t
{
	int filas;
	int columnas;
	int** datos;
} matriz_t;

// Estructura que contiene la fila y la columna que tiene que multiplicar cada thread
typedef struct thread_params
{
	int fila;
	int columna;

}thread_params;

// Variables globales
matriz_t m1, m2, mres;

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

int multiplicaVectores(int* v1, int* v2, int size) {
	int result = 0,i;
	for (i = 0; i < size; ++i)
	{
		result += v1[i]*v2[i];
	}
	return result;
}

void multiplicarMatrices(void *params) {
		thread_params *myparams = (thread_params*)params;
    mres.datos[myparams->fila][myparams->columna] = multiplicaVectores(m1.datos[myparams->fila], m2.datos[myparams->columna], m2.columnas);

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
	// cargar datos

	m1 = leerMatriz(argv[1], 0);
	m2 = leerMatriz(argv[2], 1);

	int total = m1.filas * m2.columnas;
	int contador = 0;
	int proceso = 0;
	// reservar resultado
	mres.filas = m1.filas;
	mres.columnas = m2.columnas;
	mres.datos = crearMatriz(mres.filas, mres.columnas);

	// Creación de thread_params
	pthread_t th[total];
	thread_params* params = (thread_params*)malloc(sizeof(thread_params)*total);

	pthread_attr_t attr;
  cpu_set_t cpus;
  pthread_attr_init(&attr);

	{
		DEBUG_TIME_INIT;
		DEBUG_TIME_START;

		while(contador < total){
			for (int i = 0; i < m1.filas; i++)
				for (int j = 0; j < m1.columnas; j++){

					if(proceso == PROCESOS) proceso = 0;
					CPU_ZERO(&cpus);
       		CPU_SET(proceso, &cpus);
					pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);

					params[contador].fila = i;
					params[contador].columna = j;
					//int s = pthread_setaffinity_np(th[contador], sizeof(cpu_set_t), &cpuset);
					pthread_create(&(th[contador]), &attr, (void * (*)(void *))&multiplicarMatrices, &(params[contador]));
					//printf("%d, %d, %d\n", contador, params[contador].fila, params[contador].columna);
					contador++; proceso++;
				}
			}

			DEBUG_TIME_END;
			DEBUG_PRINT_FINALTIME("Tiempo func(): ");
	}



		for(int i = 0; i < total; i++)
			pthread_join(th[1], NULL);


		// Escribir resultado
		escribirMatriz(mres.datos, mres.filas, mres.columnas, argv[3]);

		DEBUG_TIME_END;
		DEBUG_PRINT_FINALTIME("Tiempo Total: ");

	// Imprimir matriz
	//printMatrix(mres);
	// Liberar datos
}
