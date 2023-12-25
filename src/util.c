/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * util.h - utility functions
 *
 * Copyright (c) 2002-2023, Victor Antonovich (v.antonovich@gmail.com)
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
 */

#include "util.h"

#include <ctype.h>
#include <string.h>

/**
 * Removes leading whitespace from the string. 
 * Parameters: s - the string to be trimmed.
 * Return: a pointer to the first non-whitespace character in string.
 */
char *
util_ltrim(const char *s)
{
  while (*s && isspace((unsigned char )(*s)))
    s++;
  return (char *) s;
}

/**
 * Removes trailing whitespace from the string. 
 * The first trailing whitespace is replaced with a NUL-terminator
 * in the given string.
 * Parameters: s - the string to be trimmed.
 * Return: a pointer to the string.
 */
char *
util_rtrim(char *s)
{
  char *p = s + strlen(s);
  while (p > s && isspace((unsigned char )(*--p)))
    *p = '\0';
  return s;
}

/**
 * Removes leading and trailing whitespace from the string. 
 * The first trailing whitespace is replaced with a NUL-terminator
 * in the given string.
 * Parameters: s - the string to be trimmed.
 * Return: a pointer to the first non-whitespace character in string.
 */
char *
util_trim(char *s)
{
	if (!strlen(s))
		return s;

	return util_ltrim(util_rtrim(s));
}