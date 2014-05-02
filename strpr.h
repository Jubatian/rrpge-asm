/**
**  \file
**  \brief     String processing assistance functions
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
*/


#ifndef STRPR_H
#define STRPR_H


#include "types.h"


/* Copies zero terminated string to target with limit; target string is
** ensured to be terminated (It will have limit-1 effective characters).
** Returns number of characters (without terminator) in destination. 'len' is
** the limit, it must be at least 1. */
auint strpr_copy(uint8* dst, uint8 const* src, auint len);


/* Checks if a character is a valid character for a symbol or literal. These
** may be characters of [0-9][a-z][A-Z][_][.]. Returns nonzero (TRUE) if it is
** so. */
auint strpr_issym(uint8 c);


/* Checks if a character is a whitespace. These are the horizontal tab (0x9)
** and the space (0x20) characters. Returns nonzero (TRUE) if so. */
auint strpr_isspc(uint8 c);


/* Checks if a character is a line terminator. Apart from normal terminators
** (newline or zero) the comment markers (; and #) are also terminators.
** Returns nonzero (TRUE) if so. */
auint strpr_isend(uint8 c);


/* Finds next non-whitespace character in the passed string. Returns it's
** position. 'beg' is the position to begin the search at. */
auint strpr_nextnw(uint8 const* src, auint beg);


/* Checks for string literal from the current position, and attempts to
** extract it. Returns the end position of it (which is always larger than
** zero) if any valid string was extracted (this includes empty string as ""
** or ''). Translates any escape sequences within. Returns zero otherwise and
** leaves the target undefined. If the source string literal is longer than
** the target it is truncated, a terminating zero is always placed on the end
** of the target. */
auint strpr_extstr(uint8* dst, uint8 const* src, auint len);


#endif
