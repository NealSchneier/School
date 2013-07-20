void *mm_realloc(void *oldptr, size_t size)
{
	//ptr is null return a mm_malloc call
	if (oldptr == NULL){
		return mm_malloc(size);
	}
	if (size == 0){
		mm_free(oldptr);
		return;
	}
	//op points to the place on the heap where the old allocated block is.
	void * op = oldptr;

	//Gets some of the old information
	op = op - sizeof(struct allocated_block_header); //Sets it to the header
	struct allocated_block_header oldHeader = (struct allocated_block_header *) op;
	size_t oldSize = oldHeader->size;

	if (oldSize == size){
		return; //The sizes are the same, no need to realloc
	}
	else if ( oldSize > size){
		//This block will need to be reallocated.
		void * newPointer = mm_malloc(size);
		oldptr = oldptr - sizeof(struct allocated_block_header); //Get the head
		struct allocated_block_header * oldHead = (struct allocated_block_header *) oldptr;
		size_t oldSize = oldHeader->size - sizeof(struct allocated_block_header)*2;
		oldptr = oldptr + sizeof(struct allocated_block_header); //Point back to the data.
		memcpy(newPointer, oldptr, oldSize);
		free(oldptr);
		return newPointer;
	}
	else{	//oldSize < size
		//This block will need to be reallocated.
                void * newPointer = mm_malloc(size);
                oldptr = oldptr - sizeof(struct allocated_block_header); //Get the head
                struct allocated_block_header * oldHead = (struct allocated_block_header *) oldptr;
                size_t oldSize = oldHeader->size - sizeof(struct allocated_block_header)*2;
                oldptr = oldptr + sizeof(struct allocated_block_header); //Point back to the data.
                memcpy(newPointer, oldptr, oldSize);
                free(oldptr);
		return newPointer;
	}
	

	/*
    if (newptr == NULL)
      return NULL;

    /* Assuming 'oldptr' was a '&payload[0]' in an allocated_block_header,
     * determine its start as 'oldblk'.  Then its size can be accessed
     * more easily.
     */
	/*
    struct allocated_block_header *oldblk;
    oldblk = oldptr - offsetof(struct allocated_block_header, payload);

    size_t copySize = oldblk->size;
    if (size < copySize)
      copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;*/
}

