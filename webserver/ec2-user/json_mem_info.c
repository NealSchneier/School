//Opens the file.
file = fopen("/proc/meminfo", "r");
if (file == NULL){
	printf("File not found");
}

//Start of the json format	
strcpy(body, strcat(body, "{"));
int flag = 1;
while (!feof(file)){
	
	//char arrays to contain the strings
	char first[30];
	char second[30];
	char third[30];

	//clears whatever was in it before to make sure that there aren't constant writes

//	memset(first, '\0', sizeof(first));
	memset(second, '\0', sizeof(second));
	memset(third, '\0', sizeof(third));
	if (strcmp(first, "kB") == 0 || flag){
		memset(first,'\0', sizeof(first));
		fscanf(file, "%s", first);
		flag = 0;
		//This is the space where there arent any data elemen
	}
	fscanf(file, "%s", second);
	
	//Loops to split up the words
	int x = 0;
	for( x = 0; x < 30; x++){
		if (first[x] == ':'){
			break;
		}
		third[x] = first[x];
	}
	
	strcpy(body, strcat(body, "\""));
	strcpy(body, strcat(body, third));
	strcpy(body, strcat(body, "\": "));
	
	int y = 0;
	for (y =0; y < 30; y++){
		if (second[y] == '\0'){
			break;
		}
	}
	
	strcpy(body, strcat(body, "\""));
	strcpy(body, strcat(body, second));
	strcpy(body, strcat(body, "\""));
	
	fscanf(file, "%s", first);
	//This if is to stop at the end of all the objects
	if (!feof(file)){
		strcat(body, ", ");
	}
}
strcpy(body, strcat(body, "}"));
//End of the json format.
