/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * modbus.c - MODBUS protocol related procedures
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
 * $Id: modbus.c,v 1.1 2003/09/13 20:38:39 kapyar Exp $
 */

#include "modbus.h"

/*
 * Check CRC of MODBUS frame
 * Parameters: FRAME - address of the frame,
 *             LEN   - frame length;
 * Return: 0 if CRC failed,
 *         non-zero otherwise
 */
int
modbus_crc_correct(unsigned char *frame, unsigned int len)
{
  return (!crc16(frame, len));
}

/*
 * Write CRC to MODBUS frame
 * Parameters: FRAME - address of the frame,
 *             LEN   - frame length;
 * Return: none
 */
void
modbus_crc_write(unsigned char *frame, unsigned int len)
{
  WORD_WR_LE(frame + len, crc16(frame, len));
}

/*
 * Write exception response to OpenMODBUS request
 * Parameters: PACKET - address of the request packet,
 *             CODE - exception code;
 * Return: none
 */
void
modbus_ex_write(unsigned char *packet, unsigned char code)
{
  MB_HDR(packet, MB_FCODE) |= 0x80;
  MB_HDR(packet, MB_DATA) = code;
  WORD_WR_BE(packet + MB_LENGTH_H, MB_EX_LEN);
}

/*
 * Check MODBUS packet header consistency
 * Parameters: HEADER - address of the header
 * Return: RC_OK if (mostly) all is right, RC_ERR otherwise
 */
int
modbus_check_header(unsigned char *packet)
{
  return (MB_HDR(packet, MB_PROTO_ID_H) == 0 &&
          MB_HDR(packet, MB_PROTO_ID_L) == 0 &&   
          MB_HDR(packet, MB_LENGTH_H) == 0   &&
          MB_HDR(packet, MB_LENGTH_L) > 0) ? RC_OK : RC_ERR;
}
