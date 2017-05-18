#include <stdio.h>
#include <stdlib.h>

#include "threads.h"


int main(int argc, char** argv)
{

	pthread_t th[5];
	thread_params* params = (thread_params*)malloc(sizeof(thread_params)*5);
	pthread_mutex_init(&cerrojo,NULL);
	int i;

	for (i=0;i<5;i++)
	{
		params[i].threadId = i;
		pthread_create(&(th[i]), NULL, (void * (*)(void *))&funcParalela, &(params[i]));
	}
	printf("Hola\n");
	for (i=0;i<5;i++)
	{
		pthread_join(th[i], NULL);
	}

	free(params);
	pthread_mutex_destroy(&cerrojo);

	return 0;
}
