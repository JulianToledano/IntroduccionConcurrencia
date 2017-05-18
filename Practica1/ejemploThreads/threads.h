#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#ifndef _THREADS_H_
#define _THREADS_H_


extern int contador;
extern pthread_mutex_t cerrojo;

typedef struct thread_params
{
	int threadId;

}thread_params;


void funcParalela(void* params);

#endif
