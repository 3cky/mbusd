/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * sig.c - signals management procedures
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
 * $Id: sig.c,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */
 
#include "sig.h"
#include "tty.h"

/* signal flag */
volatile sig_atomic_t sig_flag = 0;

/* internal prototypes */
void sig_bus(int signum);
void sig_segv(int signum);
void sig_handler(int signum);

/*
 * Signal handler initialization
 * Return: none
 */
void
sig_init(void)
{
  struct sigaction sa;
  
  sa.sa_flags = 0;

  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask, SIGSEGV);
  sigaddset(&sa.sa_mask, SIGBUS);
  sigaddset(&sa.sa_mask, SIGTERM);
  sigaddset(&sa.sa_mask, SIGHUP);
  sigaddset(&sa.sa_mask, SIGINT);
  sigaddset(&sa.sa_mask, SIGPIPE);
  sigaddset(&sa.sa_mask, SIGCHLD);
  sigaddset(&sa.sa_mask, SIGALRM);
  sigaddset(&sa.sa_mask, SIGUSR1);
  sigaddset(&sa.sa_mask, SIGUSR2);

  sa.sa_handler = sig_segv;
  sigaction(SIGSEGV, &sa, NULL);

  sa.sa_handler = sig_bus;
  sigaction(SIGBUS, &sa, NULL);

  sa.sa_handler = sig_handler;
  sigaction(SIGTERM, &sa, NULL);

  sa.sa_handler = sig_handler;
  sigaction(SIGHUP, &sa, NULL);

  sa.sa_handler = sig_handler;
  sigaction(SIGINT, &sa, NULL);

  sa.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &sa, NULL);

  sa.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &sa, NULL);

  sa.sa_handler = SIG_IGN;
  sigaction(SIGALRM, &sa, NULL);

  sa.sa_handler = SIG_IGN;
  sigaction(SIGUSR1, &sa, NULL);

  sa.sa_handler = SIG_IGN;
  sigaction(SIGUSR2, &sa, NULL);
}

/*
 * SIGSEGV signal handler
 */
void
sig_segv(int signum)
{
#ifndef SAFESIG
  signum = signum; /* prevent compiler warning */
  fprintf(stderr, "caught SIGSEGV, dumping core...");
  fclose(stderr);
#endif
  abort();
}

/*
 * SIGBUS signal handler
 */
void
sig_bus(int signum)
{
#ifndef SAFESIG
  signum = signum; /* prevent compiler warning */
  fprintf(stderr, "caught SIGBUS, dumping core...");
  fclose(stderr);
#endif
  abort();
}

/*
 * Unignored signals handler
 */
void
sig_handler(int signum)
{
#ifndef SAFESIG
  tty_sighup();
#endif
  sig_flag = signum;
}

/*
 * Signal action execution
 */
void
sig_exec(void)
{
#ifdef LOG
  static char *signames[] = { 
    "", "HUP", "INT", "QUIT", "ILL", "TRAP", "IOT", "BUS", "FPE",
    "KILL", "USR1", "SEGV", "USR2", "PIPE", "ALRM", "TERM" };
  logw(2, "Terminated by signal: SIG%s", signames[sig_flag]);
#endif
  /* currently simply exit the program */
  switch(sig_flag)
  {
  case SIGINT:
  case SIGTERM: // fall-through
    exit(0);
  default:
    exit(1);
  }
}
