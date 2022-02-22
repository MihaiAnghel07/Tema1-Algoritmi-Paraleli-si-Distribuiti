// structura ce ma ajuta sa parsez input-ul catre functia 
// pe care opereaza thread-urile
typedef struct input 
{
	sack_object *objects;
	int object_count;
	int sack_capacity;
	int generations_count;
	int thread_id;
	int P;
	individual *current_generation;
	individual *next_generation;
	pthread_barrier_t *barrier;
	pthread_mutex_t *mutex;
	
} input;