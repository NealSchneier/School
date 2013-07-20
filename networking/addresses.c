/*
 * Find suitable IPv6/IPv4 address to bind to based on a numeric port number
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

/*
int getaddrinfo(const char *node, const char *service,
               const struct addrinfo *hints,
               struct addrinfo **res);

void freeaddrinfo(struct addrinfo *res);

struct addrinfo {
   int     ai_flags;
   int     ai_family;
   int     ai_socktype;
   int     ai_protocol;
   size_t  ai_addrlen;
   struct sockaddr *ai_addr;
   char   *ai_canonname;
   struct addrinfo *ai_next;
};

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
               char *host, size_t hostlen,
               char *serv, size_t servlen, int flags);

*/


int
main(int ac, char *av[])
{
    struct addrinfo *info, *pinfo;
    struct addrinfo hint;
    memset(&hint, 0, sizeof hint);
    if (ac < 2) printf("Usage: %s <port>\n", av[0]), exit(0);

    hint.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    hint.ai_protocol = IPPROTO_TCP;
    char *port_number_string = av[1];
    int rc = getaddrinfo(NULL, port_number_string, &hint, &info);
    if (rc != 0)
        gai_strerror(rc), exit(-1);

    char printed_addr[1024];
    for (pinfo = info; pinfo; pinfo = pinfo->ai_next) {
        int rc = getnameinfo(pinfo->ai_addr, pinfo->ai_addrlen,
                             printed_addr, sizeof printed_addr, NULL, 0,
                             NI_NUMERICHOST);
        if (rc != 0)
            gai_strerror(rc), exit(-1);

        printf("%s: %s\n", pinfo->ai_family == AF_INET ? "AF_INET" :
                           pinfo->ai_family == AF_INET6 ? "AF_INET6" : "?", 
                           printed_addr);

    }
    freeaddrinfo(info);
    return 0;
}

