
#include "csapp.h"

void parse_line(rio_t *rio, int fd);
void parse_uri(char *uri, char *filename, char *cgiargs);


void * echo(void * connfd)
{
	int fd = *((int *) connfd);
	char method[1024], uri[8192], version[1024];
	char filename[1024], cgiargs[1024];
	size_t n;
	char buf[8192];
	long allocatedMemory[1000];
	memset(buf, '\0', sizeof(buf));
	memset(method, '\0', sizeof(method));
	memset(uri, '\0', sizeof(uri));
	memset(version, '\0', sizeof(version));

	rio_t rio;
	
	Rio_readinitb(&rio, fd);
	Rio_readlineb(&rio, buf, 8192);
	
	sscanf(buf, "%s %s %s", method, uri, version);
	
	if (strcasecmp(method, "GET") == 0)
	{		
		//Rio_writen(fd, buf, sizeof(buf));
		
		//search for the file of the request
		FILE *file; 
		
		char filetype[100];
		char * type;
		int s = '.';
		int position = -1;
		int current = 100;
		int count = 4;
		//gets the extensio
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
		
		if (strcmp(filetype, ".html") == 0 || strcmp(filetype, ".htm") == 0)
			type = "text/html";
		else if (strcmp(filetype, ".jpg") == 0)
			type = "image/jpg";
		else if (strcmp(filetype, ".gif") == 0)
			type = "image/gif";
		else if (strcmp(filetype, ".txt") == 0)
			type = "text/plain";
		else if (strcmp(filetype, ".ram") == 0 || strcmp(filetype, ".ra") == 0)
			type = "audio/x-pn-realaudio";
		else 
			type = "text/plain";
		
		char status[100];
		if (version == "HTTP/1.1")
		{	
			strcpy(status, "HTTP/1.1");
		}
		else if (version == "HTTP/1.0")
		{
			strcpy(status, "HTTP/1.0");
		}
		//char contentTypeLine[100] = "Content-type: ";
		char body[8192];// = "<html><head></head><body>";
		
		
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
		else if (strncmp(uri, "/index.html", 11) == 0)
		{
			//printf("index.html\n");
			file = fopen("index.html", "r");
			if (file == NULL)
				printf("not found\n");
			
			char line[100];
			while (!feof(file))
			{
				fscanf(file, "%s\n", line);
				
				Rio_writen(fd, line, sizeof(line));
			}
			fclose(file);
		}
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
		else if (strstr(uri, "/meminfo?") != NULL && strstr(uri, "&callback=") != NULL)
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
			#include "json_mem_info.c"
			strcat(body, ")");
			
			send(fd, body, strlen(body), 0);
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
		//else if (strncmp(uri, "/meminfo?callback", 17) == 0)
		
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
		else 
		{

			file = fopen(uri, "r");
			if (file != NULL)
			{
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
				strcat(status, "200 OK");
				//strcat(contentTypeLine, type);
				
			
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





