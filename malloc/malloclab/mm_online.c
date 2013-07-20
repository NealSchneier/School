/*
 * mm-malloc.c - (almost) The fastest, least memory-efficient malloc package.
 *
 * > Type of allocator: segregated free lists
 *
 * > structure of lists/pointers: unidirectional. A list of size 48b is
 *   initiated in mm_init() to hold a maximum of 12 4byte ptrs to free list. 
 *   Each ptr in this list (the initial 48 bytes of the heap) points either
 *   to a free list of identically sized blocks or to NULL if the free list
 *   is empty.
 *
 *   Each free list contains only free blocks all of the same size. The 1st
 *   4 bytes indicate the size of the list, the following 4 bytes a pointer
 *   to the next free block or '0' if the block is the last in the list.
 *   These 8 blocks comprise the header and the payload is equal to the
 *   size storred by the header. So the block in practice is the size read
 *   from the 1st 4 bytes of the block + 8 since the size does not take
 *   into account the size of the header.
 *   
 * > size of blocks (minimum size? rounding?): All blocks are aligned to an
 *   8 byte boundry. Block sizes are predetermined at runtime and can be found
 *   in the mm_malloc function and mm_free function. Sizes of blocks are (in 
 *   bytes and minus 8byte header): 128, 256, 512, 1024, 2048,
 *   4096, 8192, 16384, 32768, 65536 and 614784. For the task
 *   no larger block size is needed by this malloc implementation.
 *
 * > size of any block header/footer: 8 bytes (header only).
 *
 * > structure of block header: as above. 4b blocksize and 4b pointer
 *
 * > how blocks are chosen by malloc(): malloc chooses the first block on the
 *   free list that is large enough to hold what malloc needs to return. If
 *   no blocks are available a new one is created through mem_sbrk().
 *
 * > how blocks are freed: Freed blocks go on the front of the free list
 *   for blocks of size N (N is the size of the pointer being freed determined
 *   by the ptr's header). If the list is currently empty the ptr being
 *   freed will have its direction pointer set to '0' otherwise a ptr to the
 *   next block on the list is placed. In either case the sizes list will be
 *   updated to point to the new head of the corresponding free list. *where
 *   direction bytes refers to the 2nd lot of 4 bytes in the header appointed
 *   for pointer placement.
 *
 * > splitting: unsupported.
 *
 * > coalescing: unsupported
 *
 * > realloc(): realloc checks to see if the current block is greater than the
 *   new size. If so the same block is returned (which may result in internal
 *   fragmentation for a gain in performance), otherwise malloc is called to
 *   get a new block of size and that is returned and the old block is freed.
 *   If malloc() fails NULL is returned. 
 * 
 *
 *
 *
 *
 *   Empty list will have the corresponding position in the "pointers to list"
 *   section as '0'.
 *
 *   size List (index numbers)
 *   1    2    3    4    5   6    7    8         14
 *   ____ ____ ____ ____ ____ ____ ____ ____      ____ 
 *  |_P__|_0__|_0__|_P__|_P__|_0__|_0__|_0__|....|_0__|
 *    |             |      |
 *    \             \      \
 *   ___ Free List  ___   ___  
 *  |s+0|   ||     |s+p| |s+p|
 *  |___|   \\_\   |   | |   |
 *             /   |   | |   |
 *                 |_ _| |   |
 *   Last block => |s+0| |___|
 *   on List       |   | |s+0|
 *                 |   | |   |
 *                 |___| |   |
 *                       |   |
 *                       |___|  
 *
 *           s = size, P = Pointer   
*/



#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "mm.h"
#include "memlib.h"

/*The following Macros were copied from p746 of Computer Systems:A Programmers
 * Perspective, Randal Bryant and David O'Hallaron 2003.
 */

/* Read and write a word at address p */
#define GET(p)       (*(int *)(p))
#define PUT(p, val)  (*(int *)(p) = (val))  

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((int *)(bp) - 2)  


/* Ptr to start of the heap */
static int* list;

/* Necessary Function Declarations */
int mm_check(void);

team_t team = {
    /* Team name */
    "group114",
    /* First member's full name */
    "Neal Schneier",
    /* First member's SLO (@cs.vt.edu) email address */
    "neal89",
    /* Second member's full name (leave blank if none) */
    "Michael Chao",
    /* Second member's SLO (@cs.vt.edu) email address (leave blank if none) */
    "mchao8"
};



/* 
 * mm_init - initialize the malloc package.
 */

int mm_init(void) {

    if ((list = mem_sbrk(48)) == (void*) - 1) {
	return -1;
    } 
    
    PUT (list + 0, 0);   /*128*/
    PUT (list + 1, 0);   /*256*/
    PUT (list + 2, 0);   /*512*/
    PUT (list + 3, 0);   /*1024*/
    PUT (list + 4, 0);   /*2048*/ 
    PUT (list + 5, 0);   /*4096*/
    PUT (list + 6, 0);   /*8192*/
    PUT (list + 7, 0);   /*16384*/
    PUT (list + 8, 0);   /*32768*/
    PUT (list + 9, 0);   /*65536*/
    PUT (list + 10, 0);  /*614784*/

    return 0;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */

void *mm_malloc(size_t size) {
    int index; 
    int* newPtr;
    
    if (size <= 128) {
	index = 0;
	size = 128;
    }
    
    else if (size <= 256) {
        index = 1;
	size = 256;
    }
   
    else if (size <= 512) {
        index = 2;
	size = 512;
    }
    
    else if (size <= 1024) {
        index = 3;
	size = 1024;
    }
    
    else if (size <= 2048) {
	index = 4;  
	size = 2048;      
    }
    
    else if (size <= 4096) {
        index = 5;
	size = 4096;
    }
    
    else if (size <= 8192) {
        index = 6;
	size = 8192;
    }
    
    else if (size <= 16384) {
        index = 7;
	size = 16384;
    }
    
    else if (size <= 32768) {
        index = 8;
	size = 32768;
    }
    
    else if (size <= 65536) {
        index = 9;
	size = 65536;
    }
    
    else if (size <= 614784) {
        index = 10;
        size = 614784;
    } 
    
    else {
	return NULL;
    }    

    /* check if the selected free list is empty, if so get a new block
     * otherwise reuse a free one
     */
    
    //(int**) 
	newPtr = (list + index);

    if (!newPtr) {
	if ((newPtr = mem_sbrk(size + 8)) == (void*) - 1) {
	    return NULL;
	}

	PUT(newPtr, size);
	
    } else {
 
	/* set free list to point to the 2nd block on the list. (or null).
	 * In effect take a block off the freelist */

	*((int**) list + index) =  (int*)*(newPtr + 1);
    }

    /* return pointer to payload */
    
    return (void*)((int*) newPtr + 2); 

}


/*
 * mm_free - Freeing a block by placing it on the free list.
 */

void mm_free(void *ptr) {
    int index = -1;        
    int* listPtr;

    /* check the size and get the index of the free list */
    switch (GET_SIZE(HDRP(ptr))) {   

    case 128:
	index = 0;
	break;
	
    case 256:
	index = 1;
	break;
	
    case 512:
	index = 2;
	break;
	
    case 1024:
	index = 3;
	break;
	
    case 2048:
	index = 4;
	break;
	
    case 4096: 
	index = 5; 
	break; 
	
    case 8192: 
	index = 6;
	break; 
	
    case 16384:
	index = 7;
	break;	
	
    case 32768:
 	index = 8;
	break;	
	
    case 65536:
	index = 9;
	break;
	
    case 614784:
	index = 10;
	break;
    }
    
    /* if the list is empty make sure 1st block in it is 
     * pointing to 0, otherwise
     * point the pointer to what was previously 1st in the list*/
    
    listPtr = (list + index);    
    
    if (! *listPtr) {
	*((int**)ptr - 1) = 0;

    } else {
	*((int*) ptr - 1) = *listPtr;
    }
    
    /*point list to newly freed block*/
    
    *((int**) listPtr) = (int*) ptr - 2;
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 * returns the same block if the realloc call request a smaller
 * block. If new block is larger malloc and free are executed.
 */

void *mm_realloc(void *ptr, size_t size)
{
    
    void *oldPtr = ptr;
    void *newPtr;
    size_t copySize;
    
    
    copySize = *(size_t *)((int *) oldPtr - 2);
    if (size > copySize) {
	
	copySize = size;
	newPtr = mm_malloc(size);
	
	if (newPtr == NULL) {
	    return NULL;
	}
	
	memcpy(newPtr, oldPtr, copySize);
	mm_free(oldPtr);
	return newPtr;   
	
    } else {
	return oldPtr;
    }
}


/* mm_check_size (void* ptr)
 * Given a ptr to a block, return the size of the block if the size is
 * a valid value, otherwise return -1. Sizes are the same as those
 * in mm_init.
 */

int mm_check_size(void* ptr) {
    
    switch (*((int*) ptr)) {
	
    case 128:
	return 128;
	break;

    case 256:
	return 256;
        break;

    case 512:
        return 512;
        break;

    case 1024:
        return 1024;
        break;
    
    case 2048:
        return 2048;
        break;
    
    case 4096:
        return 4096;
        break;
    
    case 8192:
        return 8192;
        break;

    case 16384:
        return 16384;
        break;

    case 32768:
        return 32768;
        break;

    case 65536:
        return 65536;
        break;

    case 614784:
        return 614784;
        break;

    default:
	return -1;
    }
}

/* mm_get_block_size (int index)
 * Given an integer index, return the size of the bloc for the freelist
 * at position list+i (see mm_init). If an invalid index is supplied
 * return -1.
 */

int mm_get_block_size (int index) {
    switch (index) {
	
    case 0:
        return 128;
	
    case 1:
        return 256;

    case 2:
		return 512;

    case 3:
        return 1024;

    case 4:
        return 2048;

    case 5:
        return 4096;

    case 6:
        return 8192;

    case 7:
        return 16384;

    case 8:
        return 32768;

    case 9:
        return 65536;

    case 10:
        return 614784;

    default:
	return -1;
    }
}

   
/*
 * mm_check - Check the heap and free blocks. Exits if a problem is found
 * with a non 0 status. To run the mm_check place a call to the function
 * in mm_free, mm_realloc or mm_malloc. Be aware that mm_check assumes
 * that at least one block of memory has been allocated to the heap besides
 * that created in mm_init so it will only operate for mm_malloc AFTER
 * mem_sbrk has been called. If it is placed before it will prematurely
 * assume your heap's structure is incorrect or may not function at all.
 *
 * Test Run:
 * Test On free List:
 * > Ensure block is alighed to 8 bytes
 * > Ensure Size is valid. THis also ensure entegrity of header since no
 *   code updates size after it's being set in mm_malloc.
 * > Ensure block is on the right list by comparing size.
 * Test On overall heap
 * > Block aligned to 8 bytes
 * > Block has a valid value for size and thus entegrity of block structure
 *   (not necessarily contents) is maintained.
 *
 * Some test repeat between free list and heap so that issues can be 
 * isolated to the free or allocated memory.
 */

int mm_check(void) {
    int i = 0;
    int blockSize;
    int currentSize;
    int* ptr;
    int scan =1;    
    
    /* test on the free list */
    
    while (i < 11) {
	blockSize = mm_get_block_size(i);	
	if (*(list + i) != 0) {
	    ptr = (list + i);
	    
	    /* while the current list has blocks left */

	    while (ptr) {		

		/* check block alignment */

		if (((int) ptr) % 2) {
		    printf ("Warning: block starting at %d is not aligned to "
			    "an 8 byte boundary.(on free list)\n", (int) ptr);
		    exit(1);
		}

		/* check header contains an into of valid size */
		
		currentSize = mm_check_size ((void*) ptr);		
		if (currentSize == -1) {
		    printf ("Warning: address starting at %d does not contain a valid header. Incorrect size or not a size.", (int) ptr);
			printf(" (on free list)\n");
                    exit(1);
                }
		
		/* check that not only is the header an int of a correct size
		 * but that it is also the correct size for this free list */

		if (currentSize != blockSize) {
                    printf ("Warning: block starting at %d does not belong "
                            "on the free list. Incorrect blocksize. "
			    "Should be %d but is %d.\n", (int) ptr, blockSize,
			    currentSize);
                    exit(1);
                }
		 ptr = (ptr + 1);		
	    }
	}

	/* Move to next free list */
	i++;
    } 

    /* Test on the entire heap */
     ptr = (list + 12);

    while (scan) {

	/* Check byte alignment */
	if (((int) ptr) % 2) {
	    printf ("Warning: block starting at %d is not aligned to"
		    " an 8 byte boundary. (in heap)\n", (int) ptr);
	    exit(1);
	}
	
	/* make sure block is an integer of valid size */

	if (mm_check_size((void*) ptr) == -1) {
	    printf ("Warning: block starting at %d does not have a valid"
		    " header. Incorrect blocksize and was possibly"
		    " caused by memory being overwritten."
		    " (in heap)", (int) ptr);
	    exit(1);
	}

	/* continue searching if the end of the heap hasn't been reached */
	if (((int) ptr + *ptr + 8) < ((int)((int*) mem_heap_hi()))) {
	    (ptr) = (int*)ptr + (*ptr / 4) + 2;
      	} else {
	    scan = 0;
	}	
    }
    return 1;
}
	    




