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
char write[40];

int numIndex = 0;
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
Rio_writen(fd, write, 18 + numIndex);
allocatedMemory[i] =  p;
