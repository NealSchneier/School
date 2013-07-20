#include <sys/mman.h>
char *p = malloc(67108864);
if (p == NULL){
	numMap++;
	//This needs to mmap it.
	p = mmap(NULL, 67108864, PROT_NONE, MAP_SHARED |MAP_ANONYMOUS, -1, 0); 	
}else{
	p = memset(p, '0', 67108864);
}

int i;
//Assigns a place in the array to the pointer
for (i = 0; i < 1000; i++){
	if (allocatedMemory[i] == NULL){
		break;
	}
}
lastMemoryIndex++;
allocatedMemory[i] =  p;
