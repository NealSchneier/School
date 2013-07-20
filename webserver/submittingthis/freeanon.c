#include <sys/mman.h>
char write[40];
int numIndex = 0;

if (lastMemoryIndex != -1){
	if (numMap == 0){
		free(allocatedMemory[lastMemoryIndex]);
	}
	else{
		numMap--;
		munmap(allocatedMemory[lastMemoryIndex], 67108864);
	}
	
	lastMemoryIndex--;
	if (lastMemoryIndex < 10){
		numIndex = 1;
	}
	else if(lastMemoryIndex < 100){
		numIndex = 2;
	}
	else{
		numIndex = 3;
	}
	strcpy(write, "Number of blocks: ");
	char num[10];
	sprintf(num, "%d", lastMemoryIndex + 1);
	strcat(write, num);
	Rio_writen(fd, write, 18+numIndex);
}else if (lastMemoryIndex == -1){
	strcpy(write, "Number of blocks; 0");
	Rio_writen(fd, write, 19);
}


