/* Copyright © 2012 Bart Massey */

/* TFTP send program. */

#include <assert.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
    assert(argc == 2);
    struct hostent *h = gethostbyname(argv[1]);
    if (!h) {
        herror("bad destination");
        exit(1);
    }
    assert(h->h_addrtype == AF_INET);
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(69);
    sin.sin_addr.s_addr = *(uint32_t *)h->h_addr;
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s != -1);
    int r = connect(s, (const struct sockaddr *)&sin, sizeof(sin));
    assert(r != -1);
}
