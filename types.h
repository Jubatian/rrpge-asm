/**
**  \file
**  \brief     Basic type defs & includes
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.20
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


/* Return codes for line parsers */
/* Error during parse, fault printed, compilation must stop. */
#define PARSER_ERR 0U
/* Parse completed OK, other parsers in the sequence may continue. */
#define PARSER_OK  1U
/* Parse completed OK, end of line reached, parsing for the line should stop. */
#define PARSER_END 2U


/* Ugly hack for Windows: no strerror_r, so substitute it */
#ifdef TARGET_WINDOWS_MINGW
#define strerror_r(num, buf, len) strncpy(buf, strerror(num), len)
#endif


#endif
