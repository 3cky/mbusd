/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * conn.c - connections management procedures
 *
 * Copyright (c) 2002-2003, 2013, 2015 Victor Antonovich (v.antonovich@gmail.com)
 * Copyright (c) 2011 Andrew Denysenko <nitr0@seti.kr.ua>
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
 * $Id: conn.c,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */

#include "conn.h"
#include "queue.h"
#include "state.h"

/* global variables */
extern int server_sd;
extern queue_t queue;
extern ttydata_t tty;
extern cfg_t cfg;

conn_t *actconn; /* last active connection */
int max_sd; /* major descriptor in the select() sets */

void conn_tty_start(ttydata_t *tty, conn_t *conn);
ssize_t conn_read(int d, void *buf, size_t nbytes);
ssize_t conn_write(int d, void *buf, size_t nbytes, int istty);
void conn_fix_request_header_len(conn_t *conn, unsigned char len);

#define FD_MSET(d, s) do { FD_SET(d, s); max_sd = MAX(d, max_sd); } while (0);

int tty_reopen()
{
  logw(3, "tty re-opening...");
  tty_close(&tty);
#ifdef  TRXCTL
  tty_init(&tty, cfg.ttyport, cfg.ttyspeed, cfg.trxcntl);
#else
  tty_init(&tty, cfg.ttyport, cfg.ttyspeed);
#endif
  if (tty_open(&tty) != RC_OK)
  {
#ifdef LOG
    logw(0, "tty_reopen():"
           " can't open tty device %s (%s)",
           cfg.ttyport, strerror(errno));
#endif
    return RC_ERR;
  }
  state_tty_set(&tty, TTY_PAUSE);
  logw(3, "tty re-opened.");
  return RC_OK;
}

void tty_reinit()
{
  logw(3, "trying to re-open tty...");
  long delay = 100000l; /* initial open retry delay, 100 msecs */
  while (tty_reopen())
  {
    usleep(delay);
    if (sig_flag) sig_exec(); /* check for signals */
    delay = MIN(delay*2, 3000000l); /* retry delay exponential backoff, up to 3 secs */
  }
}

/*
 * Connections startup initialization
 * Parameters: none
 * Return: RC_OK in case of success, RC_ERR otherwise
 */
int
conn_init(void)
{
  /* tty device initialization */
#ifdef  TRXCTL
  tty_init(&tty, cfg.ttyport, cfg.ttyspeed, cfg.trxcntl);
#else
  tty_init(&tty, cfg.ttyport, cfg.ttyspeed);
#endif
  if (tty_open(&tty) != RC_OK)
  {
#ifdef LOG
    logw(0, "conn_init():"
           " can't open tty device %s (%s)",
           cfg.ttyport, strerror(errno));
#endif
    return RC_ERR;
  }
  state_tty_set(&tty, TTY_PAUSE);

  /* create server socket */
  if ((server_sd = sock_create_server(cfg.serveraddr, cfg.serverport, TRUE)) < 0)
  {
#ifdef LOG
    logw(0, "conn_init():"
           " can't create listen socket (%s)",
           (errno != 0) ? strerror(errno) : "failed");
#endif
    return RC_ERR;
  }

  /* connections queue initialization */
  queue_init(&queue);

  return RC_OK;
}

/*
 * Open new client connection
 * Parameters: none
 * Return: none
 */
void
conn_open(void)
{
  int sd;
  conn_t *newconn;
  struct sockaddr_storage rmt_addr;
  char ipstr[INET6_ADDRSTRLEN];

  if ((sd = sock_accept(server_sd, (struct sockaddr *)&rmt_addr,
                        sizeof(rmt_addr), TRUE)) == RC_ERR)
  { /* error in conn_accept() */
#ifdef LOG
    logw(0, "conn_open(): error in accept() (%s)", strerror(errno));
#endif
    return;
  }
 inet_ntop(rmt_addr.ss_family, sock_addr((struct sockaddr *)&rmt_addr),
           ipstr, sizeof(ipstr));
#ifdef LOG
  logw(2, "conn_open(): accepting connection from %s", ipstr);
#endif
  /* compare descriptor of connection with FD_SETSIZE */
  if (sd >= FD_SETSIZE)
  {
#ifdef LOG
    logw(1, "conn_open(): FD_SETSIZE limit reached,"
           " connection from %s will be dropped", ipstr);
#endif
    close(sd);
    return;
  }
  /* check current number of connections */
  if (queue.len == cfg.maxconn)
  {
#ifdef LOG
    logw(1, "conn_open(): number of connections limit reached,"
           " connection from %s will be dropped", ipstr);
#endif
    close(sd);
    return;
  }
  /* enqueue connection */
  newconn = queue_new_elem(&queue);
  newconn->sd = sd;
  memcpy((void *) &newconn->remote_addr, &ipstr, sizeof(ipstr));
  state_conn_set(newconn, CONN_HEADER);
}

/*
 * Close client connection
 * Parameters: CONN - ptr to connection to close
 * Return: pointer to next queue element
 */
conn_t *
conn_close(conn_t *conn)
{
  conn_t *nextconn;
#ifdef LOG
  logw(2, "conn_close(): closing connection from %s", conn->remote_addr);
#endif
  /* close socket */
  close(conn->sd);
  /* get pointer to next element */
  nextconn = queue_next_elem(&queue, conn);
  /* dequeue connection */
  queue_delete_elem(&queue, conn);
  if (actconn == conn) actconn = nextconn;
  return nextconn;
}

/*
 * Start tty transaction
 * Parameters: TTY - ptr to tty structure,
 *             CONN - ptr to active connection
 * Return: none
 */
void
conn_tty_start(ttydata_t *tty, conn_t *conn)
{
  (void)memcpy((void *)tty->txbuf,
               (void *)(conn->buf + HDRSIZE),
               MB_FRAME(conn->buf, MB_LENGTH_L));
  modbus_crc_write(tty->txbuf, MB_FRAME(conn->buf, MB_LENGTH_L));
  tty->txlen = MB_FRAME(conn->buf, MB_LENGTH_L) + CRCSIZE;
  state_tty_set(tty, TTY_RQST);
  actconn = conn;
}

/*
 * Read() wrapper. Read nomore BYTES from descriptor D in buffer BUF
 * Return: number of successfully read bytes,
 *         RC_ERR in case of error.
 */
ssize_t
conn_read(int d, void *buf, size_t nbytes)
{
  int rc;
  do
  { /* trying read from descriptor while breaked by signals */
    rc = read(d, buf, nbytes);
  } while (rc == -1 && errno == EINTR);
  return (rc < 0) ? RC_ERR : rc;
}

#include <stdio.h>
#include <stdlib.h>


/*
 * Write() wrapper. Write no more than BYTES to descriptor D from buffer BUF
 * Return: number of successfully written bytes,
 *         RC_ERR in case of error.
 */
ssize_t
conn_write(int d, void *buf, size_t nbytes, int istty)
{
  int rc;
  fd_set fs;
  struct timeval ts, tts;
  long delay;
#ifdef TRXCTL
  if (istty) {
    if (cfg.trxcntl != TRX_ADDC )
      tty_set_rts(d);
    usleep(35000000l/cfg.ttyspeed);
  }
#endif
  FD_ZERO(&fs);
  FD_SET(d, &fs);
  do
  { /* trying write to descriptor while breaked by signals */
    gettimeofday(&ts, NULL);
    rc = write(d, buf, nbytes);
  } while (rc == -1 && errno == EINTR);
  gettimeofday(&ts, NULL);


#ifdef TRXCTL
  if (istty) {
#if 1
    do {
      gettimeofday(&tts, NULL);
      delay = DV(nbytes, cfg.ttyspeed) -
        ((tts.tv_sec * 1000000l + tts.tv_usec) - (ts.tv_sec * 1000000l + ts.tv_usec));
    } while (delay > 0);
#else
    gettimeofday(&tts, NULL);
    delay = DV(nbytes, cfg.ttyspeed) -
      ((tts.tv_sec * 1000000l + tts.tv_usec) - (ts.tv_sec * 1000000l + ts.tv_usec));
    usleep(delay);
#endif
/*    tcdrain(d); - hangs sometimes, so make calculated delay */
    if (cfg.trxcntl != TRX_ADDC ) {
      tty_clr_rts(d);
    }
  }
#endif
  return (rc < 0) ? RC_ERR : rc;
}

#if 0
/*
 * Select() wrapper with signal checking.
 * Return: number number of ready descriptors,
 *         RC_ERR in case of error.
 */
int
conn_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
            struct timeval *timeout)
{
  int rc;
  do
  { /* trying to select() while breaked by signals */
    rc = select(nfds, readfds, writefds, exceptfds, timeout);
  } while (rc == -1 && errno == EINTR);
  return (rc < 0) ? RC_ERR : rc;
}
#endif

/*
 * Connections serving loop
 * Parameters: none
 * Return: none
 */
void
conn_loop(void)
{
  int rc, max_sd, len, min_timeout;
  unsigned int i;
  fd_set sdsetrd, sdsetwr;
  struct timeval ts, tts, t_out;
  unsigned long tval, tout_sec, tout = 0ul;
  conn_t *curconn = NULL;
#ifdef DEBUG
  char t[1025], v[5];
#endif

  while (TRUE)
  {
    /* check for the signals */
    if (sig_flag) sig_exec();

    /* update FD_SETs */
    FD_ZERO(&sdsetrd);
    max_sd = server_sd;
    FD_MSET(server_sd, &sdsetrd);
    FD_ZERO(&sdsetwr);

    /* update FD_SETs by TCP connections */
    len = queue.len;
    curconn = queue.beg;
    min_timeout = cfg.conntimeout;
    while (len--)
    {
      switch (curconn->state)
      {
        case CONN_HEADER:
        case CONN_RQST_FUNC:
        case CONN_RQST_NVAL:
        case CONN_RQST_TAIL:
          FD_MSET(curconn->sd, &sdsetrd);
          break;
        case CONN_RESP:
          FD_MSET(curconn->sd, &sdsetwr);
          break;
      }
      min_timeout = MIN(min_timeout, curconn->timeout);
      curconn = queue_next_elem(&queue, curconn);
    }

    /* update FD_SETs by tty connection */
    FD_MSET(tty.fd, &sdsetrd);
    if (tty.state == TTY_RQST) FD_MSET(tty.fd, &sdsetwr);

    if (tty.timer)
    { /* tty.timer is non-zero in TTY_PAUSE, TTY_RESP states */
      t_out.tv_sec = tty.timer / 1000000ul;
      t_out.tv_usec = tty.timer % 1000000ul;
    }
    else
    {
      t_out.tv_usec = 0ul;
      if (cfg.conntimeout)
        t_out.tv_sec = min_timeout; /* minor timeout value */
      else
        t_out.tv_sec = 10ul; /* XXX default timeout value */
    }

    (void)gettimeofday(&ts, NULL); /* make timestamp */

#ifdef DEBUG
    logw(7, "conn_loop(): select(): max_sd = %d, t_out = %06lu:%06lu ",
           max_sd, t_out.tv_sec, t_out.tv_usec);
#endif
    rc = select(max_sd + 1, &sdsetrd, &sdsetwr, NULL, &t_out);
#ifdef DEBUG
    logw(7, "conn_loop(): select() returns %d ", rc);
#endif
    if (rc < 0)
    { /* some error caused while select() */
      if (errno == EINTR) continue; /* process signals */
      /* unrecoverable error in select(), exiting */
#ifdef LOG
      logw(0, "conn_loop(): error in select() (%s)", strerror(errno));
#endif
/*      break; */
    }

    /* calculating elapsed time */
    (void)gettimeofday(&tts, NULL);
    tval = 1000000ul * (tts.tv_sec - ts.tv_sec) +
                       (tts.tv_usec - ts.tv_usec);

    /* modify tty timer */
    if (tty.timer)
    { /* tty timer is active */
      if (tty.timer <= tval)
        switch (tty.state)
        { /* timer expired */
          case TTY_PAUSE:
            /* inter-request pause elapsed */
            /* looking for connections in CONN_TTY state */
            curconn = state_conn_search(&queue, actconn, CONN_TTY);
            if (curconn != NULL)
              conn_tty_start(&tty, curconn);
            else
              state_tty_set(&tty, TTY_READY);
            break;
          case TTY_RESP:
          case TTY_PROC:
            /* checking for received data */
            if (FD_ISSET(tty.fd, &sdsetrd)) break;
            /* response timeout handling */
            if (!tty.ptrbuf)
            {/* there no bytes received */
#ifdef DEBUG
              logw(5, "tty: response timeout", tty.ptrbuf);
#endif
              if (!tty.trynum)
                modbus_ex_write(actconn->buf, MB_EX_TIMEOUT);
              else
              { /* retry request */
#ifdef DEBUG
                logw(5, "tty: attempt to retry request (%u of %u)",
                       cfg.maxtry - tty.trynum + 1, cfg.maxtry);
#endif
                state_tty_set(&tty, TTY_RQST);
                FD_SET(tty.fd, &sdsetwr);
                break;
              }
            }
            else
            { /* some data received */
#ifdef DEBUG
              logw(5, "tty: response read (total %d bytes, offset %d bytes)", tty.ptrbuf, tty.rxoffset);
#endif
              /* Check if there is enough data for an error response
                  and if the error flag is set in the function code */
              if ((tty.ptrbuf >= MB_ERR_LEN) &&
                  (tty.rxbuf[tty.rxoffset+TTY_FCODE_IDX] & TTY_ERR_MASK))
              {
                /* This is an error response, set the length to
		             5 (1 + 1 + 1 + 2) = Slave Address + Function Code + Error Code + CRC */
                tty.ptrbuf = MB_ERR_LEN;
              }
              if (tty.ptrbuf >= MB_MIN_LEN &&
                     modbus_crc_correct(tty.rxbuf + tty.rxoffset, tty.ptrbuf - tty.rxoffset))
              { /* received response is correct, make OpenMODBUS response */
#ifdef DEBUG
                logw(5, "tty: response is correct");
                // Optionally print the correct packet data
                t[0] = '\0';
                for (i = 0; i < tty.ptrbuf; i++) {
                  sprintf(v, "[%2.2x]", tty.rxbuf[i]);
                  strncat(t, v, 1024-strlen(t));
                }
                logw(9, "tty: response: %s", t);
#endif
                (void)memcpy((void *)(actconn->buf + HDRSIZE),
                             (void *)(tty.rxbuf + tty.rxoffset), tty.ptrbuf - CRCSIZE - tty.rxoffset);
                WORD_WR_BE(actconn->buf + MB_LENGTH_H, tty.ptrbuf - CRCSIZE - tty.rxoffset);
              }
              else
              {
                /* received response is incomplete or CRC failed */
#ifdef DEBUG
                t[0] = '\0';
                for (i = 0; i < tty.ptrbuf; i++) {
                  sprintf(v, "[%2.2x]", tty.rxbuf[i]);
                  strncat(t, v, 1024-strlen(t));
                }
                logw(5, "tty: response is incorrect: %s", t);
#endif
                if (!tty.trynum) {
                  modbus_ex_write(actconn->buf, MB_EX_CRC);
                  logw(3, "tty: response is incorrect (%d of %d bytes, offset %d), return error", tty.ptrbuf,
                    tty.rxoffset + tty.rxlen, tty.rxoffset);
                } else
                { /* retry request */
#ifdef DEBUG
                  logw(5, "tty: attempt to retry request (%u of %u)",
                         cfg.maxtry - tty.trynum + 1, cfg.maxtry);
#endif
                  state_tty_set(&tty, TTY_RQST);
                  FD_SET(tty.fd, &sdsetwr);
                  break;
                }
              }
            }
            /* switch connection to response state */
            state_conn_set(actconn, CONN_RESP);
            /* make inter-request pause */
            state_tty_set(&tty, TTY_PAUSE);
            break;
          case TTY_RQST:
#ifdef DEBUG
            logw(5, "tty: TTY_RQST timeout");
#endif
	    break;
        }
      else
        tty.timer -= tval;
    }

    if (cfg.conntimeout)
    { /* expire staled connections */
      tout += tval;
      tout_sec = tout / 1000000ul;
      if (tout_sec)
      { /* at least one second elapsed, check for staled connections */
        len = queue.len;
        curconn = queue.beg;
        while (len--)
        {
          curconn->timeout -= tout_sec;
          if (curconn->timeout <= 0)
          { /* timeout expired */
            if (curconn->state == CONN_TTY)
            { /* deadlock in CONN_TTY state, make attempt to reinitialize serial port */
#ifdef LOG
              logw(0, "conn[%s]: state CONN_TTY deadlock.", curconn->remote_addr);
#endif
              tty_reinit();
            }
            /* purge connection */
#ifdef LOG
            logw(2, "conn[%s]: timeout, closing connection", curconn->remote_addr);
#endif
            curconn = conn_close(curconn);
            continue;
          }
          curconn = queue_next_elem(&queue, curconn);
        }
        tout = tout % 1000000ul;
      }
    }

    /* checking for pending connections */
    if (FD_ISSET(server_sd, &sdsetrd)) conn_open();

    /* tty processing */
    if (tty.state == TTY_RQST)
      if (FD_ISSET(tty.fd, &sdsetwr))
      {
        tcflush(tty.fd, TCIOFLUSH);
        rc = conn_write(tty.fd, tty.txbuf + tty.ptrbuf,
                        tty.txlen - tty.ptrbuf, 1);
        if (rc <= 0)
        { /* error - make attempt to reinitialize serial port */
#ifdef LOG
          logw(0, "tty: error in write() (%s)", strerror(errno));
#endif
          tty_reinit();
        }
#ifdef DEBUG
        logw(7, "tty: written %d bytes", rc);
#endif
        tty.ptrbuf += rc;
        if (tty.ptrbuf == tty.txlen)
        { /* request transmitting completed, switch to TTY_RESP */
#ifdef DEBUG
          logw(7, "tty: request written (total %d bytes)", tty.txlen);
#endif
          state_tty_set(&tty, TTY_RESP);
          switch (tty.txbuf[1]) {
            case 1:
            case 2:
              tty.rxlen = 5 + (tty.txbuf[4] * 256 + tty.txbuf[5] + 7)/8;
              break;
            case 3:
            case 4:
              tty.rxlen = 5 + tty.txbuf[5] * 2;
              break;
            case 7:
              tty.rxlen = 5;
              break;
            case 11:
            case 15:
            case 16:
              tty.rxlen = 8;
              break;
            default:
              tty.rxlen = tty.txlen;
              break;
          }
          if (tty.rxlen > TTY_BUFSIZE)
            tty.rxlen = TTY_BUFSIZE;
          tty.timer += DV(tty.rxlen, tty.speed);
#ifdef DEBUG
          logw(5, "tty: estimated %d bytes, waiting %lu usec", tty.rxlen, tty.timer);
#endif
        }
      }

    if (FD_ISSET(tty.fd, &sdsetrd))
    {
      if (tty.state == TTY_RESP)
      {
        if (tty.rxlen - tty.ptrbuf + tty.rxoffset <= 0) {
          tcflush(tty.fd, TCIFLUSH);
          state_tty_set(&tty, TTY_PAUSE);
          continue;
        }
        rc = conn_read(tty.fd, tty.rxbuf + tty.ptrbuf,
                       tty.rxlen - tty.ptrbuf + tty.rxoffset);
        if (rc <= 0)
        { /* error - make attempt to reinitialize serial port */
#ifdef LOG
          logw(0, "tty: error in read() (%s)", rc ? strerror(errno) : "port closed");
#endif
          tty_reinit();
        }
#ifdef DEBUG
          logw(7, "tty: read %d bytes", rc);
#endif
        if (tty.ptrbuf - tty.rxoffset < 3 && tty.ptrbuf - tty.rxoffset + rc >= 3) {
          /* we received more than 3 bytes from header - address, request id and bytes count */
          if (!tty.rxoffset) {
            /* offset is unknown */
            unsigned char i;
            for (i = 0; i < tty.ptrbuf - tty.rxoffset + rc - 1; i++) {
              if (tty.rxbuf[i] == tty.txbuf[0] && tty.rxbuf[i+1] == tty.txbuf[1]) {
#ifdef DEBUG
                logw(5, "tty: rx offset is %d", i);
#endif
                tty.rxoffset = i;
                break;
              }
            }
            switch (tty.txbuf[1]) {
              case 1:
              case 2:
              case 3:
              case 4:
                i = 5 + tty.rxbuf[tty.rxoffset + 2];
                break;
              default:
                i = tty.rxlen;
                break;
            }
            if (i + tty.rxoffset > TTY_BUFSIZE)
              i = TTY_BUFSIZE - tty.rxoffset;
            if (i != tty.rxlen) {
#ifdef DEBUG
              logw(5, "tty: rx len changed from %d to %d", tty.rxlen, i);
#endif
              tty.rxlen = i;
            }
          }
        }
        tty.ptrbuf += rc;
        logw(5, "tty: read %d bytes of %d, offset %d", tty.ptrbuf, tty.rxlen + tty.rxoffset, tty.rxoffset);
        if (tty.ptrbuf == tty.rxlen + tty.rxoffset)
          state_tty_set(&tty, TTY_PROC);
      }
      else if (tty.state != TTY_PROC)
      { /* drop unexpected tty data */
        if ((rc = conn_read(tty.fd, tty.rxbuf, BUFSIZE)) <= 0)
        { /* error - make attempt to reinitialize serial port */
#ifdef LOG
          logw(0, "tty: error in read() (%s)", rc ? strerror(errno) : "port closed");
#endif
          tty_reinit();
        }
#ifdef DEBUG
        else {
          logw(7, "tty: dropped %d bytes", rc);
        }
#endif
      }
    }
    if (tty.state == TTY_PROC) {
#ifdef DEBUG
      logw(5, "tty: response read (total %d bytes, offset %d bytes)", tty.ptrbuf, tty.rxoffset);
#endif
      /* Check if there is enough data for an error response
          and if the error flag is set in the function code */
      if ((tty.ptrbuf >= MB_ERR_LEN) && (tty.rxbuf[tty.rxoffset+TTY_FCODE_IDX] & TTY_ERR_MASK))
      {
        /* This is an error response, set the length to
             5 (1 + 1 + 1 + 2) = Slave Address + Function Code + Error Code + CRC */
        tty.ptrbuf = MB_ERR_LEN;
      }
      if (tty.ptrbuf >= MB_MIN_LEN &&
         modbus_crc_correct(tty.rxbuf + tty.rxoffset, tty.ptrbuf - tty.rxoffset))
      { /* received response is correct, make OpenMODBUS response */
#ifdef DEBUG
        logw(5, "tty: response is correct");
        // Optionally print the correct packet data
        t[0] = '\0';
        for (i = 0; i < tty.ptrbuf; i++) {
          sprintf(v, "[%2.2x]", tty.rxbuf[i]);
          strncat(t, v, 1024-strlen(t));
        }
        logw(9, "tty: response: %s", t);
#endif
        (void)memcpy((void *)(actconn->buf + HDRSIZE),
                     (void *)(tty.rxbuf + tty.rxoffset), tty.ptrbuf - CRCSIZE - tty.rxoffset);
        WORD_WR_BE(actconn->buf + MB_LENGTH_H, tty.ptrbuf - CRCSIZE - tty.rxoffset);
        /* switch connection to response state */
        state_conn_set(actconn, CONN_RESP);
        /* make inter-request pause */
        state_tty_set(&tty, TTY_PAUSE);
      } else {
        /* received response is incomplete or CRC failed */
#ifdef DEBUG
        t[0] = '\0';
        for (i = 0; i < tty.ptrbuf; i++) {
          sprintf(v, "[%2.2x]", tty.rxbuf[i]);
          strncat(t, v, 1024-strlen(t));
        }
        logw(5, "tty: response is incorrect: %s", t);
#endif
        if (!tty.trynum) {
          logw(3, "tty: response is incorrect (%d of %d bytes, offset %d), return error", tty.ptrbuf,
            tty.rxoffset + tty.rxlen, tty.rxoffset);
          modbus_ex_write(actconn->buf, MB_EX_CRC);
          /* switch connection to response state */
          state_conn_set(actconn, CONN_RESP);
          /* make inter-request pause */
          state_tty_set(&tty, TTY_PAUSE);
        } else { /* retry request */
#ifdef DEBUG
          logw(5, "tty: attempt to retry request (%u of %u)",
                 cfg.maxtry - tty.trynum + 1, cfg.maxtry);
#endif
          state_tty_set(&tty, TTY_RQST);
        }
      }
    }

    if (rc == 0)
      continue;	/* timeout caused, we will do select() again */

    /* processing data on the sockets */
    len = queue.len;
    curconn = queue.beg;
    while (len--)
    {
      switch (curconn->state)
      {
        case CONN_HEADER:
        case CONN_RQST_FUNC:
        case CONN_RQST_NVAL:
        case CONN_RQST_TAIL:
          if (FD_ISSET(curconn->sd, &sdsetrd))
          {
            rc = conn_read(curconn->sd,
                           curconn->buf + curconn->ctr,
                           curconn->read_len - curconn->ctr);
            if (rc <= 0)
            { /* error - drop this connection and go to next queue element */
              curconn = conn_close(curconn);
              break;
            }
            curconn->ctr += rc;
            if (curconn->state == CONN_HEADER)
              if (curconn->ctr >= MB_UNIT_ID)
              { /* header received completely */
                if (modbus_check_header(curconn->buf) != RC_OK)
                { /* header is damaged, drop connection */
                  curconn = conn_close(curconn);
                  break;
                }
                state_conn_set(curconn, CONN_RQST_FUNC);
              }
            if (curconn->state == CONN_RQST_FUNC)
              if (curconn->ctr >= MB_DATA)
              {
                /* check request function code */
                unsigned char fc = MB_FRAME(curconn->buf, MB_FCODE);
#ifdef DEBUG
                logw(7, "conn[%s]: read request fc %d", curconn->remote_addr, fc);
#endif
                switch (fc)
                {
                  case 1: /* Read Coil Status */
                  case 2: /* Read Input Status */
                  case 3: /* Read Holding Registers */
                  case 4: /* Read Input Registers */
                  case 5: /* Force Single Coil */
                  case 6: /* Preset Single Register */
                  {
                    /* set data length for requests with fixed length */
                    conn_fix_request_header_len(curconn, 6);
                    state_conn_set(curconn, CONN_RQST_TAIL);
                  }
                    break;
                  case 15: /* Force Multiple Coils */
                  case 16: /* Preset Multiple Registers */
                    /* will read number of registers/coils to compute request data length */
                    state_conn_set(curconn, CONN_RQST_NVAL);
                    break;
                  default:
                    /* unknown function code, will rely on data length from header */
                    state_conn_set(curconn, CONN_RQST_TAIL);
                    break;
                }
              }
            if (curconn->state == CONN_RQST_NVAL)
              if (curconn->ctr >= MB_DATA_NBYTES)
              {
                /* compute request data length for fc 15/16 */
                unsigned int len;
                switch (MB_FRAME(curconn->buf, MB_FCODE))
                {
                  case 15: /* Force Multiple Coils */
                    len = 7 + (MB_FRAME(curconn->buf, MB_DATA_NVAL_H) * 256 +
                        MB_FRAME(curconn->buf, MB_DATA_NVAL_L) + 7) / 8;
                    break;
                  case 16: /* Preset Multiple Registers */
                    len = 7 + MB_FRAME(curconn->buf, MB_DATA_NVAL_L) * 2;
                    break;
                }
                if (len == 0 || len > BUFSIZE - 2)
                { /* invalid request data length, drop connection */
                  curconn = conn_close(curconn);
                  break;
                }
                conn_fix_request_header_len(curconn, len);
                state_conn_set(curconn, CONN_RQST_TAIL);
              }
            if (curconn->state == CONN_RQST_TAIL)
              if (curconn->ctr >= HDRSIZE + MB_FRAME(curconn->buf, MB_LENGTH_L))
              { /* ### frame received completely ### */
#ifdef DEBUG
                t[0] = '\0';
                for (i = MB_UNIT_ID; i < curconn->ctr; i++) {
                  sprintf(v, "[%2.2x]", curconn->buf[i]);
                  strncat(t, v, 1024-strlen(t));
                }
                logw(5, "conn[%s]: request: %s", curconn->remote_addr, t);
#endif
                state_conn_set(curconn, CONN_TTY);
                if (tty.state == TTY_READY)
                  conn_tty_start(&tty, curconn);
              }
          }
          break;
        case CONN_RESP:
          if (FD_ISSET(curconn->sd, &sdsetwr))
          {
            rc = conn_write(curconn->sd,
                            curconn->buf + curconn->ctr,
                            MB_FRAME(curconn->buf, MB_LENGTH_L) +
                            HDRSIZE - curconn->ctr, 0);
            if (rc <= 0)
            { /* error - drop this connection and go to next queue element */
              curconn = conn_close(curconn);
              break;
            }
            curconn->ctr += rc;
            if (curconn->ctr == (MB_FRAME(curconn->buf, MB_LENGTH_L) + HDRSIZE))
              state_conn_set(curconn, CONN_HEADER);
          }
          break;
      } /* switch (curconn->state) */
      curconn = queue_next_elem(&queue, curconn);
    } /* while (len--) */
  } /* while (TRUE) */

  /* XXX some cleanup must be here */
}

/*
 * Fix request header length field, if needed
 * Parameters: CONN - ptr to connection
 *             LEN - expected request data length
 * Return: none
 */
void
conn_fix_request_header_len(conn_t *conn, unsigned char len)
{
  if (MB_FRAME(conn->buf, MB_LENGTH_L) != len)
  {
#ifdef DEBUG
    logw(5, "conn[%s]: request data len changed from %d to %d",
         MB_FRAME(conn->buf, MB_LENGTH_L), len);
#endif
    MB_FRAME(conn->buf, MB_LENGTH_L) = len;
  }
}

