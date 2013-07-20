#include <sys/mman.h>
if (lastMemoryIndex != -1){
	if (numMap == 0){
		free(allocatedMemory[lastMemoryIndex]);
	}
	else{
		numMap--;
		munmap(allocatedMemory[lastMemoryIndex], 67108864);
	}
	
	lastMemoryIndex--;
}
