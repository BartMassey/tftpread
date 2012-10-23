/*
 * Copyright © 2012 Bart Massey
 * [This program is licensed under the "MIT License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */


/* TFTP client read program. */

#include <assert.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

static unsigned char msgbuf[516];

static int format_rrq(char *filename) {
    char *modestr = "octet";
    msgbuf[0] = 0;
    msgbuf[1] = 1;
    strcpy((char *)msgbuf + 2, filename);
    strcpy((char *)msgbuf + 2 + strlen(filename) + 1, modestr);
    return 2 + strlen(filename) + 1 + strlen(modestr) + 1;
}

static int process_response(int msglen, int blocknum) {
    /* XXX should check msglen in case of corrupt packet */
    unsigned short opcode = (msgbuf[0] << 8) | msgbuf[1];
    switch (opcode) {
    case 5 /* error */:
        fprintf(stderr, "error: %s\n", msgbuf + 4);
        exit(1);
    case 3 /* data */: {
        int actualblock = (msgbuf[2] << 8) | msgbuf[3];
        if (actualblock != blocknum) {
            fprintf(stderr, "received unexpected block %d\n", actualblock);
            return -1;
        }
        int r = fwrite(msgbuf + 4, 1, msglen - 4, stdout);
        assert(r == msglen - 4);
        return msglen - 4; }
    }
    fprintf(stderr, "unknown opcode 0x%04x received, giving up\n", opcode);
    exit(1);
}

void format_ack(int blocknum) {
    msgbuf[0] = 0;
    msgbuf[1] = 4;
    msgbuf[2] = (blocknum >> 8) & 0xff;
    msgbuf[3] = blocknum & 0xff;
}

int main(int argc, char **argv) {
    assert(argc == 3);

    /* XXX Should try for raw IP address first. */
    struct hostent *h = gethostbyname(argv[1]);
    if (!h) {
        herror("bad destination");
        exit(1);
    }
    assert(h->h_addrtype == AF_INET);

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    assert(s != -1);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(69);
    /* XXX Should try all the addresses in the list. */
    sin.sin_addr.s_addr = *(uint32_t *)h->h_addr;

    int size = format_rrq(argv[2]);
    int r = sendto(s, msgbuf, size, 0,
                   (const struct sockaddr *)&sin, (socklen_t)sizeof(sin));
    assert(r == size);

    struct sockaddr_in ssin;
    socklen_t ssaddrlen = sizeof(ssin);
    r = recvfrom(s, (void *)msgbuf, sizeof(msgbuf), 0,
                 (struct sockaddr *)&ssin, &ssaddrlen);
    assert(r >= 0);

    int blocknum = 1;
    while (1) {
        int transferred = process_response(r, blocknum);
        if (transferred == -1)
            continue;
        format_ack(blocknum);
        r = sendto(s, msgbuf, size, 0,
                   (const struct sockaddr *)&ssin, (socklen_t)sizeof(ssin));
        if (transferred < 512) {
#ifdef DEBUG
            fprintf(stderr, "processing last %d bytes\n", transferred);
#endif
            break;
        }
        ssaddrlen = sizeof(ssin);
        r = recvfrom(s, (void *)msgbuf, sizeof(msgbuf), 0,
                     (struct sockaddr *)&ssin, &ssaddrlen);
        assert(r >= 0);
        blocknum++;
    }

    return 0;
}
