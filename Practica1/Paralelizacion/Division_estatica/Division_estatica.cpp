#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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

matriz_t mres;

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

// Creacion de un paquete por procesador
// paquete *lista -> lista que se rellena dentro de la finción.
void crear_paquetes(int procesadores, matriz_t m1, matriz_t m2, paquete *lista){

	int division = m1.filas / procesadores;
	int resto = division + (m1.filas % procesadores);

	for(int i = 0; i < procesadores; i++){
		lista[i].matriz1_inicial = i * division;
		// Si es el último procesador lo sobrecargamos en el caso de que la division no haya sido entera
		// ¿?¿? Tal vez mejor poner el final como el total de las filas¿?¿?
		if(i == procesadores-1){
			lista[i].matriz1_datos = crearMatriz(resto, m1.columnas);
			lista[i].matriz1_final = i * division + resto - 1;
		}
		else{
			lista[i].matriz1_datos = crearMatriz(division, m1.columnas);
			lista[i].matriz1_final = (i+1) * division - 1;
		}
		// Rellenamos los datos
		for(int j = 0; j < division; j++)
			for(int k = 0; k < m1.columnas; k++)
				lista[i].matriz1_datos[j][k] = m1.datos[lista[i].matriz1_inicial + j][k];
		// Si la división no ha sido entera y es el paquete del último proceso rellenamos los datos
		// de la sobrecarga
		if(resto && i == procesadores-1)
			for(int j = division; j < resto; j++)
				for(int k = 0; k < m1.columnas; k++)
					lista[i].matriz1_datos[j][k] = m1.datos[lista[i].matriz1_inicial + j][k];

		lista[i].matriz2_inicial = 0;
		lista[i].matriz2_final = m2.columnas - 1;
		lista[i].matriz2_datos = crearMatriz(m2.filas, m2.columnas);
		for(int j = 0; j < m2.filas; j++)
			for(int k = 0; k < m2.columnas; k++)
				lista[i].matriz2_datos[j][k] = m2.datos[j][k];
	}
}

void multiplicar_matrices(void *param){
	paquete *myparams = (paquete*)param;
	int resultado = 0;
	for(int i = 0; i <= myparams->matriz1_final - myparams->matriz1_inicial; i++)
		for(int  j = 0; j <= myparams->matriz2_final; j++){
				mres.datos[myparams->matriz1_inicial + i][j] = 0;
				for( int k = 0; k <= myparams->matriz2_final; k++)
					mres.datos[myparams->matriz1_inicial + i][j] += myparams->matriz1_datos[i][k] * myparams->matriz2_datos[j][k];
		}
}


int main(int argc, char** argv) {
/// uso programa: multiplicarMatricesSec <matriz1> <matriz2> <matrizresultado> // Recomendacion una clase matriz

	DEBUG_TIME_INIT;
	DEBUG_TIME_START;

	// cargar datos
	matriz_t m1, m2;
	m1 = leerMatriz(argv[1], 0);
	m2 = leerMatriz(argv[2], 1);

	paquete lista[PROCESADORES];
	crear_paquetes(PROCESADORES, m1, m2, lista);

	// reservar resultado
	mres.filas = m1.filas;
	mres.columnas = m2.columnas;
	mres.datos = crearMatriz(mres.filas, mres.columnas);

	pthread_t th[PROCESADORES];

	{
	DEBUG_TIME_INIT;
	DEBUG_TIME_START;

	for(int i = 0; i < PROCESADORES; i++)
		pthread_create(&(th[i]), NULL, (void * (*)(void *))&multiplicar_matrices, &(lista[i]));

		for(int i = 0; i < PROCESADORES; i++)
			pthread_join(th[i], NULL);

		DEBUG_TIME_END;
		DEBUG_PRINT_FINALTIME("Tiempo mutiplicar: ");
	}
	// Escribir resultado
	escribirMatriz(mres.datos, mres.filas, mres.columnas, argv[3]);

	// Liberar memoria
	for(int i = 0; i < m1.columnas; i++){
		free(m1.datos[i]);free(m2.datos[i]);free(mres.datos[i]);
	}
	free(m1.datos);free(m2.datos);free(mres.datos);

	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo Total: ");

	return 0;
}
