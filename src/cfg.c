/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * cfg.c - configuration related procedures
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
 * $Id: cfg.c,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */

#include "cfg.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define CFG_MAX_LINE_LENGTH 200

#define CFG_NAME_MATCH(n) strcmp(n, name) == 0
#define CFG_VALUE_MATCH(n) strcasecmp(n, value) == 0

/* Global configuration storage variable */
cfg_t cfg;

/* Configuration error message */
char cfg_err[INTBUFSIZE + 1];

#define CFG_ERR(s, v) snprintf(cfg_err, INTBUFSIZE, s, v)

/*
 * Setting up config defaults
 */
void
cfg_init(void)
{
#ifdef LOG
  cfg.dbglvl = 2;
  strncpy(cfg.logname, LOGNAME, INTBUFSIZE);
#endif
  strncpy(cfg.ttyport, DEFAULT_PORT, INTBUFSIZE);
  cfg.ttyspeed = DEFAULT_SPEED;
  strncpy(cfg.ttymode, DEFAULT_MODE, INTBUFSIZE);
#ifdef TRXCTL
  cfg.trxcntl = TRX_ADDC;
  *cfg.trxcntl_file = '\0';
#endif
  strncpy(cfg.serveraddr, DEFAULT_SERVERADDR, INTBUFSIZE);
  cfg.serverport = DEFAULT_SERVERPORT;
  cfg.maxconn = DEFAULT_MAXCONN;
  cfg.maxtry = DEFAULT_MAXTRY;
  cfg.rqstpause = DEFAULT_RQSTPAUSE;
  cfg.respwait = DEFAULT_RESPWAIT;
  cfg.resppause = DV(3, cfg.ttyspeed);
  cfg.conntimeout = DEFAULT_CONNTIMEOUT;
}

static char *
cfg_rtrim(char *s)
{
  char *p = s + strlen(s);
  while (p > s && isspace((unsigned char )(*--p)))
    *p = '\0';
  return s;
}

static char *
cfg_ltrim(const char *s)
{
  while (*s && isspace((unsigned char )(*s)))
    s++;
  return (char *) s;
}

int
cfg_handle_param(char *name, char *value)
{
  if (CFG_NAME_MATCH("device"))
  {
    strncpy(cfg.ttyport, value, INTBUFSIZE);
  }
  else if (CFG_NAME_MATCH("speed"))
  {
    cfg.ttyspeed = strtoul(value, NULL, 0);
  }
  else if (CFG_NAME_MATCH("mode"))
  {
    int mode_invalid;
    if (strlen(value) != 3)
      mode_invalid = 1;
    else
    {
      char parity = toupper(value[1]);
      mode_invalid = value[0] != '8' || (value[2] != '1' && value[2] != '2') ||
          (parity != 'N' && parity != 'E' && parity != 'O');
    }
    if (mode_invalid)
    {
      CFG_ERR("invalid device mode: %s", value);
      return 0;
    }
    strncpy(cfg.ttymode, value, INTBUFSIZE);
  }
  else if (CFG_NAME_MATCH("address"))
  {
    strncpy(cfg.serveraddr, value, INTBUFSIZE);
  }
  else if (CFG_NAME_MATCH("port"))
  {
    cfg.serverport = strtoul(value, NULL, 0);
  }
  else if (CFG_NAME_MATCH("maxconn"))
  {
    cfg.maxconn = strtoul(value, NULL, 0);
    if (cfg.maxconn < 1 || cfg.maxconn > MAX_MAXCONN)
    {
      CFG_ERR("invalid maxconn value: %s", value);
      return 0;
    }
  }
  else if (CFG_NAME_MATCH("retries"))
  {
    cfg.maxtry = strtoul(value, NULL, 0);
    if (cfg.maxtry > MAX_MAXTRY)
    {
      CFG_ERR("invalid retries value: %s", value);
      return 0;
    }
  }
  else if (CFG_NAME_MATCH("pause"))
  {
    cfg.rqstpause = strtoul(value, NULL, 0);
    if (cfg.rqstpause < 1 || cfg.rqstpause > MAX_RQSTPAUSE)
    {
      CFG_ERR("invalid pause value: %s", value);
      return 0;
    }
  }
  else if (CFG_NAME_MATCH("wait"))
  {
    cfg.respwait = strtoul(value, NULL, 0);
    if (cfg.respwait < 1 || cfg.respwait > MAX_RESPWAIT)
    {
      CFG_ERR("invalid wait value: %s", value);
      return 0;
    }
  }
  else if (CFG_NAME_MATCH("timeout"))
  {
    cfg.conntimeout = strtoul(value, NULL, 0);
    if (cfg.conntimeout > MAX_CONNTIMEOUT)
      return 0;
#ifdef TRXCTL
  }
  else if (CFG_NAME_MATCH("trx_control"))
  {
    if (CFG_VALUE_MATCH("addc"))
    {
      cfg.trxcntl = TRX_ADDC;
    }
    else if (CFG_VALUE_MATCH("rts"))
    {
      cfg.trxcntl = TRX_RTS;
    }
    else if (CFG_VALUE_MATCH("sysfs_0"))
    {
      cfg.trxcntl = TRX_SYSFS_0;
    }
    else if (CFG_VALUE_MATCH("sysfs_1"))
    {
      cfg.trxcntl = TRX_SYSFS_1;
    }
    else
    {
      /* Unknown TRX control mode */
      CFG_ERR("unknown trx control mode: %s", value);
      return 0;
    }
  }
  else if (CFG_NAME_MATCH("trx_sysfile"))
  {
    strncpy(cfg.trxcntl_file, value, INTBUFSIZE);
#endif
#ifdef LOG
  }
  else if (CFG_NAME_MATCH("loglevel"))
  {
    cfg.dbglvl = (char)strtol(optarg, NULL, 0);
#endif
  }
  else {
    /* Unknown parameter name */
    CFG_ERR("unknown parameter: %s", name);
    return 0;
  }
  return 1;
}

int
cfg_parse_file(void *file)
{
  char *line;
  char *start;
  char *end;
  char *name;
  char *value;
  int lineno = 0;
  int error = 0;

  *cfg_err = '\0';

  line = (char *) malloc(CFG_MAX_LINE_LENGTH);
  if (!line)
  {
    return -1;
  }

  while (fgets(line, CFG_MAX_LINE_LENGTH, file) != NULL)
  {
    lineno++;

    start = cfg_ltrim(cfg_rtrim(line));

    if (*start == '#')
    {
      /* skip comment */
      continue;
    }
    else if (*start)
    {
      /* parse `name=value` pair */
      for (end = start; *end && *end != '='; end++);
      if (*end == '=')
      {
        *end = '\0';

        name = cfg_rtrim(start);
        value = cfg_ltrim(cfg_rtrim(end + 1));

        /* handle name/value pair */
        if (!cfg_handle_param(name, value))
        {
          error = lineno;
          break;
        }
      }
      else
      {
        /* no '=' found on config line */
        error = lineno;
        CFG_ERR("can't parse line: %s", start);
        break;
      }
    }
  }

  free(line);

  return error;
}

int
cfg_read_file(const char *filename)
{
  FILE* file;
  int error;

  file = fopen(filename, "r");
  if (!file)
    return -1;
  error = cfg_parse_file(file);
  fclose(file);
  return error;
}
