/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * sock.c - socket manipulation routines
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
 * $Id: sock.c,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */

#include "sock.h"

/*
 * Bring i/o descriptor SD to BLKMODE (if non-zero - socket is nonblocking)
 *
 * Return: RC_ERR if there some errors
 */
int
sock_set_blkmode(int sd, int blkmode)
{
  int flags;

  flags = fcntl(sd, F_GETFL);
  if (flags == -1) return -1;

  flags = blkmode ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
  flags = fcntl(sd, F_SETFL, flags);

  return flags;
}

/*
 * Create new socket in BLKMODE mode (if non-zero - socket is nonblocking)
 *
 * Return: socket descriptor, otherwise RC_ERR if there some errors
 */
int
sock_create(int blkmode)
{
  int sock;

  if ((sock =
         socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
#ifdef LOG
    logw(0, "sock_create(): unable to create socket (%s)",
        strerror(errno));
#endif
    return RC_ERR;
  }

  /* set socket to desired blocking mode */
  if (sock_set_blkmode(sock, blkmode) == -1)
  {
#ifdef LOG
    logw(0, "sock_create(): unable to set "
           "server socket to nonblocking (%s)",
           strerror(errno));
#endif
    return RC_ERR;
  }

  return sock;
}

/*
 * Create new server socket with SERVER_PORT, SERVER_IP -
 * port and address to bind the socket,
 * BLKMODE - blocking mode (if non-zero - socket is nonblocking)
 *
 * Return: socket descriptor, otherwise RC_ERR if there some errors
 */
int
sock_create_server(char *server_ip,
  unsigned short server_port, int blkmode)
{
  struct sockaddr_in server_sockaddr;
  int sock_opt = 1;
  int server_s;

  /* create socket in desired blocking mode */
  server_s = sock_create(blkmode);
  if (server_s < 0) return server_s;

  /* set to close socket on exec() */
  if (fcntl(server_s, F_SETFD, 1) == -1)
  {
#ifdef LOG
    logw(0, "sock_create_server():"
           " can't set close-on-exec on socket (%s)",
           strerror(errno));
#endif
    return RC_ERR;
  }
  /* set reuse socket address */
  if (setsockopt(server_s, SOL_SOCKET,
                  SO_REUSEADDR, (void *)&sock_opt,
		          sizeof(sock_opt)) == -1)
  {
#ifdef LOG
    logw(0, "sock_create_server():"
           " can't set socket to SO_REUSEADDR (%s)",
           strerror(errno));
#endif
    return RC_ERR;
  }
  /* adjust socket rx and tx buffer sizes */
  sock_opt = SOCKBUFSIZE;
  if ((setsockopt(server_s, SOL_SOCKET,
                  SO_SNDBUF, (void *)&sock_opt,
		          sizeof(sock_opt)) == -1) ||
      (setsockopt(server_s, SOL_SOCKET,
                  SO_RCVBUF, (void *)&sock_opt,
		          sizeof(sock_opt)) == -1))
  {
#ifdef LOG
    logw(0, "sock_create_server():"
           " can't set socket TRX buffers sizes (%s)",
           strerror(errno));
#endif
    return RC_ERR;
  }

  memset(&server_sockaddr, 0, sizeof(server_sockaddr));
  server_sockaddr.sin_family = AF_INET;

  if (server_ip != NULL)
    inet_aton(server_ip, &server_sockaddr.sin_addr);
  else
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  server_sockaddr.sin_port = htons(server_port);

  if (bind(server_s, (struct sockaddr *) & server_sockaddr,
	   sizeof(server_sockaddr)) == -1)
  {
#ifdef LOG
    logw(0, "sock_create_server():"
           " unable to bind() socket (%s)",
           strerror(errno));
#endif
    return RC_ERR;
  }

  /* let's listen */
  if (listen(server_s, BACKLOG) == -1)
  {
#ifdef LOG
    logw(0, "sock_create_server():"
           " unable to listen() on socket (%s)",
           strerror(errno));
#endif
    exit(errno);
  }
  return server_s;
}

/*
 * Accept connection from SERVER_SD - server socket descriptor
 * and create socket in BLKMODE blocking mode
 * (if non-zero - socket is nonblocking)
 *
 * Return: socket descriptor, otherwise RC_ERR if there some errors;
 *         RMT_ADDR - ptr to connection info structure
 */
int
sock_accept(int server_sd, struct sockaddr_in *rmt_addr, int blkmode)
{
  int sd, sock_opt = SOCKBUFSIZE;
  int rmt_len = sizeof(struct sockaddr_in);

  sd = accept(server_sd, (struct sockaddr *) rmt_addr,
              (socklen_t *) &rmt_len);
  if (sd == -1)
  {
    if (errno != EAGAIN && errno != EWOULDBLOCK)
      /* some errors caused */
#ifdef LOG
      logw(0, "sock_accept(): error in accept() (%s)", strerror(errno));
#endif
    return RC_ERR;
  }
  /* tune socket */
  if (sock_set_blkmode(sd, blkmode) == RC_ERR)
  {
#ifdef LOG
    logw(0, "sock_accept(): can't set socket blocking mode (%s)", 
           strerror(errno));
#endif
    close(sd);
    return RC_ERR;
  }
  /* adjust socket rx and tx buffer sizes */
  if ((setsockopt(sd, SOL_SOCKET,
                  SO_SNDBUF, (void *)&sock_opt,
		          sizeof(sock_opt)) == -1) ||
      (setsockopt(sd, SOL_SOCKET,
                  SO_RCVBUF, (void *)&sock_opt,
		          sizeof(sock_opt)) == -1))
  {
#ifdef LOG
    logw(0, "sock_accept():"
           " can't set socket TRX buffer sizes (%s)",
           strerror(errno));
#endif
    return RC_ERR;
  }
  return sd;
}
