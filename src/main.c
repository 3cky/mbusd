/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * main.c - main module
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
 * $Id: main.c,v 1.1 2003/09/13 20:38:16 kapyar Exp $
 */

#include "globals.h"
#include "cfg.h"
#include "tty.h"
#include "sock.h"
#include "conn.h"
#include "queue.h"
#include "sig.h"
#ifdef LOG
#include "log.h"
#endif

int isdaemon = TRUE;

void usage(char *exename);

/* Server socket */
int server_sd = -1;
/* TTY related data storage variable */
ttydata_t tty;
/* Connections queue descriptor */
queue_t queue;

void
usage(char *exename)
{
  printf("%s-%s copyright (c) Victor Antonovich, 2002-2003 // 2:5055/124.13@fidonet\n"
	 "usage: %s [<options>] \n"
	 "-h             this help\n"
	 "-d             don't daemonize\n"
#ifdef LOG
#ifdef DEBUG
     "-v<level>      set log level (0-9, default %d, 0 - errors only)\n"
#else
     "-v<level>      set log level (0-2, default %d, 0 - errors only)\n"
#endif
     "-L<name>       set log file name (default %s%s)\n"
#endif
	 "-p<name>       set serial port device name (default %s)\n"
	 "-s<value>      set serial port speed (default %d)\n"
#ifdef TRXCTL
	 "-t             force RTS RS-485 transmitting/receiving control\n"
#endif
	 "-P<number>     set TCP server port number (default %d)\n"
     "-C<number>     set number of simultaneous connections\n"
     "               (1-128, default %d)\n"
     "-N<number>     set number of request retries\n"
     "               (0-15, default %d, 0 - without retries)\n"
     "-R<value>      set pause between requests, millisecs\n"
     "               (1-10000, default %lu)\n"
     "-W<value>      set response wait time, millisecs\n"
     "               (1-10000, default %lu)\n"
     "-T<value>      set connection timeout value, secs\n"
     "               (0-1000, default %d, 0 - no timeout)"
	 "\n", PACKAGE, VERSION, exename,
#ifdef LOG
      cfg.dbglvl, LOGPATH, LOGNAME,
#endif
      cfg.ttyport, cfg.ttyspeed, cfg.serverport, cfg.maxconn,
      cfg.maxtry, cfg.rqstpause, cfg.respwait, cfg.conntimeout);
  exit(0);
}

int
main(int argc, char *argv[])
{
  int err = 0, rc;
  sig_init();
  cfg_init();

  /* command line parsing */
  while ((rc = getopt(argc, argv,
               "dh"
#ifdef TRXCTL
               "t"
#endif
#ifdef LOG
               "v:L:"
#endif
               "p:s:P:C:N:R:W:T:")) != RC_ERR)
  {
    switch (rc)
    {
      case 'd':
        isdaemon = FALSE;
        break;
#ifdef TRXCTL
      case 't':
        cfg.trxcntl = TRX_RTS;
        break;
#endif
#ifdef LOG
      case 'v':
        cfg.dbglvl = (char)strtol(optarg, NULL, 0);
#ifdef DEBUG
        if (cfg.dbglvl < 0 || cfg.dbglvl > 9)
        { /* report about invalid log level */
          printf("-v: invalid loglevel"
                 " (%d, must be 0-9)\n", cfg.dbglvl);
#else
        if (cfg.dbglvl < 0 || cfg.dbglvl > 9)
        { /* report about invalid log level */
          printf("-v: invalid loglevel"
                 " (%d, must be 0-2)\n", cfg.dbglvl);
#endif
          exit(-1);
        }
        break;
      case 'L':
        if (*optarg != '/') 
        { /* concatenate given log file name with default path */
          strncpy(cfg.logname, LOGPATH, INTBUFSIZE);
          strncat(cfg.logname, optarg,
                  INTBUFSIZE - strlen(cfg.logname));
        }
        else strncpy(cfg.logname, optarg, INTBUFSIZE);
        break;
#endif
      case 'p':
        if (*optarg != '/') 
        { /* concatenate given port name with default */
          /* path to devices mountpoint */
          strncpy(cfg.ttyport, "/dev/", INTBUFSIZE);
          strncat(cfg.ttyport, optarg,
                  INTBUFSIZE - strlen(cfg.ttyport));
        }
        else strncpy(cfg.ttyport, optarg, INTBUFSIZE);
        break;
      case 's':
        cfg.ttyspeed = strtoul(optarg, NULL, 0);
        break;
      case 'P':
        cfg.serverport = strtoul(optarg, NULL, 0);
        break;
      case 'C':
        cfg.maxconn = strtoul(optarg, NULL, 0);
        if (cfg.maxconn < 1 || cfg.maxconn > 128)
        { /* report about invalid max conn number */
          printf("-C: invalid number"
                 " (%d, must be 1-128)\n", cfg.maxconn);
          exit(-1);
        }
        break;
      case 'N':
        cfg.maxtry = strtoul(optarg, NULL, 0);
        if (cfg.maxtry > 15)
        { /* report about invalid max conn number */
          printf("-N: invalid number"
                 " (%d, must be 0-15)\n", cfg.maxtry);
          exit(-1);
        }
        break;
      case 'R':
        cfg.rqstpause = strtoul(optarg, NULL, 0);
        if (cfg.rqstpause < 1 || cfg.rqstpause > 10000)
        { /* report about invalid rqst pause value */
          printf("-R: invalid number"
                 " (%lu, must be 1-10000)\n", cfg.rqstpause);
          exit(-1);
        }
        break;
      case 'W':
        cfg.respwait = strtoul(optarg, NULL, 0);
        if (cfg.respwait < 1 || cfg.respwait > 10000)
        { /* report about invalid resp wait value */
          printf("-W: invalid number"
                 " (%lu, must be 1-10000)\n", cfg.respwait);
          exit(-1);
        }
        break;
      case 'T':
        cfg.conntimeout = strtoul(optarg, NULL, 0);
        if (cfg.conntimeout > 1000)
        { /* report about invalid conn timeout value */
          printf("-T: invalid number"
                 " (%d, must be 1-1000)\n", cfg.conntimeout);
          exit(-1);
        }
        break;
      case 'h':
        usage(argv[0]);
    }
  }

#ifdef LOG
  log_init(cfg.logname);
  log(2, "%s-%s started...", PACKAGE, VERSION);
#endif

  if (conn_init())
  {
#ifdef LOG
    err = errno;
    log(2, "conn_init() failed, exiting...");
#endif
    exit(err);
  }

  /* go or not to daemon mode? */
  if (isdaemon && (rc = daemon(TRUE, FALSE)))
  {
#ifdef LOG
    log(0, "Can't be daemonized (%s), exiting...", strerror(errno));
#endif
    exit(rc);
  }

  conn_loop();
  err = errno;
#ifdef LOG
  log(2, "%s-%s exited...", PACKAGE, VERSION);
#endif
  return (err);
}

