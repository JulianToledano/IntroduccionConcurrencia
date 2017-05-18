#include "threads.h"

int contador = 0;
pthread_mutex_t cerrojo;

void funcParalela(void* params)
{
	thread_params *myparams = (thread_params*)params;

	pthread_mutex_lock(&cerrojo);
	contador++;
	printf("holamundo thread: %d contador: %d\n",myparams->threadId, contador);
	pthread_mutex_unlock(&cerrojo);

	pthread_exit(NULL);

}
