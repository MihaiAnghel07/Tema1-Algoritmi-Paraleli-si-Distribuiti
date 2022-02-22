#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "genetic_algorithm.h"


int main(int argc, char *argv[]) {
	// array with all the objects that can be placed in the sack
	sack_object *objects = NULL;

	// number of objects
	int object_count = 0;

	// maximum weight that can be carried in the sack
	int sack_capacity = 0;

	// number of generations
	int generations_count = 0;
	
	if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, argc, argv)) {
		return 0;
	}
  	
  	// number of threads
  	int P = (int)atoi(argv[3]);
	pthread_t tid[P];
	input in[P];
	pthread_barrier_t *barrier = (pthread_barrier_t*)malloc(sizeof(pthread_barrier_t));
	pthread_mutex_t *mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));


	individual *current_generation = (individual*) calloc(object_count, sizeof(individual));
	individual *next_generation = (individual*) calloc(object_count, sizeof(individual));


	pthread_barrier_init(barrier, NULL, P);
	pthread_mutex_init(mutex, NULL);

	for (int i = 0; i < P; i++) {
		in[i].objects = objects;
		in[i].object_count = object_count;
		in[i].sack_capacity = sack_capacity;
		in[i].generations_count = generations_count;
		in[i].thread_id = i;
		in[i].P = P;
		in[i].current_generation = current_generation;
		in[i].next_generation = next_generation;
		in[i].barrier = barrier;
		in[i].mutex = mutex;
		pthread_create(&tid[i], NULL, run_genetic_algorithm, &in[i]);
	}

	// se asteapta thread-urile
	for (int i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}
	
	pthread_mutex_destroy(mutex);
	pthread_barrier_destroy(barrier);
	free(objects);
	free(current_generation);
	free(next_generation);

	return 0;
}
