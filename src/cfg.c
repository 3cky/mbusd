/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * cfg.c - configuration related procedures
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
 * $Id: cfg.c,v 1.1 2003/09/13 20:38:11 kapyar Exp $
 */

#include "cfg.h"

/* Global configuration storage variable */
cfg_t cfg;

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
#ifdef  TRXCTL
  cfg.trxcntl = TRX_ADDC;
#endif
  cfg.serverport = DEFAULT_SERVERPORT;
  cfg.maxconn = DEFAULT_MAXCONN;
  cfg.maxtry = DEFAULT_MAXTRY;
  cfg.rqstpause = DEFAULT_RQSTPAUSE;
  cfg.respwait = DEFAULT_RESPWAIT;
  cfg.resppause = DV(3, cfg.ttyspeed);
  cfg.conntimeout = DEFAULT_CONNTIMEOUT;
}
