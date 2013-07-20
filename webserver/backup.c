/*
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


#define BUFSIZE 8192 

void * echo(void * connfd);
void* allocatedMemory[1000];
void cleanup(int sock2);

typedef struct {
   pthread_attr_t *attr;
   pthread_mutex_t *mutex;
   int rcv_length;
   int s;
} arg_t;

void cleanup(int sock2)
{
  int   retval;

/*
 * If given, shutdown and close sock2.
 */
  retval = shutdown(sock2,2);
  if (retval == -1)
    perror ("shutyxxdown");

  retval = close (sock2);
  if (retval)
    perror ("close");

} /* end cleanup*/

int
main(int ac, char *av[])
{
    struct addrinfo *info, *pinfo;
    struct addrinfo hint;
    int sockets[10], nsockets = 0;
	int nthreads = 10;
	//pthread_t threads[10];
	int thread;
	int count = 0;
	
	
	arg_t *arg;
	pthread_t dthread;
	pthread_mutex_t *mutex;
	
	
	//pthread_attr_t attr;
	//size_t stacksize = 204800;
	//pthread_t threads[10];


    memset(&hint, 0, sizeof hint);//clear the object and then check for the right number of arguments passed
	
    if (ac < 3) printf("Usage: %s -p <port>\n", av[0]), exit(0);
	//set the right flags for ipv4
    hint.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG;
    hint.ai_protocol = IPPROTO_TCP; // only interested in TCP
	if (strcmp(av[1],"-p"))
	{
		return -1;
	}
    char *port_number_string = av[2];
	//char *t;
	//get address information
    int rc = getaddrinfo(NULL, port_number_string, &hint, &info);
	
    if (rc != 0)
        gai_strerror(rc), exit(-1);

    char printed_addr[1024];
    for (pinfo = info; pinfo; pinfo = pinfo->ai_next) {
        assert (pinfo->ai_protocol == IPPROTO_TCP);
        int rc = getnameinfo(pinfo->ai_addr, pinfo->ai_addrlen,
                             printed_addr, sizeof printed_addr, NULL, 0,
                             NI_NUMERICHOST);
		
        if (rc != 0)
            gai_strerror(rc), exit(-1);

        printf("%s: %s\n", pinfo->ai_family == AF_INET ? "AF_INET" :
                           pinfo->ai_family == AF_INET6 ? "AF_INET6" : "?", 
                           printed_addr);

        int s = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (s == -1)
            perror("socket"), exit(-1);

        int opt = 1;
        setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));

        rc = bind(s, pinfo->ai_addr, pinfo->ai_addrlen);
        if (rc == -1 && errno == EADDRINUSE) {
            // ignore Linux limitation
            close(s);
            continue;
        }

        if (rc == -1)
            perror("bind"), exit(-1);
		//begin listening on the socket
        rc = listen(s, 10);
        if (rc == -1)
            perror("listen"), exit(-1);

        assert(nsockets < sizeof(sockets)/sizeof(sockets[0]));
        sockets[nsockets++] = s;
    }
    freeaddrinfo(info);


    assert(nsockets == 1);
	
    struct sockaddr_storage rem;
    socklen_t remlen = sizeof (rem);
	
	//pthread_attr_init(&attr);
	//pthread_detach(threads[0]);
	//count++;
		
	//pthread_attr_setstacksize(&attr, stacksize);
	
	
	mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
    for (;;) {
		//accept a connection
		arg = malloc(sizeof(arg_t));
		arg->mutex = mutex;
		
        arg->s = accept (sockets[0], (struct sockaddr *) &rem, &remlen);
		
		
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
        // do reverse DNS lookup
		
        char buf1[200];

		//get information about the connection
        if (getnameinfo ((struct sockaddr *) &rem, remlen, buf1, sizeof (buf1), NULL, 0, 0))
            strcpy (buf1, "???");   // hostname unknown

        char buf2[100];
        (void) getnameinfo ((struct sockaddr *) &rem, remlen, 
                buf2, sizeof (buf2), NULL, 0, NI_NUMERICHOST);
        printf ("connection from %s (%s)\n", buf1, buf2);
		
		
		//create the thread and call the echo functions
		pthread_mutex_unlock(mutex);
		arg->attr = malloc(sizeof(pthread_attr_t));
		pthread_attr_init(arg->attr);
		pthread_attr_setstacksize(arg->attr, PTHREAD_STACK_MIN+BUFSIZE);
		pthread_attr_setdetachstate(arg->attr, PTHREAD_CREATE_DETACHED);
		pthread_create(&dthread, arg->attr, echo, arg);
		
    }

    return 0;
}


#include "csapp.h"

void parse_line(rio_t *rio, int fd);
void parse_uri(char *uri, char *filename, char *cgiargs);


void * echo(void * connfd)
{
	//int fd = *((int *) connfd);//get the file descriptor
	
	arg_t *arg = (arg_t *) connfd;
	int fd = arg->s;
	char method[1024], uri[8192], version[1024];
	char filename[1024], cgiargs[1024];
	size_t n;
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
	
	sscanf(buf, "%s %s %s", method, uri, version);
	//if it is a get
	if (strcasecmp(method, "GET") == 0)
	{		
		//search for the file of the request
		FILE *file; 
		
		char filetype[100];
		char * type;
		int s = '.';
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
			{
				filetype[count] = uri[current];
			}
			count++;
		}
		//get the correct mime type
		if (strcmp(filetype, ".html") == 0 || strcmp(filetype, ".htm") == 0)
			type = "text/html";
		else if (strcmp(filetype, ".jpg") == 0)
			type = "image/jpg";
		else if (strcmp(filetype, ".js") == 0)
			type = "text/javascript";
		else if (strcmp(filetype, ".gif") == 0)
			type = "image/gif";
		else if (strcmp(filetype, ".txt") == 0)
			type = "text/plain";
		else if (strcmp(filetype, ".ram") == 0 || strcmp(filetype, ".ra") == 0)
			type = "audio/x-pn-realaudio";
		else 
			type = "text/plain";
		
		char status[100];
		//format correctly
		if (version == "HTTP/1.1")
		{	
			strcpy(status, "HTTP/1.1");
		}
		else 
		{
			strcpy(status, "HTTP/1.0");
		}
		//char contentTypeLine[100] = "Content-type: ";
		char body[8192];// = "<html><head></head><body>";
		
		//if it is a junk file in any way
		if (strstr(uri, "junk") != NULL)
		{
			if (strcmp(version, "HTTP/1.0"))
			{
				strcat(body, "HTTP/1.0 404 Not Found");
			}
			else 
				strcat(body, "HTTP/1.1 404 Not Found");
			send(fd, body, strlen(body), 0);
		}
		//one of the checks
		else if (strstr(uri, "meminfo?callback=") != NULL)
		{

			char argument[100];
			char * position = strchr(uri, '=');
			position++;
			
			printf("tests %s\n", position);
			
			strcpy(argument, position);
			int y;
			int x = 0;
			for (y = 0; y < 100; y++)
			{
				if((argument[y] == '&' || argument[y] =='_' || argument[y] == '=') && x != 1)
				{
					argument[y] = '\0';
					y = 100;
					x = 1;
				}
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
			printf("here %s\n", argument);
		}
		else if (strncmp(uri, "/meminfo", 8) == 0)
		{
			//send info
			#include "json_mem_info.c"
			fclose(file);
			//strcpy(body, strcat(body, "</body></html>"));
			send(fd, body, strlen(body), 0);
		}		
		else if (strstr(uri, "loadavg?callback=") != NULL )
		{
			
			char argument[8192];
			
			char * position = strchr(uri, '=');
			position++;
			strcpy(argument, position);
			printf("tests %s\n", argument);
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
			printf("here %s\n", argument);
		}
		else if (strncmp(uri, "/loadavg", 8) == 0)
		{
			printf("loadavg\n");
			
			if (strcmp(version, "HTTP/1.0") == 0)
			{
				strcat(body, "HTTP/1.0 200 OK\r\n\r\n");
			}
			#include "json_loadavg.c"
			fclose(file);

			send(fd, body, strlen(body), 0);
		}
		//else if (strncmp(uri, "/loadavg?callback", 17) == 0)
		
		
		else if (strncmp(uri, "/runloop", 8) == 0){
			#include "runloop.c"
		}
		else if (strncmp(uri, "/allocanon", 10) == 0){
			#include "allocanon.c"
		}
		else if (strncmp(uri, "/freeanon", 9) == 0){
			#include "freeanon.c"
		}
		else 
		{
			
			int p;
			for(p = 1; p < strlen(uri)+1; p++)
			{
				uri[p-1] = uri[p];
			}
			file = fopen(uri, "r");
			if (file != NULL)
			{
				printf("fuck\n");
				char line[100];
				while (!feof(file))
				{
					fscanf(file, "%s\n", line);
					
					Rio_writen(fd, line, sizeof(line));
				}
				fclose(file);
			}
			else 
			{
				printf("yeah\n");
				if (strcmp(version, "HTTP/1.0"))
				{
					strcat(body, "HTTP/1.0 404 Not Found");
				}
				else 
					strcat(body, "HTTP/1.1 404 Not Found");
				send(fd, body, strlen(body), 0);				
				
			}
			
			
		}
	}
	else 
	{
		send(fd, "HTTP/1.1 405 Method Not Allowed", strlen("HTTP/1.1 405 Method Not Allowed"), 0);
	}

	close (fd);
	pthread_exit(NULL);
}





