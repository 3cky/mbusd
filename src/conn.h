/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * conn.h - connections management procedures
 *
 * Copyright (c) 2002-2003, Victor Antonovich (avmlink@vlink.ru)
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
 * $Id: conn.h,v 1.1 2003/09/13 20:38:33 kapyar Exp $
 */
 
#ifndef _CONN_H
#define _CONN_H

#include "globals.h"
#include "cfg.h"
#include "sock.h"
#include "modbus.h"
#include "sig.h"
#ifdef LOG
#include "log.h"
#endif

/*
 * Default values 
 */
#define DEFAULT_SERVERPORT 502
#define DEFAULT_MAXCONN 32
#define DEFAULT_MAXTRY 3
#define DEFAULT_RQSTPAUSE 100
#define DEFAULT_RESPWAIT 10
#define DEFAULT_CONNTIMEOUT 60

#define CRCSIZE 2       /* size (in bytes) of CRC */
#define HDRSIZE 6       /* size (in bytes) of header */
#define BUFSIZE 256     /* size (in bytes) of MODBUS data */
#define RQSTSIZE (HDRSIZE + BUFSIZE - 2) /* size (in bytes) of MODBUS request */

/*
 * Client connection FSM states
 */
#define CONN_HEADER 0
#define CONN_RQST   1
#define CONN_TTY    2
#define CONN_RESP   3

/*
 * Client connection related data storage structure
 */
typedef struct conn_t
{
  struct conn_t *prev;  /* linked list previous connection */
  struct conn_t *next;  /* linked list next connection */
  int sd;               /* socket descriptor */
  int state;            /* current state */
  int timeout;          /* timeout value, secs */
  struct sockaddr_in sockaddr; /* connection structure */
  int ctr;              /* counter of data in the buffer */
  unsigned char buf[HDRSIZE + BUFSIZE];    /* data buffer */
} conn_t;

/* prototypes */
int conn_init(void);
void conn_loop(void);
void conn_open(void);
conn_t *conn_close(conn_t *conn);

#endif /* _CONN_H */
