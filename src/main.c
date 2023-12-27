/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * main.c - main module
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
 * $Id: main.c,v 1.6 2015/02/25 10:33:57 kapyar Exp $
 */

#include "globals.h"
#include "string.h"
#include "errno.h"
#include "cfg.h"
#include "tty.h"
#include "sock.h"
#include "conn.h"
#include "queue.h"
#include "sig.h"
#include "util.h"
#ifdef LOG
#  include "log.h"
#endif

extern char logfullname[];
int isdaemon = TRUE;

void usage(char *exename);

/* Server socket */
int server_sd = -1;
/* TTY related data storage variable */
ttydata_t tty;
/* Connections queue descriptor */
queue_t queue;

#ifndef HAVE_DAEMON
#include <fcntl.h>
#include <unistd.h>
/*
 * System function daemon() replacement based on FreeBSD implementation.
 * Original source file CVS tag:
 * $FreeBSD: src/lib/libc/gen/daemon.c,v 1.3 2000/01/27 23:06:14 jasone Exp $
 */
int
daemon(nochdir, noclose)
  int nochdir, noclose;
{
  int fd;

  switch (fork()) {
    case -1:
      return (-1);
    case 0:
      break;
    default:
      _exit(0);
  }

  if (setsid() == -1)
    return (-1);

  if (!nochdir)
    (void)chdir("/");

  if (!noclose && (fd = open("/dev/null", O_RDWR, 0)) != -1) {
    (void)dup2(fd, STDIN_FILENO);
    (void)dup2(fd, STDOUT_FILENO);
    (void)dup2(fd, STDERR_FILENO);
    if (fd > 2)
      (void)close(fd);
  }
  return (0);
}
#endif

void
usage(char *exename)
{
  cfg_init();
  printf("%s-%s Copyright (C) 2002-2003, 2011, 2013-2023 Victor Antonovich <v.antonovich@gmail.com>, "
   "Andrew Denysenko <nitr0@seti.kr.ua>\n\n"
   "Usage: %s [-h] [-d] "
#ifdef LOG
   "[-L logfile] [-v level] "
#endif
   "[-c cfgfile] \n"
   "             [-p device] [-s speed] [-m mode]"
#ifdef HAVE_TIOCRS485
   " [-S]"
#endif
   "\n"   
#ifdef TRXCTL
   "             [-t] [-r] [-y sysfsfile] [-Y sysfsfile]\n"
#endif
   "             [-A address] [-P port] [-C maxconn] [-N retries]\n"
   "             [-R pause] [-W wait] [-T timeout] [-b]\n\n"
   "Options:\n"
   "  -h         : this help\n"
   "  -d         : don't fork (non-daemonize)\n"
#ifdef LOG
   "  -L logfile : set log file name (default is %s%s, \n"
   "               '-' for logging to STDOUT only)\n"
#ifdef DEBUG
   "  -v level   : set log level (0-9, default is %d, 0 - errors only)\n"
#else
   "  -v level   : set log level (0-2, default %d, 0 - errors only)\n"
#endif
#endif
   "  -c cfgfile : read configuration from cfgfile\n"
   "  -p device  : set serial port device name (default is %s)\n"
   "  -s speed   : set serial port speed (default is %d)\n"
   "  -m mode    : set serial port mode (default is %s)\n"
#ifdef HAVE_TIOCRS485
   "  -S         : enable Linux RS-485 support for given serial port device\n"
#endif   
#ifdef TRXCTL
   "  -t         : enable RTS RS-485 data direction control using RTS, active transmit\n"
   "  -r         : enable RTS RS-485 data direction control using RTS, active receive\n"
   "  -y         : enable RTS RS-485 data direction control using sysfs file, active transmit\n"
   "               (writes '1' to sysfs file for transmit enable, '0' for transmit disable)\n"
   "  -Y         : enable RTS RS-485 data direction control using sysfs file, active receive\n"
   "               (writes '0' to sysfs file for transmit enable, '1' for transmit disable)\n"
#endif
   "  -A address : set TCP server address to bind (default is %s)\n"
   "  -P port    : set TCP server port number (default is %d)\n"
   "  -C maxconn : set maximum number of simultaneous TCP connections\n"
   "               (1-%d, default is %d)\n"
   "  -N retries : set maximum number of request retries\n"
   "               (0-%d, default is %d, 0 disables retrying)\n"
   "  -R pause   : set pause between requests in milliseconds\n"
   "               (1-%d, default is %lu)\n"
   "  -W wait    : set response wait time in milliseconds\n"
   "               (1-%d, default is %lu)\n"
   "  -T timeout : set connection timeout value in seconds\n"
   "               (0-%d, default is %d, 0 disables timeout)\n"
   "  -b         : enable reply on broadcast"
   "\n", PACKAGE, VERSION, exename,
#ifdef LOG
      LOGPATH, LOGNAME, cfg.dbglvl,
#endif
      cfg.ttyport, cfg.ttyspeed, cfg.ttymode,
      cfg.serveraddr, cfg.serverport,
      MAX_MAXCONN, cfg.maxconn, MAX_MAXTRY, cfg.maxtry,
      MAX_RQSTPAUSE, cfg.rqstpause, MAX_RESPWAIT, cfg.respwait,
      MAX_CONNTIMEOUT, cfg.conntimeout);
  exit(0);
}


int
main(int argc, char *argv[])
{
  int err = 0, rc, err_line;
  char *exename;
  char ttyparity;
  char *end;
  char *logfilenamevalue;
  char *logfilename;

  sig_init();

  cfg_init();

  if ((exename = strrchr(argv[0], '/')) == NULL)
    exename = argv[0];
  else
    exename++;

  /* command line argument list parsing */
  while ((rc = getopt(argc, argv,
               "dh"
#ifdef TRXCTL
               "ty:Y:"
#endif
#ifdef HAVE_TIOCRS485
               "S"
#endif
#ifdef LOG
               "v:L:"
#endif
               "p:s:m:A:P:C:N:R:W:T:c:b")) != RC_ERR)
  {
    switch (rc)
    {
      case '?':
        exit(-1);
      case 'd':
        isdaemon = FALSE;
        break;
      case 'c':
        if ((err_line = cfg_read_file(optarg)) != 0)
        {
          if (err_line > 0)
            printf("%s: can't read config file %s: error at line %d: %s\n",
                   exename, optarg, err_line, cfg_err);
          else
            printf("%s: can't read config file %s: %s\n",
                   exename, optarg, strerror(errno));
          exit(-1);
        }
        break;
#ifdef TRXCTL
      case 't':
        cfg.trxcntl = TRX_RTS_1;
        break;
      case 'r':
        cfg.trxcntl = TRX_RTS_0;
        break;
      case 'y':
        cfg.trxcntl = TRX_SYSFS_1;
        strncpy(cfg.trxcntl_file, optarg, INTBUFSIZE);
	      break;
      case 'Y':
        cfg.trxcntl = TRX_SYSFS_0;
        strncpy(cfg.trxcntl_file, optarg, INTBUFSIZE);
	      break;
#endif
#ifdef HAVE_TIOCRS485
      case 'S':
        cfg.rs485 = TRUE;
	      break;
#endif
#ifdef LOG
      case 'v':
        cfg.dbglvl = (char)strtol(optarg, NULL, 0);
#  ifdef DEBUG
        if (!(isdigit(*optarg)) || cfg.dbglvl < 0 || cfg.dbglvl > 9)
        { /* report about invalid log level */
          printf("%s: -v: invalid loglevel value"
                 " (%s, must be 0-9)\n", exename, optarg);
#  else
        if (!(isdigit(*optarg)) || cfg.dbglvl < 0 || cfg.dbglvl > 2)
        { /* report about invalid log level */
          printf("%s: -v: invalid loglevel value"
                 " (%s, must be 0-2)\n", exename, optarg);
#  endif
          exit(-1);
        }
        break;
      case 'L':
        logfilenamevalue = strdup(optarg);
        logfilename = util_trim(logfilenamevalue);
        if (!strlen(logfilename))
        { /* report about empty log file */
          printf("%s: -L: log file name is empty, exiting...\n", exename);
          exit(-1);
        }
        else if (*logfilename != '/')
        {
          if (*logfilename == '-')
          {
            /* logging to file disabled */
            *cfg.logname = '\0';
          }
          else
          { /* concatenate given log file name with default path */
            strncpy(cfg.logname, LOGPATH, INTBUFSIZE);
            strncat(cfg.logname, logfilename, INTBUFSIZE - strlen(cfg.logname));
          }
        }
        else strncpy(cfg.logname, logfilename, INTBUFSIZE);
        free(logfilenamevalue);
        break;
#endif
      case 'p':
        if (*optarg != '/')
        { /* concatenate given port name with default
             path to devices mountpoint */
          strncpy(cfg.ttyport, "/dev/", INTBUFSIZE);
          strncat(cfg.ttyport, optarg, INTBUFSIZE - strlen(cfg.ttyport));
        }
        else strncpy(cfg.ttyport, optarg, INTBUFSIZE);
        break;
      case 's':
        cfg.ttyspeed = strtoul(optarg, &end, 10);
        if (!cfg.ttyspeed || optarg == end || '\0' != *end)
        {
          printf("%s: -s: invalid serial port speed (%s)\n", exename, optarg);
          exit(-1);
        }
        break;
      case 'm':
        strncpy(cfg.ttymode, optarg, INTBUFSIZE);
        /* tty mode sanity checks */
        if (strlen(cfg.ttymode) != 3)
        {
          printf("%s: -m: invalid serial port mode ('%s')\n",
                 exename, cfg.ttymode);
          exit(-1);
        }
        if (cfg.ttymode[0] != '8')
        {
          printf("%s: -m: invalid serial port character size "
              "(%c, must be 8)\n",
              exename, cfg.ttymode[0]);
          exit(-1);
        }
        ttyparity = toupper(cfg.ttymode[1]);
        if (ttyparity != 'N' && ttyparity != 'E' && ttyparity != 'O')
        {
          printf("%s: -m: invalid serial port parity "
              "(%c, must be N, E or O)\n", exename, ttyparity);
          exit(-1);
        }
        if (cfg.ttymode[2] != '1' && cfg.ttymode[2] != '2')
        {
          printf("%s: -m: invalid serial port stop bits "
              "(%c, must be 1 or 2)\n", exename, cfg.ttymode[2]);
          exit(-1);
        }
        break;
      case 'A':
        strncpy(cfg.serveraddr, optarg, INTBUFSIZE);
        break;
      case 'P':
        cfg.serverport = strtoul(optarg, NULL, 0);
        break;
      case 'C':
        cfg.maxconn = strtoul(optarg, NULL, 0);
        if (cfg.maxconn < 1 || cfg.maxconn > MAX_MAXCONN)
        { /* report about invalid max conn number */
          printf("%s: -C: invalid maxconn value"
                 " (%d, must be 1-%d)\n", exename, cfg.maxconn, MAX_MAXCONN);
          exit(-1);
        }
        break;
      case 'N':
        cfg.maxtry = strtoul(optarg, NULL, 0);
        if (cfg.maxtry > MAX_MAXTRY)
        { /* report about invalid max try number */
          printf("%s: -N: invalid maxtry value"
                 " (%d, must be 0-%d)\n", exename, cfg.maxtry, MAX_MAXTRY);
          exit(-1);
        }
        break;
      case 'R':
        cfg.rqstpause = strtoul(optarg, NULL, 0);
        if (cfg.rqstpause < 1 || cfg.rqstpause > MAX_RQSTPAUSE)
        { /* report about invalid rqst pause value */
          printf("%s: -R: invalid inter-request pause value"
                 " (%lu, must be 1-%d)\n", exename, cfg.rqstpause, MAX_RQSTPAUSE);
          exit(-1);
        }
        break;
      case 'W':
        cfg.respwait = strtoul(optarg, NULL, 0);
        if (cfg.respwait < 1 || cfg.respwait > MAX_RESPWAIT)
        { /* report about invalid resp wait value */
          printf("%s: -W: invalid response wait time value"
                 " (%lu, must be 1-%d)\n", exename, cfg.respwait, MAX_RESPWAIT);
          exit(-1);
        }
        break;
      case 'T':
        cfg.conntimeout = strtoul(optarg, NULL, 0);
        if (cfg.conntimeout > MAX_CONNTIMEOUT)
        { /* report about invalid conn timeout value */
          printf("%s: -T: invalid conn timeout value"
                 " (%d, must be 1-%d)\n", exename, cfg.conntimeout, MAX_CONNTIMEOUT);
          exit(-1);
        }
        break;
      case 'b':
        cfg.replyonbroadcast = 1;
        break;
      case 'h':
        usage(exename);
        break;
    }
  }

#ifdef LOG
  if (log_init(cfg.logname) != RC_OK)
  {
    printf("%s: can't open logfile '%s' (%s), exiting...\n",
           exename,
           logfullname[0] ? logfullname : "no log name was given",
           strerror(errno));
    exit(-1);
  }
  logw(2, "%s-%s started...", PACKAGE, VERSION);
#endif

  if (conn_init())
  {
#ifdef LOG
    err = errno;
    logw(2, "conn_init() failed, exiting...");
#endif
    exit(err);
  }

  /* go or not to daemon mode? */
  if (isdaemon && (rc = daemon(TRUE, FALSE)))
  {
#ifdef LOG
    logw(0, "Can't be daemonized (%s), exiting...", strerror(errno));
#endif
    exit(rc);
  }

  conn_loop();
  err = errno;
#ifdef LOG
  logw(2, "%s-%s exited...", PACKAGE, VERSION);
#endif
  return (err);
}
