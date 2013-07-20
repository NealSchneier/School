#include "uthreads.h"

#include <stdio.h>
#include <stdlib.h>

void 
uthreads_sem_init(uthreads_sem_t s, int initial)
{
	s->count = initial;
	
	list_init(&s->waiters);
}

void 
uthreads_sem_post(uthreads_sem_t s)
{
	if (s->count == 0 )
	{
		uthreads_t curr = uthreads_current();
	
		//printf("list %d count %d\n", list_size(&s->waiters), s->count);

		uthreads_unblock(curr);
		
		//list_pop_back(&s->waiters);
	}
	
	if (s->count > 0)
		s->count++;
	

}

void 
uthreads_sem_wait(uthreads_sem_t s)
{
	
	if (s->count > 0 )
	{
		s->count--;
		
	}
	else if (s->count <= 0)
	{
		//	printf("wait %d count %d\n", list_size(&s->waiters), s->count);
		uthreads_t curr = uthreads_current();
		list_push_back(&s->waiters, &curr->elem);
		uthreads_block();
		
	}

}
