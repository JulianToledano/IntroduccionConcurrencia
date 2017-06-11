#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "debug_time.h"
#include <unistd.h>

#define PROCESADORES 8

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
	int result = 0;
	for (int i = 0; i < size; i++)
		result += v1[i]*v2[i];

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

	// El sistema operativo solo deja crear un número de threads, este número se supera al multiplicar las matrices de 10000.
	// Por ese motivo se crean los threads en tandas de tamaño igual al número de columnas que se van destruyendo.
	// De esta manera se crean todos los threads necesarios pero no a la vez.
	pthread_t th[m1.columnas];
	thread_params *params = (thread_params*)malloc(sizeof(thread_params)*m1.columnas);


	// Se capa el número de procesadores que se pueden usar a mano.
	/**********************************************************************************************
	* Como se indica manualmente que número de procesador usar:																		*
	*		1 procesador -> procesador 0																															*
	*		2 procesador -> procesador 0,1																														*
	*		4 procesador -> procesador 0,1,2,3																												*
	*		8 procesador -> procesador 0,1,2,3,4,5,6,7																								*
	*	Es posible que ese procesador esté ocupado. Siendo menos eficiente de lo que se suponía.		*
	**********************************************************************************************/
	// Inicializamos los atributos a los que puntará attr. (http://man7.org/linux/man-pages/man3/pthread_attr_init.3.html)
	pthread_attr_t attr;
  cpu_set_t cpus;
	pthread_attr_init(&attr);

	{
	DEBUG_TIME_INIT;
	DEBUG_TIME_START;


	for (int i = 0; i < m1.filas; i++){
		for (int j = 0; j < m1.columnas; j++){
			// Cuando se supere el número máximo de procesadores que se pueden utilizar el próximo thread se le
			// adjudica al procesador 0.
			if(proceso == PROCESADORES) proceso = 0;
			// http://man7.org/linux/man-pages/man3/CPU_SET.3.html
			CPU_ZERO(&cpus);																									// Clears set, so that it contains no CPUs.
      CPU_SET(proceso, &cpus);																					// Add CPU cpu to set.
			// pthread_attr_setaffinity_np sets the CPU affinity mask attribute of the thread attributes
			// object referred to by attr to the value specified in cpuset
			//(http://man7.org/linux/man-pages/man3/pthread_attr_setaffinity_np.3.html)
			pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);

			params[j].fila = i;
			params[j].columna = j;
			// Se crean tantos thrads como númeroDeColumnas.
			pthread_create(&(th[j]), &attr, (void * (*)(void *))&multiplicarMatrices, &(params[j]));
			proceso++;
		}
		// Se destruyen los threads para que luego se puedan crear más.
		for (int j = 0; j < m1.columnas; j++)
			pthread_join(th[j], NULL);
	}

	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo multiplicacion: ");
	}

	pthread_attr_destroy(&attr);
	// Escribir resultado
	escribirMatriz(mres.datos, mres.filas, mres.columnas, argv[3]);

	// Liberar memoria
	for(int i = 0; i < m1.columnas; i++){
		free(m1.datos[i]);free(m2.datos[i]);free(mres.datos[i]);
	}
	free(m1.datos);free(m2.datos);free(mres.datos);

	DEBUG_TIME_END;
	DEBUG_PRINT_FINALTIME("Tiempo Total: ");

	// Liberar datos
}
