#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int** crearMatriz(int numFilas, int numColumnas) {
    int** matriz =(int**) malloc(sizeof(int*) * numFilas);
    int i;
    for(i = 0; i < numFilas; i++) {
        matriz[i] = (int*) malloc(sizeof(int) * numColumnas);
    }
    return matriz;
}

void rellenarMatriz(int** matriz, int numFilas, int numColumnas, int identidad) {

    int i, j;
    if(identidad) {
        for(i = 0; i < numFilas; i++) {
            for(j = 0; j < numFilas; j++) {
                if(i == j)
                {
                    matriz[i][j] = 1;
                }  else {
                    matriz[i][j] = 0;
                }
            }
        }
    } else {
        for(i = 0; i < numFilas; i++) {
            for(j = 0; j < numFilas; j++) {
                matriz[i][j] = rand();
            }
        }
    }

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
        for(j = 0; j < numFilas; j++) {
            sprintf(charaux, "%d ", matriz[i][j]);
            fwrite(charaux, sizeof(char), strlen(charaux), fich);
        }
    }
    fclose(fich);
}



int main(int argc, char** argv)
{
/// Uso programa: crearMatrices <numfilas> <numcolumnas> <identidad> <nombreFich> en identidad 0 para identidad y 1 matriz random
    // Lectura numfilas
    int numFilas = atoi(argv[1]);
    // Lectura numcolumnas
    int numColumnas = atoi(argv[2]);
    // Lectura identidad
    int identidad = atoi(argv[3]);
    // Lectura nombreFichero
    char* fileName = argv[4];
    // Crear matriz reserva de memoria malloc
    int** matriz = crearMatriz(numFilas, numColumnas);
    // Rellenar matriz
    rellenarMatriz(matriz, numFilas, numColumnas, identidad);
    // Escribir matriz
    escribirMatriz(matriz, numFilas, numColumnas, fileName);
    return 0;
}
