/*
 * This is our webserver. It uses threading where it detaches each thread and allows it to run
 and close on its own after execution. It uses mutexes to protect data during certain tasks.
 This relies heavily on file descriptor. It has 10 sockets. It waits for a connection request. Once it accepts
 a connection it creates a new thread which executes the echo function. That function parses
 the uri and performs the requested actions based on the uri request. It responds based on both http1.0 and
 http 1.1 requests with the correct headers. It correctly handles binary files and bad requests. For the binary
 files it will handle any of the files based on the mime types that i check for which includes but not limited to
 css, js, txt, html, jpg, and gif files. 
 
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h> 
#include "csapp.h"

#define PTHREAD_STACK_MIN 65535

//global variable for the allocated memory
void* allocatedMemory[1000];
int lastMemoryIndex = -1;
int numMap = 0;
char * path;
//the thread functions
void * echo(void * connfd);

//struct for threading
typedef struct {
   pthread_attr_t *attribute;
   pthread_mutex_t *mutex;
   int s;
} myStruct;


int main(int ac, char *av[])
{
	//initialize a lot of struct and data points needed
    struct addrinfo *in, *pin;
    struct addrinfo start;
    int sockets[10], socket_num = 0;	
	allocatedMemory[0] = 0;
	myStruct *arg;
	pthread_t dthread;
	pthread_mutex_t *mutex;
	//clear the struct data values
	memset(&start, 0, sizeof start);//clear the object and then check for the right number of arguments passed
	
    if (ac < 3) 
	{
		printf("Usage: %s -p <port>\n", av[0]), exit(0);
	}
	if (ac == 4)
	{
		printf("Usage: %s -p <port> -R <path>\n", av[0]), exit(0);
	}
	else if (ac == 5) 
	{
		path = av[4];
	}
	//set the flags for the correct type of server
    start.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG;
    start.ai_protocol = IPPROTO_TCP; // only interested in TCP
    start.ai_family = AF_INET6;
	
	if (strcmp(av[1],"-p"))
		return -1;
	//gets the port number
    char *nport = av[2];

    int gai = getaddrinfo(NULL, nport, &start, &in);
	
    if (gai != 0)
	{
        gai_strerror(gai);
		exit(-1);
	}

    char printed_addr[1024];
    for (pin = in; pin; pin = pin->ai_next) {
        assert (pin->ai_protocol == IPPROTO_TCP);
        int gai = getnameinfo(pin->ai_addr, pin->ai_addrlen,
                             printed_addr, sizeof printed_addr, NULL, 0,
                             NI_NUMERICHOST);
		
        if (gai != 0)
            gai_strerror(gai), exit(-1);

        printf("%s: %s\n", pin->ai_family == AF_INET ? "AF_INET" :
                           pin->ai_family == AF_INET6 ? "AF_INET6" : "?", 
                           printed_addr);

        int s = socket(pin->ai_family, pin->ai_socktype, pin->ai_protocol);
        if (s == -1)
            perror("socket"), exit(-1);

        int opt = 1;
        setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));

        gai = bind(s, pin->ai_addr, pin->ai_addrlen);
        if (gai == -1 && errno == EADDRINUSE) {
            // ignore Linux limitation
            close(s);
            continue;
        }

        if (gai == -1)
            perror("bind"), exit(-1);
		//begin listening on the socket
        gai = listen(s, 10);
        if (gai == -1)
            perror("listen"), exit(-1);

        assert(socket_num < sizeof(sockets)/sizeof(sockets[0]));
        sockets[socket_num++] = s;
    }
	freeaddrinfo(in);
    assert(socket_num == 1);
	//initialize mutex before accepting
    struct sockaddr_storage rem;
    socklen_t remlen = sizeof (rem);
	
	mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	while(1){
		//accept a connection
		arg = malloc(sizeof(myStruct));
		arg->mutex = mutex;
		//accept the connection
        arg->s = accept (sockets[0], (struct sockaddr *) &rem, &remlen);
		
		//if the connection works
        if (arg->s == -1)
		{
			pthread_mutex_lock(mutex);
            perror ("accept");
			pthread_mutex_unlock(mutex);
			free(arg);
			pthread_mutex_destroy(mutex);
			exit(-1);			
		}
		pthread_mutex_lock(mutex);

        char buffer[200];

		//get information about the connection
        if (getnameinfo ((struct sockaddr *) &rem, remlen, buffer, sizeof (buffer), NULL, 0, 0))
            strcpy (buffer, "???");   // hostname unknown

        char buf2[100];
        (void) getnameinfo ((struct sockaddr *) &rem, remlen, 
                buf2, sizeof (buf2), NULL, 0, NI_NUMERICHOST);
        printf ("connection from %s (%s)\n", buffer, buf2);
		
		
		//create the thread and call the echo functions
		//makes sure to lock and unlock and set the stack size
		pthread_mutex_unlock(mutex);
		arg->attribute = malloc(sizeof(pthread_attr_t));
		pthread_attr_init(arg->attribute);
		pthread_attr_setstacksize(arg->attribute, PTHREAD_STACK_MIN+8192);
		pthread_attr_setdetachstate(arg->attribute, PTHREAD_CREATE_DETACHED);
		pthread_create(&dthread, arg->attribute, echo, arg);
		
    }

    return 0;
}

/*
	This is the threaded function. It is called after a good connection has been made.
	It parses the header and deteremines the proper response that needs to be sent 
	from the server in the correct format. It responds in both 1.0 and 1.1 HTTP requests.
	

*/
void * echo(void * connfd)
{
		
	myStruct *arg = (myStruct *) connfd;//gets the argument from the void pointer
	int fd = arg->s;//gets the file descriptor
	//declare storage char arrays
	char method[1024], uri[8192], version[1024];
	char buf[8192];
	//clear the char arrays
	memset(buf, '\0', sizeof(buf));
	memset(method, '\0', sizeof(method));
	memset(uri, '\0', sizeof(uri));
	memset(version, '\0', sizeof(version));

	rio_t rio;
	//connection the rio the the fd and then read
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, 8192);
	//gets the methods, uri, version
	sscanf(buf, "%s %s %s", method, uri, version);
	//make sure it is a get request
	if (strcasecmp(method, "GET") == 0)
	{		
		//search for the file of the request
		FILE *file; 
		
		char filetype[100];
		char * type;
		int position = -1;
		int current = 100;
		int count = 4;
		//gets the extensio
		//get the file type in a very slow way
		for(current = 100; current != 0; current--)
		{
			if (uri[current] == '.' && position == -1)
			{
				position = current;
				break;
			}
			else 
				filetype[count] = uri[current];
			count++;
		}
		//get the correct mime type
		if (strstr(filetype, "html") == 0 || strstr(filetype, "htm") == 0)
			type = "text/html";
		else if (strstr(filetype, "jpg") == 0)
			type = "image/jpeg";
		else if (strstr(filetype, "js") == 0)
			type = "text/javascript";
		else if (strstr(filetype, "gif") == 0)
			type = "image/gif";
		else if (strstr(filetype, "txt") == 0)
			type = "text/plain";
		else if (strstr(filetype, "css") == 0)
			type = "text/css";
		else if (strstr(filetype, "ram") == 0 || strstr(filetype, "ra") == 0)
			type = "audio/x-pn-realaudio";
		else 
			type = "text/plain";
			
		char status[100];
		//format correctly
		if (strstr(uri, "HTTP/1.1") != NULL)	
			strcpy(status, "HTTP/1.1");
		else 
			strcpy(status, "HTTP/1.0");
			
		char body[8192];//used to store the body of the request in all but opening binary files
		
		//if it is a junk file in any way
		if (strstr(uri, "junk") != NULL)
		{
			//returns 404 not found error
			if (strcmp(version, "HTTP/1.0"))
				strcat(body, "HTTP/1.0 404 Not Found");
			else 
				strcat(body, "HTTP/1.1 404 Not Found");
			send(fd, body, strlen(body), 0);
		}
		//one of the checks
		else if (strstr(uri, "meminfo?callback=") != NULL)
		{
			//parses the request for the callback name and
			//then returns meminfo with the name formatting.
			char argument[100];
			char * position = strchr(uri, '=');
			position++;
			
			strcpy(argument, position);
			int y;
			int x = 0;
			for (y = 0; y < 100; y++)
				if((argument[y] == '&' || argument[y] =='_' || argument[y] == '=') && x != 1)
				{
					argument[y] = '\0';
					y = 100;
					x = 1;
				}
			strcat(body, argument);
			strcat(body, "(");
			#include "json_mem_info.c"
			strcat(body, ")");
			send(fd, body, strlen(body), 0);
		}
		//other check
		else if (strstr(uri, "/meminfo?") != NULL && strstr(uri, "&callback=") != NULL)
		{
			//parses the request for the callback name and
			//then returns meminfo with the name formatting.
			
			char argument[8192];
			char * position = strstr(uri, "&callback=");
			position += 10;
			strcpy(argument, position);
			int a;
			int n = 0;
			//checks for random ascii characters
			for (a = 0; a < 8192; a++)
			{
				if((argument[a] == '&' || argument[a] =='_' || argument[a] == '=') && n != 1)
				{
					argument[a] = '\0';
					a = 100;
					n = 1;	
				}
			}
			strcat(body, argument);
			strcat(body, "(");
			#include "json_mem_info.c" 
			strcat(body, ")");
			send(fd, body, strlen(body), 0);//sends the information
		}
		//default meminfo
		else if (strncmp(uri, "/meminfo", 8) == 0)
		{
			#include "json_mem_info.c"
			fclose(file);
			send(fd, body, strlen(body), 0);
		}		
		else if (strstr(uri, "loadavg?callback=") != NULL )
		{
			//the loadavg call, parses the uri to get the correct name
			char argument[8192];
			char * position = strchr(uri, '=');
			position++;
			strcpy(argument, position);
			int a;
			int n = 0;
			for (a = 0; a < 8192; a++)
			{
				if((argument[a] == '&' || argument[a] =='_' || argument[a] == '=') && n != 1)
				{
					argument[a] = '\0';
					a = 100;
					n = 1;
				}
			}
			strcat(body, argument);
			strcat(body, "(");
			#include "json_loadavg.c"
			strcat(body, ")");
			send(fd, body, strlen(body), 0);
		}
		else if (strstr(uri, "/loadavg?") != NULL && strstr(uri, "&callback=") != NULL)
		{
			//the most complicated loadvg callback
			char argument[8192];
			char * position = strstr(uri, "&callback=");
			position += 10;
			strcpy(argument, position);
			int a;
			int n = 0;
			for (a = 0; a < 8192; a++)
			{
				if((argument[a] == '&' || argument[a] =='_' || argument[a] == '=') && n != 1)
				{
					argument[a] = '\0';
					a = 100;
					n = 1;
					
				}
			}
			strcat(body, argument);
			strcat(body, "(");
			#include "json_loadavg.c"
			strcat(body, ")");
			send(fd, body, strlen(body), 0);
		}
		else if (strstr(uri, "/loadavg") != NULL)
		{
			//the basic loadavg
			//1.0 or 1.1
			if (strcmp(version, "HTTP/1.0") == 0)
			{
				strcat(body, "HTTP/1.0 200 OK\r\n\r\n");
			}
			#include "json_loadavg.c"
			fclose(file);

			send(fd, body, strlen(body), 0);
		}
		//this calls the runloop functionality
		else if (strstr(uri, "runloop") != NULL){
			#include "runloop.c"
		}
		//this calls the allocanon functionality
		else if (strstr(uri, "allocanon") != NULL){
			#include "allocanon.c"
		}
		//this calls the freeanon functionality
		else if (strstr(uri, "freeanon") != NULL){
			#include "freeanon.c"
		}
		else 
		{	
			//when the request is not a predifined request. Checks if the file 
			//exists based on the uri. If it does it reads it. If it doesnt
			//it returns 404 not found
			//check if its a file
			int p;
			for(p = 1; p < strlen(uri)+1; p++)
			{
				uri[p-1] = uri[p];
			}
			//the file
			if (path != NULL)
			{
				strcpy(uri, strcat(path, uri));
			}
			file = fopen(uri, "r");
			if (file != NULL)
			{
				//when the file exists it gets the proper header based on the content type and
				//length and the request
				char header[100];
				char length[100];
				char typeH[100];
				char * sent;
				
				strcat(header, version);
				strcat(header, " 200 OK\r\n");
				strcat(typeH, "Content-Type: ");
				strcat(typeH, type);
				strcat(typeH, "\r\n");
				
				strcat(length, "Content-Length: ");
				//this gets the files length and then reads the file and stores it
				fseek(file, 0, SEEK_END);
				int size = ftell(file);
				fseek(file, 0, SEEK_SET);
				sent = (char *) malloc(size);
				fread(sent, size, 1, file);

				strcat(length, "\r\n\r\n");
				//sends the header, content type, content header and the binary file
				send(fd, header, strlen(header), 0);
				send(fd, typeH, strlen(typeH), 0);
				send(fd, length, strlen(length), 0);
				send(fd, sent, size, 0);
				fclose(file);
			}
			else 
			{
				//when the file does not exist
				if (strcmp(version, "HTTP/1.0"))
					strcat(body, "HTTP/1.0 404 Not Found");
				else 
					strcat(body, "HTTP/1.1 404 Not Found");
				send(fd, body, strlen(body), 0);				
			}
		}
	}
	else 
	{
		//when the method is not recognized
		send(fd, "HTTP/1.1 405 Method Not Allowed", strlen("HTTP/1.1 405 Method Not Allowed"), 0);
	}
	close (fd);
	pthread_exit(NULL);
}





