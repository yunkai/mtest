#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_QUEUE_SIZE	1024
#define RANDOM(s)       (rng() % (s))

unsigned int max_object_size = 128;
unsigned int nr_thread_pairs = 10;
unsigned int nr_total_loops = 10000;

struct obj_queue {
	unsigned int head;
	unsigned int tail;
	unsigned int nr_objs;
	void *objs[MAX_QUEUE_SIZE];
};

unsigned int atomic_read(unsigned int *ptr)
{
	return __sync_add_and_fetch(ptr, 0);
}

unsigned int atomic_dec(unsigned int *ptr)
{
	return __sync_add_and_fetch(ptr, -1);
}

unsigned int atomic_inc(unsigned int *ptr)
{
	return __sync_add_and_fetch(ptr, 1);
}

/*
 * Ultra-fast RNG: Use a fast hash of integers.
 * 2**64 Period.
 * Passes Diehard and TestU01 at maximum settings
 */
static __thread unsigned long long rnd_seed;

static inline unsigned rng(void)
{
	unsigned long long c = 7319936632422683443ULL;
	unsigned long long x = (rnd_seed += c);

	x ^= x >> 32;
	x *= c;
	x ^= x >> 32;
	x *= c;
	x ^= x >> 32;

	/* Return lower 32bits */
	return x;
}

void *rand_malloc()
{
	return malloc(RANDOM(max_object_size));
}

void *producer_routine(void * data)
{
	struct obj_queue *q = (struct obj_queue *)data;
	unsigned int i;

	for (i = 0; i < nr_total_loops; i++) {
		if (atomic_read(&q->nr_objs) >= MAX_QUEUE_SIZE)
			continue;

		q->objs[q->tail++ % MAX_QUEUE_SIZE] = rand_malloc();

		atomic_inc(&q->nr_objs);

	}

	pthread_exit(NULL);
}

void *consumer_routine(void *data)
{
	struct obj_queue *q = (struct obj_queue *)data;
	unsigned int i;

	for (i = 0; i < nr_total_loops; i++) {
		void *ptr;

		if (atomic_read(&q->nr_objs) == 0)
			continue;

		ptr = q->objs[q->head++ % MAX_QUEUE_SIZE];

		free(ptr);

		atomic_dec(&q->nr_objs);
	}

	pthread_exit(NULL);
}

void show_help(const char *cmd)
{
	printf("%s <max_object_size> <nr_thread_pairs> <nr_total_loops>\n",
	       cmd);
}

int main(int argc, char *argv[])
{
	int i;

	if (argc < 4) {
		show_help(argv[0]);
		exit(EXIT_FAILURE);
	}

	max_object_size = atoi(argv[1]);
	nr_thread_pairs = atoi(argv[2]);
	nr_total_loops = atoi(argv[3]);

	pthread_t *pids = malloc(2 * nr_thread_pairs * sizeof(pthread_t));

	struct obj_queue *queues = calloc(nr_thread_pairs,
					  sizeof(struct obj_queue));

	for (i = 0; i < nr_thread_pairs; i++) {
		if (pthread_create(&pids[2 * i], NULL,
				   producer_routine,
				   (void *)&queues[i]) < 0)
			exit(EXIT_FAILURE);

		if (pthread_create(&pids[2 * i + 1], NULL,
				   consumer_routine,
				   (void *)&queues[i]) < 0)
			exit(EXIT_FAILURE);
	}

	for (i = 0; i < 2 * nr_thread_pairs; i++) {
		pthread_join(pids[i], NULL);
	}

	printf("finished!\n");
	return 0;
}
