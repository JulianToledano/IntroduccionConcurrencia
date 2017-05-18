#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug_time.h"

#define PROCESADORES 8

typedef struct matriz_t
{
	int filas;
	int columnas;
	int** datos;
} matriz_t;

typedef struct paquete_trabajo{
	int **matriz1_datos;
	int matriz1_inicial;
	int matriz1_final;

	int **matriz2_datos;
	int matriz2_inicial;
	int matriz2_final;
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
/*
int multiplicaVectores(int* v1, int* v2, int size) {
	int result = 0,i;
	for (i = 0; i < size; ++i)
	{
		result += v1[i]*v2[i];
	}
	return result;
}

void multiplicarMatrices(matriz_t m1, matriz_t m2, matriz_t mres) {
	int i, j;
	for(i = 0; i < m1.filas; i++) {
        for(j = 0; j < m2.columnas; j++) {
            mres.datos[i][j] = multiplicaVectores(m1.datos[i], m2.datos[j], m2.columnas);
        }
	}
}
*/
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

__global__  void multiplicarMatrices(void){
	printf("zdfds");
}

int main(int argc, char** argv) {
/// uso programa: multiplicarMatricesSec <matriz1> <matriz2> <matrizresultado> // Recomendacion una clase matriz

	// cargar datos
	//matriz_t m1, m2, mres;
	int *d_m1;
	/*m1 = leerMatriz(argv[1], 0);
	m2 = leerMatriz(argv[2], 1);*/

	/*int **temp;
	temp = (int**)malloc(sizeof(int*)*m1.filas);*/
	cudaMalloc((void**)&d_m1, 10*sizeof(int));
	/*for(int i = 0; i < m1.filas; i++){
		cudaMalloc((void**)&(temp[i]),m1.columnas*sizeof(int));
		cudaMemcpy(temp[i], m1.datos[i], m1.columnas*sizeof(int), cudaMemcpyHostToDevice);
		cudaMemcpy(d_m1+i, &(temp[i]), sizeof(int*), cudaMemcpyHostToDevice);
	}*/

	// reservar resultado
	/*mres.filas = m1.filas;
	mres.columnas = m2.columnas;
	mres.datos = crearMatriz(mres.filas, mres.columnas);*/
	printf("antes");
	multiplicarMatrices<<<2,2>>>();
	printf("dspues");


	// Escribir resultado
	//escribirMatriz(mres.datos, mres.filas, mres.columnas, argv[3]);


	// Imprimir matriz
	//printMatrix(mres);
	// Liberar datos
}
