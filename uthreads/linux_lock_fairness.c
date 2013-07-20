#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

pthread_mutex_t lock;
static volatile int shutdown;

static void *
contender(void *_counter)
{
    long volatile * counter = _counter;

    while (!shutdown) {
        pthread_mutex_lock(&lock);
        counter[0]++;
        pthread_mutex_unlock(&lock);
		sched_yield();
    }
	return NULL;
}

int
main(int ac, char *av[])
{
    int i, N = ac < 2 ? 2 : atoi(av[1]);
    long threadngotlock[N];
    pthread_t t[N];

    lock = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
   //pthread_mutex_init(&lock, NULL);

    for (i = 0; i < N; i++) {
        //t[i] = pthread_create(&t[i], NULL, contender, threadngotlock + i);
		pthread_create(&t[i], NULL, contender, threadngotlock + i);
    }

    time_t start = time(NULL);
    // spin until next second
    while (time(NULL) == start)
        sched_yield();

    start = time(NULL);
    printf("Created %d threads, time = %ld\n", N, start);
    for (i = 0; i < N; i++)
        threadngotlock[i] = 0;

    while (time(NULL) - start < 3)
        sched_yield();

    printf("Took %ld seconds\n", time(NULL) - start);
    for (i = 0; i < N; i++)
        printf("Thread %d acquired lock %ld many times\n", i, threadngotlock[i]);

    shutdown = 1;

    for (i = 0; i < N; i++)
        pthread_join(t[i], NULL);

    return 0;
}
