#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <list>
#include "debug_time.h"

#define PROCESADORES 8

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
/*
		// Liberar memoria nuevo.datos??
		for(int i = 0; i < m1.columnas; i++)
			free(nuevo.matriz1_datos[i]);
		free(nuevo.matriz1_datos);
		for(int i = 0; i < m2.columnas; i++)
			free(nuevo.matriz2_datos[i]);
		free(nuevo.matriz2_datos);*/
	}
}

void multiplicar_matrices(paquete pa){
	//paquete *myparams = (paquete*)param;
	int resultado = 0;

	for(int i = 0; i <= pa.matriz1_final - pa.matriz1_inicial; i++)
		for(int  j = 0; j <= pa.matriz2_final; j++){
				mres.datos[pa.matriz1_inicial + i][j] = 0;
				for( int k = 0; k <= pa.matriz2_final; k++)
					mres.datos[pa.matriz1_inicial + i][j] += pa.matriz1_datos[i][k] * pa.matriz2_datos[k][j];
		}
}

void obtener_paquete_y_multiplicar(void *param){
	TH_T *myparams = (TH_T*)param;
	while(!lista.empty()){
		pthread_mutex_lock(&cerrojo);
		paquete test = lista.front();
		lista.pop_front();
		pthread_mutex_unlock(&cerrojo);
		//printf("ID: %d inicial1: %d final1: %d inicial2: %d final2: %d\n",myparams->id, test.matriz1_inicial, test.matriz1_final, test.matriz2_inicial, test.matriz2_final);
		/*for(int i = 0; i < 2; i++){
			for(int j = 0; j < 20; j++)
				printf("%d ", test.matriz1_datos[i][j]);
			printf("%\n" );*/
			multiplicar_matrices(test);
		}
	}


int main(int argc, char** argv) {
/// uso programa: multiplicarMatricesSec <matriz1> <matriz2> <matrizresultado>
	DEBUG_TIME_INIT;
	DEBUG_TIME_START;
	int total = 100;
	// cargar datos
	matriz_t m1, m2;
	m1 = leerMatriz(argv[1], 0);
	m2 = leerMatriz(argv[2], 1);

	// reservar resultado
	mres.filas = m1.filas;
	mres.columnas = m2.columnas;
	mres.datos = crearMatriz(mres.filas, mres.columnas);


	rellenar_lista(total, m1, m2, &lista);

	// Creamos los threads
	pthread_mutex_init(&cerrojo,NULL);
	pthread_t th[PROCESADORES];
	TH_T* params = (TH_T*)malloc(sizeof(TH_T)*PROCESADORES);
	for(int i = 0; i < PROCESADORES; i++){
		params[i].id = i;
		pthread_create (&th[i],NULL, (void * (*)(void *))&obtener_paquete_y_multiplicar, &(params[i]));
	}

	for(int i = 0; i < PROCESADORES; i++)
		pthread_join(th[i], NULL);
	pthread_mutex_destroy(&cerrojo);
/*
	while(!lista.empty()){
		paquete test = lista.front();
		lista.pop_front();
		printf("inicial1: %d final1: %d inicial2: %d final2: %d\n", test.matriz1_inicial, test.matriz1_final, test.matriz2_inicial, test.matriz2_final);
		for(int i = 0; i < 1000; i++){
			for(int j = 0; j < 1000; j++)
				printf("%d ", test.matriz1_datos[i][j]);
			printf("%\n" );
		}
	}*/

	//for(int i = 0; i < PROCESADORES; i++)
	//	printf("%d %d %d\n", lista[i].matriz1_inicial, lista[i].matriz1_final,lista[i].matriz1_datos[0][0]);

	// Escribir resultado
	escribirMatriz(mres.datos, mres.filas, mres.columnas, argv[3]);

	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo Total: ");
	// Imprimir matriz
	//printMatrix(mres);
	// Liberar datos
}
