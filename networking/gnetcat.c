/*
 * Godmar's version of the netcat(nc) utility.
 *
 * Yet another version of netcat. See http://nc110.sourceforge.net/
 *
 * netcat connects to a server, or accepts a client, and subsequently
 * shovels data between the network and its own stdin/stdout.
 * This version differs in how it handles EOF.
 *
 * When it encounters EOF on stdin, it'll shutdown the send direction
 * of the network socket, then continue copying from the network socket
 * to stdout until EOF is seen on the network socket.
 *
 * When it encounters EOF on the network socket, it'll close stdout,
 * then continue copying from stdin to the network socket until
 * EOF is seen on stdin.
 *
 * This allows it to be used in connection with the dpipe program, which
 * connects its stdin/stdout to another program.  Closing stdout is necessary
 * to drive programs that do not output anything until they encounter EOF
 * on their stdin (e.g., fmt, indent, etc. etc.)
 *
 * This program uses an old-school select loop.
 *
 * This error checking in this program is deficient for production code;
 * it would lead to deduction were this program to be submitted for
 * grading.
 *
 * Godmar Back, CS 3214 Fall 2011
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

static struct addrinfo * 
get_addrinfo_from_port(char *port_number_string)
{
    struct addrinfo *info;
    struct addrinfo hint;
    memset(&hint, 0, sizeof hint);

    hint.ai_flags = AI_PASSIVE | AI_NUMERICSERV | AI_ADDRCONFIG;
    hint.ai_protocol = IPPROTO_TCP; // only interested in TCP
    int rc = getaddrinfo(NULL, port_number_string, &hint, &info);
    if (rc != 0)
        gai_strerror(rc), exit(-1);
    return info;
}

static int 
accept_client(struct addrinfo *info) 
{
    int sockets[10], nsockets = 0;
    struct addrinfo *pinfo;
    int clientfd;

    for (pinfo = info; pinfo; pinfo = pinfo->ai_next) {
        int s = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (s == -1)
            perror("socket"), exit(-1);

        int opt = 1;
        int rc = setsockopt (s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));
        if (rc != 0)
            perror("setsockopt"), exit(-1);

        rc = bind(s, pinfo->ai_addr, pinfo->ai_addrlen);
        if (rc == -1 && errno == EADDRINUSE) {
            // ignore Linux limitation that bind() on the IPv4 sockets fails if
            // IPv6 socket is dual-bound.
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

    clientfd = accept (sockets[0], (struct sockaddr *) &rem, &remlen);
    if (clientfd == -1)
        perror ("accept"), exit(-1);

    close(sockets[0]);
    return clientfd;
}

static int
connect_to(char *host, char *port)
{
    struct addrinfo *info, *pinfo;
    struct addrinfo hint;
    memset(&hint, 0, sizeof hint);
    hint.ai_flags = AI_CANONNAME | AI_NUMERICSERV | AI_ADDRCONFIG;
    hint.ai_protocol = IPPROTO_TCP;
    int rc = getaddrinfo(host, port, &hint, &info);
    if (rc != 0)
        gai_strerror(rc), exit(-1);

    for (pinfo = info; pinfo; pinfo = pinfo->ai_next) {
        int s = socket(pinfo->ai_family, pinfo->ai_socktype, pinfo->ai_protocol);
        if (s < 0)
            perror("socket"), exit(-1);
        
        if (connect(s, pinfo->ai_addr, pinfo->ai_addrlen) == 0)
            return s;

        // silently ignore connect errors, just keep trying.
    }
    perror("connect");
    exit(-1);
}

static void usage(char *p)
{
    fprintf(stderr, "Usage: %s [-l port] [hostname] [port]\n", p);
    abort();
}

static int
copy(int from, int to)
{
    char buf[8192];
    ssize_t bytesread = read (from, buf, sizeof (buf));
    if (bytesread > 0) {
        ssize_t byteswritten = write (to, buf, bytesread);
        if (bytesread != byteswritten)
            perror("write"), exit(-1);
    }

    return bytesread;
}

int
main(int ac, char *av[])
{
    struct addrinfo *serverinfo = NULL;

    int c;
    while ((c = getopt (ac, av, "l:")) != -1) {
        switch (c) {
        case 'l':
            serverinfo = get_addrinfo_from_port(optarg);
            break;
        default:
            usage(av[0]);
        }
    }

    int netfd;
    if (serverinfo) {
        netfd = accept_client(serverinfo);
    } else {
        if (optind + 1 >= ac)
            usage(av[0]);
        netfd = connect_to(av[optind], av[optind+1]);
    }

    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        FD_SET(netfd, &fds);
        int r = select(netfd+1, &fds, NULL, NULL, NULL);
        if (r < 0)
            perror("select"), exit(-1);
        assert (r > 0);

        if (FD_ISSET(0, &fds)) {
            int r = copy(0, netfd);
            if (r < 0)
                perror("read"), exit(-1);

            if (r == 0) {
                shutdown(netfd, SHUT_WR);
                while (copy(netfd, 1) > 0)
                    ; 
                break;
            }
        }

        if (FD_ISSET(netfd, &fds)) {
            int r = copy(netfd, 1);
            if (r < 0)
                perror("read"), exit(-1);

            if (r == 0) {
                close(1);
                while (copy(0, netfd) > 0)
                    ; 
                break;
            }
        }
    }

    return 0;
}
