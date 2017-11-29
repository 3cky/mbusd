/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * globals.h - global definitions
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
 * $Id: globals.h,v 1.4 2015/02/25 10:33:58 kapyar Exp $
 */

#ifndef _GLOBALS_H
#define _GLOBALS_H

/* Include global configuration header file */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#ifdef HAVE_LIBUTIL
#  include <libutil.h>
#endif

/*
 * Useful min/max macroses
 */
#define MAX(a, b) ( (a > b) ? a : b )
#define MIN(a, b) ( (a < b) ? a : b )

/*
 * Boolean constants
 */
#define FALSE 0
#define TRUE  !FALSE

/*
 * Constants
 */
#define RC_OK       0
#define RC_ERR     -1
#define RC_BREAK   -2
#define RC_TIMEOUT -3
#define RC_AOPEN   -4
#define RC_ACLOSE  -5

/* Internal string buffers size */
#if defined(PATH_MAX)
#  define INTBUFSIZE PATH_MAX
#else
#  define INTBUFSIZE 1023
#endif

#endif
