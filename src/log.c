/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * log.c - debug logging facility
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
 * $Id: log.c,v 1.6 2015/02/25 10:33:57 kapyar Exp $
 */

#include "log.h"

#ifdef LOG
#include "cfg.h"

/* log file full name */
char logfullname[INTBUFSIZE + 1];

int
log_init(char *logname)
{
  FILE *logfile;
  int maxlen = INTBUFSIZE;

  /* checking log file name */
  if (*logname == '/')
    strncpy(logfullname, logname, maxlen);
  else
  {
    if (!*logname)
    {
      /* logfile isn't needed */
      *logfullname = '\0';
      return RC_OK;
    }
    /* append default log path */
    strncpy(logfullname, LOGPATH, maxlen);
    maxlen -= strlen(logfullname);
    strncat(logfullname, logname, maxlen);
  }

  logfile = fopen(logfullname, "at");
  if (logfile)
  {
    fclose(logfile);
    return RC_OK;
  }
  return RC_ERR;
}

/* Append message STRING to log file LOGNAME */
int
log_app(char *logname, char *string)
{
  FILE *logfile;
  logfile = fopen(logname, "at");
  if (logfile)
  {
    fputs(string, logfile);
    fclose(logfile);
    return RC_OK;
  }
  return RC_ERR;
}

/* Put message with format FMT with errorlevel LEVEL to log file */
void
logw(int level, char *fmt, ...)
{
#ifdef HRDATE
  time_t tt;
  struct tm *t;
#else
  struct timeval tv;
#endif
  va_list args;
  int strsize = 0;
  static char str[INTBUFSIZE + 1] = {0}, *p;

  if (level > cfg.dbglvl) return;
#ifdef HRDATE
  tt = time(NULL);
  t = localtime(&tt);
  strsize += strftime(str, 32, "%d %b %Y %H:%M:%S ", t);
#else
  (void)gettimeofday(&tv, NULL);
  strsize += snprintf(str, 32, "%06lu:%06lu ", tv.tv_sec, tv.tv_usec);
#endif
  va_start(args, fmt);
  p = str + strsize;
  strsize += vsnprintf(p, INTBUFSIZE - strsize, fmt, args);
  va_end(args);
  strcpy(str + strsize++, "\n");
  if (!isdaemon) fprintf(stderr, "%s", str);
  if (*logfullname == '\0') return;
  log_app(logfullname, str);
}
#endif
