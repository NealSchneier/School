/*
 * This explicit implementation of memory is fairly basic. It simply uses and explicit list
 * to store information using a first fit method of the list. There are no advanced searches or
 * best fit calculations that attempt to limit wasted space. It simply finds the first fit. 
 * Most of the macros were used from the textbook example but some sizes were changed. 
 
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
	/* Team name */
	"neal89+mchao8",
	/* First member's full name */
	"Neal Schneier",
	/* First member's NYU NetID*/
	"neal89@vt.edu",
	/* Second member's full name (leave blank if none) */
	"Michael Chao",
	/* Second member's email address (leave blank if none) */
	"mchao8@vt.edu"
};


/* Basic constants and macros */
#define WSIZE 4       /* Word and header/footer size (bytes) */ 
#define DSIZE 8       /* Doubleword size (bytes) */
 

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(int *)(p))           
#define PUT(p, val)  (*(int *)(p) = (val)) 

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                  
#define GET_ALLOC(p) (GET(p) & 0x1)             

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) (bp - WSIZE)
#define FTRP(bp) (bp + GET_SIZE(HDRP(bp)))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((bp) + GET_SIZE(HDRP(bp)) + 2*WSIZE)
#define PREV_BLKP(bp) (bp - GET_SIZE(bp-DSIZE) - 2*WSIZE)

#define GET_NEXT(bp)		(*(void **)(bp + DSIZE))
#define GET_PREV(bp)		(*(void **)bp)
#define SET_NEXT(bp, ptr)	(GET_NEXT(bp) = ptr)
#define SET_PREV(bp, ptr)	(GET_PREV(bp) = ptr)

#define ALIGN(p) (((size_t)(p) + (7)) & ~(0x7))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

//global head of list
void* listHead = NULL; 


static void coalesce(void *ptr);


/*

 * Create the list for the free elements and grab 8 bytes from memory for the pointers for the
 * head of the list. Make header and footer for the 8 bytes.
 */
int mm_init(void) {

	void * heap_bottom;//create void *
	listHead = NULL;//create empty list for the free elements
	heap_bottom = mem_sbrk(DSIZE);//get pointer to the start of the heap that has just been freed
	if (heap_bottom == (int * ) -1)//check system call
		return -1;
		
	PUT(heap_bottom, PACK(0, 1));//place header
	PUT(heap_bottom + WSIZE, 0); //place footer
	
	return 0;
}



/* 
 * Uses and explicit list implementation. So first it checks the sizes and adjusts it if necessary. 
 * The next step is to search the list and see if there is an item in the list that is large enough for the 
 * aligned size. If so it inserts and creates a free block if there is enough space. 
 * Finally if there is not enough space it expands and puts the element on the end. 
 */
void *mm_malloc(size_t size) {	
	if (size == 0)//check to make sure there is something to write.
		return NULL;
	
	void *list = listHead; //internal use of the first list head
	int aligned;//the size of the element after it has been aligned.
	int tempsize = 0;//used in the loop to temp store size of the current element that the loop is checking
	
	if (size <= 3 * DSIZE) //make sure that the minimum size is reached
		aligned = 3 * DSIZE;
	else //if not the minimum size then make sure that the size is aligned correctly.
		aligned = ALIGN(size);
	
	//search through the free list trying to find an element that can store the aligned size
	while (list != NULL) 
	{
		tempsize = GET_SIZE(HDRP(list));//get the size of the current element
		if (tempsize >= aligned) //if the size fits
		{
			if (tempsize >= aligned + 24) //if the size of the block found is larg enough that it needs to be split.
			{
				int newFree = GET_SIZE(HDRP(list)) - aligned - DSIZE;//the new size of the free block
				//the following places the headers and footers for the new free block 
				PUT(HDRP(list), PACK(newFree, 0));
				PUT(FTRP(list), PACK(newFree, 0));
				void *p = NEXT_BLKP(list);
				PUT(HDRP(p), PACK(aligned, 1));
				PUT(FTRP(p), PACK(aligned, 1));
				return p;
			}			
			//get next and previous elements in the list based on current location
			void *next = GET_NEXT(list);
			void *prev = GET_PREV(list);
			
			//if the element is at the front
			if (prev == NULL) 
			{
				listHead = next;
				if (next != NULL) //is condition that needs to be handled
					SET_PREV(next, NULL);
			} 
			else 
			{
				//if the element is not at the front
				SET_NEXT(prev, next);
				if (next != NULL)
					SET_PREV(next, prev);
			}
			//put the headers and footer for element
			PUT(HDRP(list), PACK(tempsize, 1));
			PUT(FTRP(list), PACK(tempsize, 1));
			return list;
		} 
		else 
			list = GET_NEXT(list); //get the next element if the size doesnt fit.
		
	}
	
	//if now room is found on the list then add memory and check the system call
	if((int)(list = mem_sbrk(aligned + DSIZE)) == -1)
		return NULL;
	
	//for the newly allocated memory add headers and footers and pointers.
	PUT(HDRP(list), PACK(aligned, 1));
	PUT(FTRP(list), PACK(aligned, 1)); 
	PUT(FTRP(list) + WSIZE, PACK(0, 1)); 
	return list;
}


/*	
 * This is done to fix space and list size. If there are free blocks that are located next to each other 
 * Then they are put together so to cut down on the time it takes to insert new elements.
 */
static void coalesce(void *ptr) 
{
	//gets the allocation of the previous, current and next to determine whether they are free
	int next_alloc = GET_ALLOC(FTRP(ptr) + WSIZE);
	int prev_alloc = GET_ALLOC(ptr - DSIZE);
	int size = GET_SIZE(HDRP(ptr));

	//this is the easiet, when next and previous are full
	if (prev_alloc && next_alloc) 
	{
		void *head = listHead;
		SET_NEXT(ptr, head);
		SET_PREV(ptr, NULL);
		if (head != NULL)
			SET_PREV(head, ptr);
		listHead = ptr;
	} 
	//when the next one is free but previous is full
	else if (prev_alloc && !next_alloc) 
	{  
		size = size + GET_SIZE(HDRP(NEXT_BLKP(ptr))) + DSIZE;
		void *next = GET_NEXT(NEXT_BLKP(ptr));
		void *prev = GET_PREV(NEXT_BLKP(ptr));
		
		//sets the pointers
		if (prev == NULL) 
		{
			listHead = next;
			if (next != NULL) 
				SET_PREV(next, NULL);
		} 
		else 
		{
			SET_NEXT(prev, next);
			if (next != NULL)
				SET_PREV(next, prev);
		}
		//corrects the sizes
		PUT(HDRP(ptr), PACK(size, 0));
		PUT(FTRP(ptr), PACK(size, 0));
		
		
		void *head = listHead;
		SET_NEXT(ptr, head);
		SET_PREV(ptr, NULL);
		if (head != NULL)
			SET_PREV(head, ptr);
		listHead = ptr;
	} 
	//when prev is empty and next if full
	else if (!prev_alloc && next_alloc) 
	{
		ptr = PREV_BLKP(ptr);
		size = size + GET_SIZE(HDRP(ptr)) + DSIZE;
		PUT(HDRP(ptr), PACK(size, 0));
		PUT(FTRP(ptr), PACK(size, 0));
	} 
	//when it is in the center and both next and prev are free
	else 
	{ 
		void * prev = PREV_BLKP(ptr);
		void * next = NEXT_BLKP(ptr);		
		size += GET_SIZE(HDRP(prev)) + GET_SIZE(HDRP(next)) + 2*DSIZE;
		PUT(HDRP(prev), PACK(size, 0));
		PUT(FTRP(prev), PACK(size, 0));					
		void *nextP = GET_NEXT(next);
		void *prevP = GET_PREV(next);
		if (prevP != NULL) 
		{
			SET_NEXT(prevP, nextP);
			if (nextP != NULL)
				SET_PREV(nextP, prevP);
		} 
		else 
		{
			listHead = nextP;
			if (nextP != NULL) 
				SET_PREV(nextP, NULL);
		}
	}
}



/*

 * Frees the given block. It does so by changing the headers and 
 * adding it to the free list. combines neighboring blocks when necessary.
 */
void mm_free(void *ptr){
	//make sure there is something to free
	if(ptr == 0)
		return;
	//get size after sure there is something to free
	size_t size = GET_SIZE(HDRP(ptr));
	//change the headers footers
	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	//fix the arrangement if necessary
	coalesce(ptr);	
	//fix the pointers if list isnt empty
	if (listHead == NULL)
	{
		void *head = listHead;
		SET_NEXT(ptr, head);
		SET_PREV(ptr, NULL);
		if (head != NULL)
			SET_PREV(head, ptr);
		listHead = ptr;
	}
}

/*
 * the same basic realloc that was provided. relies on mm_free and mm_malloc to functions
 * which makes the run time slow. 
 */
void *mm_realloc(void *ptr, size_t size) {
    size_t oldsize;
    void *newptr;

    // If size == 0 then call mm_free, and return NULL
    if (size == 0) {
		mm_free(ptr);
		return 0;
    }

    // If oldptr is NULL, then this is just malloc
    if (ptr == NULL) {
		return mm_malloc(size);
    }

 	newptr = mm_malloc(size);

    // If realloc() fails the original block is left untouched
    if (!newptr) {
		return 0;
    }

    // Copy the old data. 
    oldsize = GET_SIZE(HDRP(ptr));
    if (size < oldsize) 
		oldsize = size;
    memcpy(newptr, ptr, oldsize);

    // Free the old block
    mm_free(ptr);

    return newptr;
}
//not necessary for minimum implementation.
void mm_check() {
	//first thing to do is check the header of list
	//next, check the header of the first element, make sure the footer matches
}

