/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * sock.h - socket manipulation routines
 *
 * Copyright (c) 2002-2003, 2013, Victor Antonovich (v.antonovich@gmail.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: sock.h,v 1.4 2015/02/25 10:33:57 kapyar Exp $
 */

#ifndef _SOCKUTILS_H
#define _SOCKUTILS_H

#include "globals.h"
#ifdef LOG
#  include "log.h"
#endif

#define BACKLOG 5

/* Socket buffers size */
#define SOCKBUFSIZE 512

#define sa_len(sa_ptr) ((sa_ptr)->sa_family == AF_INET \
    ? sizeof (struct sockaddr_in) : sizeof (struct sockaddr_in6))

int sock_set_blkmode(int sd, int blkmode);
int sock_create(int blkmode, sa_family_t sa_family);
int sock_create_server(char *server_ip, unsigned short server_port, int blkmode);
int sock_accept(int server_sd, struct sockaddr *rmt_addr, socklen_t rmt_len, int blkmode);
void *sock_addr(struct sockaddr *sa);

#endif
