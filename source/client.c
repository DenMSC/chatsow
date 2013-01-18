/*
Copyright (C) 2013 hettoo (Gerco van Heerdt)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"

#include "import.h"
#include "utils.h"
#include "ui.h"
#include "parser.h"

static qboolean started = qfalse;
static pthread_t thread;

static char *host;
static char *port;
static int port_int;
static struct sockaddr_in serv_addr;
static int sockfd;
static socklen_t slen = sizeof(serv_addr);

static int outseq = 1;
static int inseq = 0;

static void msg_clear(msg_t *msg) {
    msg->readcount = 0;
    msg->cursize = 0;
    msg->maxsize = sizeof(msg->data);
    msg->compressed = qfalse;
}

static void client_connect() {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket");

    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_int);

    if (inet_aton(host, &serv_addr.sin_addr) == 0)
        die("inet_aton");
}

static void msg_init(msg_t *msg) {
    msg_clear(msg);
    write_long(msg, outseq++);
    write_long(msg, inseq);
    write_short(msg, port_int);
}

static void msg_init_outofband(msg_t *msg) {
    msg_clear(msg);
    write_long(msg, -1);
}

static void client_send(msg_t *msg) {
    if (sendto(sockfd, msg->data, msg->cursize, 0, (struct sockaddr*)&serv_addr, slen) == -1)
        die("sendto");
}

static void client_recv(msg_t *msg) {
    msg_clear(msg);
    if ((msg->cursize = recvfrom(sockfd, msg->data, msg->maxsize, 0, (struct sockaddr*)&serv_addr, &slen)) == -1)
        die("recvfrom");
    int seq = read_long(msg);
    qboolean compressed = qfalse;
    qboolean fragmented = qfalse;
    if (seq != -1) {
        if (seq & FRAGMENT_BIT) {
            seq &= ~FRAGMENT_BIT;
            fragmented = qtrue;
        }
        inseq = seq;
        int ack = read_long(msg);
        if (ack & FRAGMENT_BIT) {
            ack &= ~FRAGMENT_BIT;
            compressed = qtrue;
        }
    }
    if (compressed) {
        die("compressed");
    }
    if (fragmented) {
        die("fragmented");
    }
}

static void *client_run(void *args) {
    client_connect();

    msg_t msg;
    msg_init_outofband(&msg);
    write_string(&msg, "getchallenge");
    client_send(&msg);

    client_recv(&msg);
    skip_data(&msg, 10);
    char challenge[50];
    strcpy(challenge, (char *)msg.data + msg.readcount);

    msg_init_outofband(&msg);
    write_string(&msg, "connect 15 ");
    msg.cursize--;
    write_string(&msg, port);
    msg.cursize--;
    write_string(&msg, " ");
    msg.cursize--;
    write_string(&msg, challenge);
    msg.cursize--;
    write_string(&msg, " \"\\name\\chattoo\" 0\n");
    client_send(&msg);

    while (1) {
        client_recv(&msg);
        parse_message(&msg);
    }

    close(sockfd);

    return NULL;
}

void client_start(char *new_host, char *new_port) {
    host = new_host;
    port = new_port;
    port_int = atoi(port);
    pthread_create(&thread, NULL, client_run, NULL);
    started = qtrue;
}

void client_stop() {
    if (started) {
        pthread_join(thread, NULL);
        started = qfalse;

        msg_t msg;
        msg_init(&msg);
        write_byte(&msg, clc_clientcommand);
        write_string(&msg, "cmd disconnect");
        client_send(&msg);
    }
}

void execute(char *command) {
    ui_output(command);
}
