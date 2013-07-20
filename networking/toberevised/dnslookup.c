/*
 * Demo for how to use getaddrinfo instead of gethostbyname
 *
 * G. Back <gback@cs.vt.edu>
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

static char *
ptostring(int prot)
{
    switch (prot) {
    case IPPROTO_IP: return "IPPROTO_IP";
    case IPPROTO_TCP: return "IPPROTO_TCP";
    case IPPROTO_UDP: return "IPPROTO_UDP";
    default: return "unknown";
    }
}

int
main(int ac, char *av[])
{
    char * host = av[1];
    struct addrinfo *info, *pinfo;
    struct addrinfo hint;
    memset(&hint, 0, sizeof hint);
    hint.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
    hint.ai_protocol = IPPROTO_TCP;
    int rc = getaddrinfo(host, NULL, &hint, &info);
    if (rc != 0)
        gai_strerror(rc), exit(-1);

    for (pinfo = info; pinfo; pinfo = pinfo->ai_next) {
        switch (pinfo->ai_family) {
        case AF_INET:
            {
                struct sockaddr_in *sa; 
                sa = (struct sockaddr_in *)pinfo->ai_addr;
                printf("AF_INET: %s canon=%s prot=%s\n", 
                    inet_ntoa(sa->sin_addr), 
                    pinfo->ai_canonname,
                    ptostring(pinfo->ai_protocol));
                break;
            }
        case AF_INET6:
            {
                struct sockaddr_in6 *sa; 
                sa = (struct sockaddr_in6 *)pinfo->ai_addr;

                char printed_addr[1024];
                inet_ntop(pinfo->ai_family, (const void *)&sa->sin6_addr, 
                    printed_addr, sizeof (printed_addr)), 
                printf("AF_INET6: %s canon=%s prot=%s\n", 
                    printed_addr,
                    pinfo->ai_canonname,
                    ptostring(pinfo->ai_protocol));
                break;
            }
        default:
            printf("Unknown address family\n");
            break;
        }
    }
    freeaddrinfo(info);
    return 0;
}

