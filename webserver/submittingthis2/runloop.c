char words[30];
time_t start, end;
time(&start);
char timeValue[200];
strcpy(words, "Running the current loop:" );
double doubleTime;
//Loop for 15 seconds.
while(difftime(end, start) <= 15.0){
	doubleTime = difftime(end, start);
	//printf("%0.21f\n", doubleTime);
	sprintf(timeValue, "%0.21f\n", doubleTime);
	//send(fd, timeValue, strlen(timeValue), 0);
	time(&end);
	memset(timeValue, '\0', sizeof(timeValue));
}
printf("Loop running\n");
