/*
 * OpenMODBUS/TCP to RS-232/485 MODBUS RTU gateway
 *
 * modbus.h - MODBUS protocol related procedures
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
 * $Id: modbus.h,v 1.3 2015/02/25 10:33:57 kapyar Exp $
 */

#ifndef _MODBUS_H
#define _MODBUS_H

#include "globals.h"
#include "crc16.h"

/*
 * Macro for accessing data in MODBUS frame
 */
#define MB_FRAME(p, d) ( *(p + d) )

/*
 * MODBUS frame lengths
 */
#define MB_EX_LEN  3
#define	MB_MIN_LEN 4
#define MB_ERR_LEN 5
#define	MB_MAX_LEN 256

/*
 * Macros for word operations
 */
#define HIGH(w) ( (unsigned char)(((w) >> 8) & 0xff) )
#define LOW(w) ( (unsigned char)((w) & 0xff) )
#define WORD_WR_BE(p, w) do \
                         { *(p) = (unsigned char)(((w) >> 8) & 0xff); \
                           *(p + 1) = (unsigned char)((w) & 0xff); } \
                         while (0);
#define WORD_RD_BE(p) ( (((unsigned short)*(p) << 8) & 0xff00) + \
                        (*(p + 1) & 0xff) )
#define WORD_WR_LE(p, w) do \
                         {  *(p) = (unsigned char)((w) & 0xff); \
                            *(p + 1) = (unsigned char)(((w) >> 8) & 0xff); } \
                         while (0);
#define WORD_RD_LE(p) ( (((unsigned short)*(p + 1) << 8) & 0xff00) + \
                        (*(p) & 0xff) )

/*
 * MODBUS packet structure
 */
#define MB_TRANS_ID_H 0    /* transaction ID high byte */
#define MB_TRANS_ID_L 1    /* transaction ID low byte */
#define MB_PROTO_ID_H 2    /* protocol ID high byte */
#define MB_PROTO_ID_L 3    /* protocol ID low byte */
#define MB_LENGTH_H   4    /* length field high byte */
#define MB_LENGTH_L   5    /* length field low byte */
#define MB_UNIT_ID    6    /* unit identifier */
#define MB_FCODE      7    /* function code */
#define MB_DATA       8    /* MODBUS data */
#define MB_DATA_ADDR_H MB_DATA    /* MODBUS data: address high byte */
#define MB_DATA_ADDR_L       9    /* MODBUS data: address low byte */
#define MB_DATA_NVAL_H      10    /* MODBUS data: number of values high byte */
#define MB_DATA_NVAL_L      11    /* MODBUS data: number of values low byte */
#define MB_DATA_NBYTES      12    /* MODBUS data: number of bytes to follow (fc 15,16) */

#define TTY_FCODE_IDX 1    /* The index of the function code in a RTU packet */
#define TTY_ERR_MASK  0x80 /* The MSB of the response function code indicates an error response */

/*
 * Exception codes
 */
#define MB_EX_CRC     4
#define MB_EX_TIMEOUT 0x0B

/* Prototypes */
int modbus_crc_correct(unsigned char *frame, unsigned int len);
void modbus_crc_write(unsigned char *frame, unsigned int len);
void modbus_ex_write(unsigned char *packet, unsigned char code);
int modbus_check_header(unsigned char *packet);


#endif /* _MODBUS_H */
