/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * cfg.h - configuration related procedures
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
 * $Id: cfg.h,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */

#ifndef _CFG_H
#define _CFG_H

#include "globals.h"
#include "log.h"
#include "tty.h"
#include "conn.h"

/* Global configuration storage structure */
typedef struct
{
#ifdef LOG
  /* debug level */
  char dbglvl;
  /* log file name */
  char logname[INTBUFSIZE + 1];
#endif
  /* tty port name */
  char ttyport[INTBUFSIZE + 1];
  /* tty speed */
  int ttyspeed;
  /* tty mode */
  char ttymode[INTBUFSIZE + 1];
  /* trx control type (0 - ADDC, 1 - by RTS, 2 - by sysfs GPIO with 1 activating transmit, 3 - by sysfs GPIO with 0 activating transmit) */
  int trxcntl;
  /* trx control sysfs file */
  char trxcntl_file[INTBUFSIZE + 1];
  /* TCP server address */
  char serveraddr[INTBUFSIZE + 1];
  /* TCP server port number */
  int serverport;
  /* maximum number of connections */
  int maxconn;
  /* number of tries of request in case timeout (0 - no tries attempted) */
  int maxtry;
  /* staled connection timeout (in sec) */
  int conntimeout;
  /* inter-request pause (in msec) */
  unsigned long rqstpause;
  /* response waiting time (in msec) */
  unsigned long respwait;
  /* inter-byte response pause (in usec) */
  unsigned long resppause;
} cfg_t;

/* Prototypes */
extern cfg_t cfg;
extern char cfg_err[];
void cfg_init(void);
int cfg_read_file(const char *filename);

#endif /* _CFG_H */
