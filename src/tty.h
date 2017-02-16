/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * tty.h - terminal I/O related procedures
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
 * $Id: tty.h,v 1.4 2015/02/25 10:33:58 kapyar Exp $
 */

#ifndef _TTY_H
#define _TTY_H

#include "globals.h"
#include "cfg.h"

/*
 * Delay value calculation macros
 */
#define	DV(x, y) (x * 10000000l / y)

/*
 * Default tty port parameters
 */
#if defined (__CYGWIN__)
#  define DEFAULT_PORT "/dev/COM1"
#elif defined (__linux__)
#  define DEFAULT_PORT "/dev/ttyS0"
#else
#  define DEFAULT_PORT "/dev/cuaa0"
#endif

#define DEFAULT_SPEED 19200
#define DEFAULT_BSPEED B19200

#define DEFAULT_MODE "8N1"

/*
 * Maximum tty buffer size
 */
#define TTY_BUFSIZE 256

/*
 * TRX control types
 */
#ifdef  TRXCTL
#define TRX_ADDC    0
#define TRX_RTS     1
#define TRX_SYSFS_1 2
#define TRX_SYSFS_0 3
#endif

/*
 * TTY device FSM states
 */
#define TTY_PAUSE 0
#define TTY_READY 1
#define TTY_RQST  2
#define TTY_RESP  3
#define TTY_PROC  4

/*
 * TTY related data storage structure
 */
typedef struct
{
  int fd;                       /* tty file descriptor */
  int speed;                    /* serial port speed */
  char *port;                   /* serial port device name */
#ifdef TRXCTL
  int trxcntl;                  /* trx control type (enum - see values in config.h) */
#endif
  struct termios tios;          /* working termios structure */
  struct termios savedtios;     /* saved termios structure */
  int state;                    /* current state */
  unsigned int trynum;             /* try counter */
  unsigned long timer;          /* time tracking variable */
  unsigned int txlen;           /* tx data length */
  unsigned int rxlen;           /* rx data length */
  unsigned char ptrbuf;         /* ptr in the buffer */
  unsigned char rxoffset;       /* ptr in the buffer */
  unsigned char txbuf[TTY_BUFSIZE]; /* transmitting buffer */
  unsigned char rxbuf[TTY_BUFSIZE]; /* receiving buffer */
} ttydata_t;

/* prototypes */
void tty_sighup(void);
#ifdef TRXCTL
void tty_init(ttydata_t *mod, char *port, int speed, int trxcntl);
#else
void tty_init(ttydata_t *mod, char *port, int speed);
#endif
int tty_open(ttydata_t *mod);
int tty_set_attr(ttydata_t *mod);
speed_t tty_transpeed(int speed);
int tty_cooked(ttydata_t *mod);
int tty_close(ttydata_t *mod);
void tty_set_rts(int fd);
void tty_clr_rts(int fd);
void tty_delay(int usec);

#endif /* _TTY_H */
