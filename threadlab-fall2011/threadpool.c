/**
 * threadpool.c
 * This describes the struct of future and thread_pool. 
 * Uses a semaphore, mutex, and conditional variables to lock
 * the implementation when necessary to protect the system. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "threadpool.h"

static void *doWork(void *ptr);

typedef struct future{
	void * data;//callable_data
	void * result;//callable_result
	thread_pool_callable_func_t callable_function;
	sem_t function_status;
	struct future* next;//pointer to the next future
} future;

typedef struct thread_pool {
	int numThreads;	//number of threads
	int size;			//size of queue
	pthread_t *threads;	//pointer to threads
	future* head;		//pointer to head
	future* tail;		//pointer to tail
	pthread_mutex_t lock;		//lock on list
	pthread_cond_t cond; //condition variable
	int shutdown; //whether it is shutdown
	int accept;//whether it accepts
} thread_pool;

/* Create a new thread pool with n threads. 
   should be started eagerly. 
   use pthread_cond_wait to wait for its children to finish
   if future is enqueue need function - remove it, execute it, store the result, signal completion to future
 */
struct thread_pool * thread_pool_new(int nthreads)
{
	if ((nthreads <= 0) )
		return NULL;
	thread_pool *pool = (thread_pool *) malloc(sizeof(thread_pool));//store pool
	int i;	
	if (pool == NULL) {
		fprintf(stderr, "malloc error\n");
		return NULL;
	}
	pool->threads = (pthread_t*) malloc (nthreads * sizeof(pthread_t));//store threads
	if(pool->threads == NULL) {
		fprintf(stderr, "malloc pthread_t error\n");
		return NULL;	
	}
	//set the pool structure values
	pool->numThreads = nthreads; 
	pool->size = 0;
	pool->head = NULL;
	pool->tail = NULL;
	pool->shutdown = 0;
	pool->accept = 0;

  //initialize mutex and condition variables.  
	if(pthread_mutex_init(&pool->lock,NULL)) {
		fprintf(stderr, "Mutex error!\n");
		return NULL;
	}
	if(pthread_cond_init(&(pool->cond),NULL)) {
		fprintf(stderr, "condition variable error!\n");	
		return NULL;
	}
	//creates the threads
	for (i = 0;i < nthreads;i++) {
		if(pthread_create(&(pool->threads[i]) , NULL, doWork, pool)) {
		
			fprintf(stderr, "pthread_create error!\n");	
			return NULL;		
		}
	}
	return pool;
}

/* Shutdown this thread pool.  May or may not execute already queued tasks. 
	shuts down the thread pool. if a futures is already executing let it complete. queued futures may or may not complete.
	set flag in the thread pool instance then use pthread_cond_broadcast to notify all worker threads of impending shutdown.
	must join all threads before returning.
	do not use pthread_cancel
*/
void thread_pool_shutdown(struct thread_pool * pool)
{
	//does the necessary needs to shutdown a thread and then joins them together.
	pthread_mutex_lock(&(pool->lock));
	pool->shutdown = 1;
	pthread_cond_broadcast(&(pool->cond));
	pthread_mutex_unlock(&(pool->lock));
	int i;
	for (i = 0; i < pool->numThreads; i++)
		pthread_join((pool->threads[i]), NULL);
}


/* Submit a callable to thread pool and return future.
 allocate a new future, initialize a semaphone, add the future to the end of work queue and notify
	one worker via pthread_cond_signal
 */
struct future * thread_pool_submit(struct thread_pool * pool, thread_pool_callable_func_t callable, 
        void * callable_data)		
{
	future * cur = (future*) malloc(sizeof(future));//make the current future
	if(cur == NULL) {
		fprintf(stderr, "malloc error future\n");
		return NULL;	
	}
	//store the parameters in the new future
	cur->callable_function = callable;
	cur->data = callable_data;
	sem_init(&(cur->function_status), 0, 0);//initialize the semaphone
	pthread_mutex_lock(&(pool->lock));//lock the mutex
	//corrects the pointers for the head and tail based on position
	if (pool->head == NULL)
	{
		pool->head = pool->tail = cur;
	}
	else 
	{
		pool->tail->next = cur;
		pool->tail = cur;
	}

	pthread_cond_signal(&(pool->cond));
	pool->size++;
	pthread_mutex_unlock(&(pool->lock));
	return pool->tail;
}

/* Make sure that thread pool has completed executing this callable,
 * then return result. 
 * wait for the future's semaphone to be signaled, return the futures result, do not deallocate
 */
void * future_get(struct future * future)
{

	sem_wait(&(future->function_status));
	return future->result;
}


/*
frees the memory for future instance, is called by the client. do not call within.
*/
void future_free(struct future * future)
{
	free(future);
}

/*
	This is the function that the thread it self calls to actually do the work of the threads
*/
static void *doWork(void *ptr)
{
	thread_pool * pool = (thread_pool *) ptr;//adjust the void pointer
	future* cur;	//The q element
	while(1) 
	{
		pthread_mutex_lock(&(pool->lock));  //get the lock.
		//whiel the head is null
		while(pool->head == NULL)
		{
			if (pool->shutdown)
			{
				pthread_mutex_unlock(&(pool->lock));
				pthread_exit(NULL);
			}
			pthread_cond_wait(&(pool->cond), &(pool->lock));
		}
		//set the pointers
		cur = pool->head;
		pool->head = cur->next;
		pthread_mutex_unlock(&(pool->lock));
		//the pool isnt shut down
		if (pool->shutdown != 1)
		{
			cur->result = (cur->callable_function)(cur->data);
			sem_post(&(cur->function_status));
		}	
	}
	return NULL;
}

