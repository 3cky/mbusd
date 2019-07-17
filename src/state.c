/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * state.c - state management procedures
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
 * $Id: state.c,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */

#include "state.h"

/*
 * Search for first client connection in state STATE
 * Parameters: CONN - ptr to queue element
 *             (if NULL - search from queue begin),
 *             QUEUE - ptr to the queue;
 * Return:     pointer to queue element
 *             or NULL if none found
 */
conn_t *
state_conn_search(queue_t *queue, conn_t *conn, int state)
{
  int len = queue->len;

  /* check for zero queue length */
  if (!queue->len) return NULL;

  if (conn == NULL)
    conn = queue->beg;
  else
    conn = queue_next_elem(queue, conn);

  while (len--)
  {
    if (conn->state == state)
      return conn;
    conn = queue_next_elem(queue, conn);
  }

  return NULL; /* none found */
}

/*
 * Set connection CONN to STATE
 */
void
state_conn_set(conn_t *conn, int state)
{
  switch (state)
  {
    case CONN_HEADER:
      conn->ctr = 0;
      conn->read_len = HDRSIZE;
#ifdef DEBUG
      logw(5, "conn[%s]: state now is CONN_HEADER", conn->remote_addr);
#endif
      break;
    case CONN_RQST_FUNC:
      conn->read_len = HDRSIZE + MB_FRAME(conn->buf, MB_LENGTH_L);
#ifdef DEBUG
      logw(5, "conn[%s]: state now is CONN_RQST_FUNC", conn->remote_addr);
#endif
    break;
    case CONN_RQST_NVAL:
#ifdef DEBUG
      logw(5, "conn[%s]: state now is CONN_RQST_NVAL", conn->remote_addr);
#endif
    break;
    case CONN_RQST_TAIL:
#ifdef DEBUG
      logw(5, "conn[%s]: state now is CONN_RQST_TAIL", conn->remote_addr);
#endif
    break;
    case CONN_TTY:
#ifdef DEBUG
      logw(5, "conn[%s]: state now is CONN_TTY", conn->remote_addr);
#endif
      break;
    case CONN_RESP:
      conn->ctr = 0;
#ifdef DEBUG
      logw(5, "conn[%s]: state now is CONN_RESP", conn->remote_addr);
#endif
      break;
    default:
      /* unknown state, exiting */
#ifdef DEBUG
      logw(5, "conn_set_state([%s]) - invalid state (%d)", conn->remote_addr, state);
#endif
      exit (-1);
  }
  conn->state = state;
  /* reset timeout value */
  conn->timeout = cfg.conntimeout;
}

/*
 * Set tty device to STATE
 */
void
state_tty_set(ttydata_t *mod, int state)
{
  switch (state)
  {
    case TTY_PAUSE:
      mod->trynum = 0;
      mod->timer = (unsigned long)cfg.rqstpause * 1000l;
#ifdef DEBUG
      logw(5, "tty: state now is TTY_PAUSE");
#endif
      break;
    case TTY_READY:
      mod->trynum = 0;
      mod->timer = 0l;
#ifdef DEBUG
      logw(5, "tty: state now is TTY_READY");
#endif
      break;
    case TTY_RQST:
      mod->ptrbuf = 0;
      mod->timer = 0l;
      mod->trynum = mod->trynum ? mod->trynum - 1 : (unsigned)cfg.maxtry;
#ifdef DEBUG
      logw(5, "tty: state now is TTY_RQST");
#endif
#ifndef NOSILENT
      tty_delay(DV(2, cfg.ttyspeed));
#endif
      break;
    case TTY_RESP:
      mod->ptrbuf = 0;
      mod->rxoffset = 0;
      /* XXX need real recv length? */
      mod->rxlen = TTY_BUFSIZE;
      mod->timer = cfg.respwait * 1000l + DV(mod->txlen, mod->speed);
#ifdef DEBUG
      logw(5, "tty: state now is TTY_RESP");
#endif
      break;
    case TTY_PROC:
#ifdef DEBUG
      logw(5, "tty: state now is TTY_PROC");
#endif
      break;
    default:
      /* unknown state, exiting */
#ifdef DEBUG
      logw(5, "tty_set_state() - invalid state (%d)", state);
#endif
      exit (-1);
  }
  mod->state = state;
}
