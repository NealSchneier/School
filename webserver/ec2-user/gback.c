/*
 * Simple echo server that supports both IPv6 and IPv4.
 * (Linux only).
 *
 * Godmar Back, CS 3214 Spring 2010
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

int
main(int ac, char *av[])
{
    struct addrinfo *info, *pinfo;
    struct addrinfo hint;
    int sockets[10], nsockets = 0;

    memset(&hint, 0, sizeof hint);
    if (ac < 2) printf("Usage: %s <port>\n", av[0]), exit(0);

    hint.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG;
    hint.ai_protocol = IPPROTO_TCP; // only interested in TCP
    char *port_number_string = av[1];
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

        rc = listen(s, 10);
        if (rc == -1)
            perror("listen"), exit(-1);

        assert(nsockets < sizeof(sockets)/sizeof(sockets[0]));
        sockets[nsockets++] = s;
    }
    freeaddrinfo(info);

    // Linux-only. Assume that we were only able to bind 1 socket
    // Following Ulrich Drepper's example from here:
    // http://people.redhat.com/drepper/userapi-ipv6.html
    assert(nsockets == 1);

    struct sockaddr_storage rem;
    socklen_t remlen = sizeof (rem);

    for (;;) {
        int fd = accept (sockets[0], (struct sockaddr *) &rem, &remlen);
        if (fd == -1)
            perror ("accept"), exit(-1);

        // do reverse DNS lookup
        char buf1[200];
        if (getnameinfo ((struct sockaddr *) &rem, remlen, buf1, sizeof (buf1), NULL, 0, 0))
            strcpy (buf1, "???");   // hostname unknown

        char buf2[100];
        (void) getnameinfo ((struct sockaddr *) &rem, remlen, 
                buf2, sizeof (buf2), NULL, 0, NI_NUMERICHOST);
        printf ("connection from %s (%s)\n", buf1, buf2);

        char buf[1024];
        for (;;) {
            ssize_t l = read (fd, buf, sizeof (buf));
            if (l <= 0)
                break; 
            write (fd, buf, l);
        }
        close (fd);
    }

    return 0;
}
