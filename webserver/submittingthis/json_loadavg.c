//Opens the file
file = fopen("/proc/loadavg", "r");

if (file == NULL){
	fprintf(stderr, "There was an error opening the file");
}
//Arrays to take the strings
char first[10];
char second[10];
char third[10];
char fourth[10];

//Scans the file and stores the strings
fscanf(file, "%s", first);
fscanf(file, "%s", second);
fscanf(file, "%s", third);
fscanf(file, "%s", fourth);

//Starts a JSON formatted string
strcat(body, "{\"total_threads\": \"");
int x = 0;
int y = 0;
int z = 0;

//Two strings to split up the thread division.
//number of threads
char fourth1[10];
//Total threads
char fourth2[10];

//Loops to format the strings
for (x = 0; x < 10; x++){
	if (fourth[x] == '/'){
		break;
	}
	fourth1[x] = fourth[x];
}
x++;
for (y = x; y < 10; y ++){
	fourth2[z] = fourth[y];
	z++;
}

//Adds all the string back to the given array to be sent to the server.
strcat(body, fourth2);
strcat(body, "\", \"loadavg\": [\"");
strcat(body, first);
strcat(body, "\", \"");
strcat(body, second);
strcat(body, "\", \"");
strcat(body, third);
strcat(body, "\"], \"running_threads\": \"");
strcat(body, fourth1);
strcat(body, "\"}");

