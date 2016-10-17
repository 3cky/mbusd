/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * queue.c - connections queue management procedures
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
 * $Id: queue.c,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */

#include "queue.h"

/*
 * Queue structure initialization
 */
void
queue_init(queue_t *queue)
{
  queue->beg = NULL;
  queue->end = NULL;
  queue->len = 0;
}

/*
 * Add new element to queue
 */
conn_t *
queue_new_elem(queue_t *queue)
{
  conn_t *newconn = (conn_t *)malloc(sizeof(conn_t));
  if (!newconn)
  { /* Aborting program execution */
#ifdef LOG
    logw(0, "queue_new_elem(): out of memory for new element (%s)",
           strerror(errno));
#endif
    exit(errno);
  }
  newconn->next = NULL;
  if ((newconn->prev = queue->end) != NULL)
    queue->end->next = newconn;
  else /* we add first element */
    queue->beg = newconn;
  queue->end = newconn;
  queue->len++;
#ifdef DEBUG
  logw(5, "queue_new_elem(): length now is %d", queue->len);
#endif
  return newconn;
}

/*
 * Remove element from queue
 */
void
queue_delete_elem(queue_t *queue, conn_t *conn)
{
  if (queue->len <= 0)
  { /* queue is empty */
#ifdef LOG
    logw(1, "queue_delete_elem(): queue empty!");
#endif
    return;
  }
  if (conn->prev == NULL)
  { /* deleting first element */
    if ((queue->beg = queue->beg->next) != NULL)
      queue->beg->prev = NULL;
  }
  else 
    conn->prev->next = conn->next;
  if (conn->next == NULL)
  { /* deleting last element */
    if ((queue->end = queue->end->prev) != NULL)
      queue->end->next = NULL;
  }
  else
    conn->next->prev = conn->prev;
  queue->len--;
  free((void *)conn);
#ifdef DEBUG  
  logw(5, "queue_delete_elem(): length now is %d", queue->len);
#endif
  return;
}

/*
 * Obtain pointer to next element in the QUEUE (with wrapping)
 * Parameters: CONN - pointer to current queue element
 * Return: pointer to next queue element
 */
conn_t *
queue_next_elem(queue_t *queue, conn_t *conn)
{
  return (conn->next == NULL) ? queue->beg : conn->next;
}
