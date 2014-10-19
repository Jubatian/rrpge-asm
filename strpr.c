/**
**  \file
**  \brief     String processing assistance functions
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.19
*/


#include "strpr.h"



/* Copies zero terminated string to target with limit; target string is
** ensured to be terminated (It will have limit-1 effective characters).
** Returns number of characters (without terminator) in destination. 'len' is
** the limit, it must be at least 1. */
auint strpr_copy(uint8* dst, uint8 const* src, auint len)
{
 auint i = 0U;
 auint m = len - 1U;

 while (src[i] != 0U){
  if (i == m){ break; }
  dst[i] = src[i];
  i++;
 }
 dst[i] = 0U;

 return i;
}



/* Checks if a character is a valid character for a symbol or literal. These
** may be characters of [0-9][a-z][A-Z][_][.]. Returns nonzero (TRUE) if it is
** so. */
auint strpr_issym(uint8 c)
{
 if ( (c >= (uint8)('0')) && (c <= (uint8)('9')) ){ return 1U; }
 if ( (c >= (uint8)('A')) && (c <= (uint8)('Z')) ){ return 1U; }
 if ( (c >= (uint8)('a')) && (c <= (uint8)('z')) ){ return 1U; }
 if ( (c == (uint8)('_'))                        ){ return 1U; }
 if ( (c == (uint8)('.'))                        ){ return 1U; }
 return 0U;
}



/* Checks if a character is a whitespace. These are the horizontal tab (0x9)
** and the space (0x20) characters. Returns nonzero (TRUE) if so. */
auint strpr_isspc(uint8 c)
{
 if ( (c == (uint8)(' '))                        ){ return 1U; }
 if ( (c == (uint8)('\t'))                       ){ return 1U; }
 return 0U;
}



/* Checks if a character is a line terminator. Apart from normal terminators
** (newline or zero) the comment markers (; and #) are also terminators.
** Returns nonzero (TRUE) if so. */
auint strpr_isend(uint8 c)
{
 if ( (c == (uint8)(0U))                         ){ return 1U; }
 if ( (c == (uint8)('\n'))                       ){ return 1U; }
 if ( (c == (uint8)('\r'))                       ){ return 1U; }
 if ( (c == (uint8)(';'))                        ){ return 1U; }
 if ( (c == (uint8)('#'))                        ){ return 1U; }
 return 0U;
}



/* Finds next non-whitespace character in the passed string. Returns it's
** position. 'beg' is the position to begin the search at. */
auint strpr_nextnw(uint8 const* src, auint beg)
{
 while (strpr_isspc(src[beg])){ beg++; }
 return beg;
}



/* Checks for string literal from the current position, and attempts to
** extract it. Returns the end position of it (which is always larger than
** zero) if any valid string was extracted (this includes empty string as ""
** or ''). Translates any escape sequences within. Returns zero otherwise and
** leaves the target undefined. If the source string literal is longer than
** the target it is truncated, a terminating zero is always placed on the end
** of the target. */
auint strpr_extstr(uint8* dst, uint8 const* src, auint len)
{
 uint8 b;
 uint8 c;
 auint i;
 auint j;
 auint e;

 /* Check beginning, a bordering character has to be found */
 if ( (src[0] == '\'') || (src[0] == '\"') ){
  b = src[0];
 }else{
  return 0U; /* Not a string literal */
 }

 /* Collect characters */
 i = 1U;     /* Source pointer */
 j = 0U;     /* Target pointer */
 e = 0U;     /* An escape has to be performed? */
 while(1U){

  /* Check for premature string termination */
  if ( (src[i] < 0x20U) && (src[i] != '\t') ){ /* Only tab is allowed from controls */
   return 0U;  /* No valid string */
  }

  c = 0U;    /* New char. to be put into target */

  /* If escaped, and the escape is valid, process, otherwise just ignore the
  ** escape. */
  if (e){
   if      (src[i] == 'n' ){ c = '\n'; }
   else if (src[i] == 't' ){ c = '\t'; }
   else if (src[i] == 'r' ){ c = '\r'; }
   else                    { c = src[i]; } /* Also does ', " and \ escapes */
   e = 0U;   /* Escape processed */

  /* Otherwise ordinary character */
  }else{

   if (src[i] == b){ /* Terminator of string literal */
    dst[j] = 0U;
    return i + 1U;   /* Position after the end of the literal */

   }else if (src[i] == '\\'){ /* Escape character */
    e = 1U;

   }else{            /* Normal characters */
    c = src[i];
   }
  }

  /* If a character was acquired, store it */
  if (c != 0U){
   if (j + 1U < len){
    dst[j] = c;
    j ++;
   }
  }

  i++;

 };
}
