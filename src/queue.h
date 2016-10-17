/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * queue.h - connections queue management procedures
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
 * $Id: queue.h,v 1.4 2015/02/25 10:33:57 kapyar Exp $
 */

#ifndef _QUEUE_H
#define _QUEUE_H

#include "globals.h"
#include "conn.h"
#ifdef LOG
#  include "log.h"
#endif

/*
 * Queue parameters structure
 */
typedef struct
{
  conn_t *beg; /* address of first queue element */
  conn_t *end; /* address of last queue element */
  int len;     /* number of elements in the queue */
} queue_t;

/* prototypes */
void queue_init(queue_t *queue);
conn_t *queue_new_elem(queue_t *queue);
void queue_delete_elem(queue_t *queue, conn_t *conn);
conn_t *queue_next_elem(queue_t *queue, conn_t *conn);

#endif /* _QUEUE_H */
