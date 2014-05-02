/**
**  \file
**  \brief     Basic type defs & includes
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
*/


#ifndef TYPES_H
#define TYPES_H


#include <stdio.h>              /* printf */
#include <stdlib.h>             /* NULL, malloc, free, etc. */
#include <string.h>             /* Basic block memory routines */
#include <stdint.h>             /* Fixed width integer types */
#include <errno.h>              /* For errors */


typedef   signed  int   asint;  /* Architecture signed integer (At least 2^31) */
typedef unsigned  int   auint;  /* Architecture unsigned integer (At least 2^31) */
typedef  int16_t        sint16;
typedef uint16_t        uint16;
typedef  int32_t        sint32;
typedef uint32_t        uint32;
typedef   int8_t        sint8;
typedef  uint8_t        uint8;


#ifdef TARGET_WINDOWS_MINGW
/* Ugly hack for Windows: no strerror_r, so substutute it */
#define strerror_r(num, buf, len) strncpy(buf, strerror(num), len)
#endif


#endif
