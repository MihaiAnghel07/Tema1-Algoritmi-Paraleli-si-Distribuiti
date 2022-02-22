#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include "genetic_algorithm.h"


int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4) {
		fprintf(stderr, "Usage:\n\t./tema1_par in_file generations_count P\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2) {
		fclose(fp);
		return 0;
	}

	if (*object_count % 10) {
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *) calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i) {
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2) {
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int) strtol(argv[2], NULL, 10);
	
	if (*generations_count == 0) {
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

int min(int a, int b) {
    return a < b ? a : b;
}

int compute_start(input in, int limit)
{
	return in.thread_id * ceil(limit / in.P);
}

int compute_end(input in, int limit)
{
	return min((in.thread_id + 1) * ceil(limit / in.P), limit);
}

void swap(individual *a, individual *b)
{
	individual tmp = *a;
	*a = *b;
	*b = tmp;
}

int check(int end, input in, int limit)
{	
	// check if there are unassigned tasks to the threads
	// and the rest of tasks will be assigned to the last thread
	// (not the best idea but the simplest)
	 if (end == 0 || (in.thread_id == in.P - 1 && end != limit))
	 	return 1;

	 return 0;
}

void oets(input in, individual *generation, int limit)
{
	int start = compute_start(in, limit);
    int end = compute_end(in, limit);
    if (end == limit) {
    	end--;
	}
    
    for (int k = 0; k < limit; k++) {
	    for (int i = start; i < end; i += 2) {
	    	if (generation[i].fitness < generation[i + 1].fitness) {
	        	swap(&generation[i], &generation[i + 1]);
	      	}
	    }
	    pthread_barrier_wait(in.barrier);
	    for (int i = start + 1; i < end; i += 2) {
	      	if (generation[i].fitness < generation[i + 1].fitness) {
	        	swap(&generation[i], &generation[i + 1]);
	      	}
	    }
	    pthread_barrier_wait(in.barrier);
  	}

}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i) {
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i) {
		for (int j = 0; j < generation[i].chromosome_length; ++j) {
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(input in, sack_object *objects, individual *generation, int object_count, int sack_capacity)
{
	int weight;
	int profit;
	
	int start = compute_start(in, object_count);
    int end = compute_end(in, object_count);
    if (check(end, in, object_count)) {
    	end = object_count;
    }
    
    for (int i = start; i < end; ++i) {
		weight = 0;
		profit = 0;
		
		for (int j = 0; j < generation[i].chromosome_length; j++) {
			if (generation[i].chromosomes[j]) {
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}

}

int cmpfunc(const void *a, const void *b)
{
	int i;
	individual *first = (individual *) a;
	individual *second = (individual *) b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0) {
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i) {
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0) {
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(input in, const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);
	
	if (ind->index % 2 == 0) {

		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step) {
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
		
	} else {
		
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		i = ind->chromosome_length - mutation_size;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step) {
				ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}

}

void mutate_bit_string_2(input in, const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step) {
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
	
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(input in, individual *current_generation, individual *next_generation)
{
	int start = compute_start(in, current_generation->chromosome_length);
    int end = compute_end(in, current_generation->chromosome_length);
    
    // free memory for current_generation and next_generation
    // both generations have the same length
	for (int i = start; i < end; ++i) {
		free(current_generation[i].chromosomes);
		free(next_generation[i].chromosomes);
		current_generation[i].chromosomes = NULL;
		current_generation[i].fitness = 0;
		next_generation[i].chromosomes = NULL;
		next_generation[i].fitness = 0;
	}
	
}

void *run_genetic_algorithm(void *arg)
{

	input in = *(input*)arg;
	sack_object *objects = in.objects;
	int object_count = in.object_count;
	int generations_count = in.generations_count;
	int sack_capacity = in.sack_capacity;
	int thread_id = in.thread_id;
	static int flag = 0;

	int count, cursor;
	individual *current_generation = in.current_generation;
	individual *next_generation = in.next_generation;
	individual *tmp = NULL;

	int start = compute_start(in, object_count);
    int end = compute_end(in, object_count);
    if (check(end, in, object_count)) {
    	end = object_count;
    }
	
	// set initial generation 
	// (composed of object_count individuals with a single item in the sack)
	for (int i = start; i < end; i++) {
		current_generation[i].fitness = 0;
		current_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		current_generation[i].chromosomes[i] = 1;
		current_generation[i].index = i;
		current_generation[i].chromosome_length = object_count;

		next_generation[i].fitness = 0;
		next_generation[i].chromosomes = (int*) calloc(object_count, sizeof(int));
		next_generation[i].index = i;
		next_generation[i].chromosome_length = object_count;
	}
	
	// iterate for each generation
	for (int k = 0; k < generations_count; ++k) {
		cursor = 0;
		compute_fitness_function(in, objects, current_generation, object_count, sack_capacity);
		
		// sorting should be done only after the fittnes has been calculated
		pthread_barrier_wait(in.barrier);
		oets(in, current_generation, object_count);

		// this part has remained since I was sorting with qsort
		// (it helps to change generations with just one thread)
		pthread_mutex_lock(in.mutex);
		if (flag == k) {
			flag++;
		}
		pthread_mutex_unlock(in.mutex);

		// keep first 30% children (elite children selection)
		count = object_count * 3 / 10;
		int start2 = compute_start(in, count);
		int end2 = compute_end(in, count);
		if (check(end2, in, count)) {
    		end2 = count;
    	}
    	// for the situation when there are more tasks than threads
    	if (start2 == 0 && end2 == count && in.P > 1) {
    		start2 = thread_id;
			end2 = start2 + 1;
    	}
    	if (end2 <= count) {
    		for (int i = start2; i < end2; ++i) {
				copy_individual(current_generation + i, next_generation + i);
			}
		}
		cursor = count;
		
		// mutate first 20% children with the first version of bit string mutation
		count = object_count * 2 / 10;
		int start3 = compute_start(in, count);
		int end3 = compute_end(in, count);
		if (check(end3, in, count)) {
	    	end3 = count;
	    }
	    // for the situation when there are more tasks than threads
    	if (start3 == 0 && end3 == count && in.P > 1) {
    		start3 = thread_id;
			end3 = start3 + 1;
    	}
		if (end3 <= count) {
			for (int i = start3; i < end3; ++i) {
				copy_individual(current_generation + i, next_generation + cursor + i);
				mutate_bit_string_1(in, next_generation + cursor + i, k);
			}
		}
		cursor += count;

		// mutate next 20% children with the second version of bit string mutation
		count = object_count * 2 / 10;
		int start4 = compute_start(in, count);
		int end4 = compute_end(in, count);
		if (check(end4, in, count)) {
	    	end4 = count;
	    }
	    if (start4 == 0 && end4 == count && in.P > 1) {
    		start4 = thread_id;
			end4 = start4 + 1;
    	}
		if (end4 <= count) {
			for (int i = start4; i < end4; ++i) {
				copy_individual(current_generation + i + count, next_generation + cursor + i);
				mutate_bit_string_2(in, next_generation + cursor + i, k);
			}
		}
		cursor += count;

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)
		count = object_count * 3 / 10;
		if (count % 2 == 1) {
			copy_individual(current_generation + object_count - 1, next_generation + cursor + count - 1);
			count--;
		}

		int start5 = compute_start(in, count);
		int end5 = compute_end(in, count);
		if (check(end5, in, count)) {
	    	end5 = count;
	    }
	    // for the situation when there are more tasks than threads
	    if (start5 == 0 && end5 == count && in.P > 1) {
    		start5 = thread_id;
			end5 = start5 + 1;
    	}
    	if (start5 % 2 != 0) {
    		start5++;
    	}

    	if (end5 <= count) {
			for (int i = start5; i < end5; i += 2) {
				crossover(current_generation + i, next_generation + cursor + i, k);
			}
		}	
		
		pthread_barrier_wait(in.barrier);
		// switch to new generation
		if (flag == k + 1) {
			tmp = current_generation;
			current_generation = next_generation;
			next_generation = tmp;
		}
	
		int start6 = compute_start(in, object_count);
   		int end6 = compute_end(in, object_count);
   		if (check(end6, in, object_count)) {
	    	end6 = object_count;
	    }	    	
		for (int i = start6; i < end6; ++i) {
			current_generation[i].index = i;
		}
		// for simplicity, the output is made by thread 0
		if (k % 5 == 0 && thread_id == 0) {
			print_best_fitness(current_generation);
		}
	}

	pthread_barrier_wait(in.barrier);
	compute_fitness_function(in, objects, current_generation, object_count, sack_capacity);
	pthread_barrier_wait(in.barrier);
	oets(in, current_generation, object_count);
	if (thread_id == 0) {
		print_best_fitness(current_generation);
	}
		
	pthread_barrier_wait(in.barrier);
	// free memory
	// free(current_generation->chromosomes);
	// free(next_generation->chromosomes);
	// free_generation(in, current_generation, next_generation);
	
	pthread_exit(NULL);
}