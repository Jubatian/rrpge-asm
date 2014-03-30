/**
**  \file
**  \brief     Literal processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/



#include "litpr.h"
#include "pass2.h"
#include "strpr.h"



/* Check whether LITPR_INV is zero. Don't mess with that! */
#if (LITPR_INV != 0U)
#error "LITPR_INV must be 0"
#endif



/* Tries to interpret and retrieve value of a literal in the given string. The
** following outcomes are possible:
** Valid literal found, value returned (LITPR_VAL).
** Not yet defined symbol found (LITPR_UND).
** String literal found (LITPR_STR).
** String is not interpretable as literal (LITPR_INV).
** LITPR_VAL may pair with LITPR_STR. If LITPR_STR is returned alone, it
** indicates long (>4 chars) string. The interpretation stops when the line
** terminates or a seperator (,), or a function ({ or }) or an addrssing mode
** bracket (]) is found. Returns the offset of stopping (pointing at the ',',
** ']', '{' or '}' if any) in 'len' (zero for LITPR_INV), and the value of the
** literal in 'val' (only for LITPR_VAL). If LITPR_UND is returned, the symbol
** should be appropriately submitted with pass2_addsymuse(). */
auint litpr_getval(uint8 const* src, auint* len, uint32* val, compst_t* hnd)
{
 auint e;               /* Position in string */
 auint u;               /* For generating the result in val */
 auint r = 0U;          /* Return value */
 uint8 s[LINE_MAX];
 auint t;

 /* Check for string */

 e = strpr_extstr(&(s[0]), src, LINE_MAX);
 if (e != 0U){ /* String found */
  r |= LITPR_STR;
  u = 0U;
  for (t = 0; t < 5U; t++){
   if (s[t] == 0) break;
   u = (u << 8) + s[t]; /* Big Endian order */
  }
  if (t < 5){ goto end_val; }
  else      { goto end_sok; }
 }

 /* Check for decimal literal */

 e = 0U;
 u = 0U;
 while ( (src[e] >= (uint8)('0')) && (src[e] <= (uint8)('9')) ){
  if (u > (0xFFFFFFFFU / 10U)){
   e = 0U;
   break;
  }
  u = (u * 10U) + (src[e] - (uint8)('0'));
  e++;
 }
 if ( (e != 0U) && (!strpr_issym(src[e])) ){
  goto end_val;
 }

 /* Check for hexadecimal literal */

 if (src[0] == '0'){
  if (src[1] == 'x'){
   e = 2U;
   u = 0U;
   while (1){
    if      ((src[e] >= (uint8)('0')) && (src[e] <= (uint8)('9'))){ t = src[e] - (uint8)('0'); }
    else if ((src[e] >= (uint8)('a')) && (src[e] <= (uint8)('f'))){ t = src[e] - (uint8)('a') + 10U; }
    else if ((src[e] >= (uint8)('A')) && (src[e] <= (uint8)('F'))){ t = src[e] - (uint8)('A') + 10U; }
    else                                                          { break; }
    if (u > (0xFFFFFFFFU >> 4)){
     e = 0U;
     break;
    }
    u = (u << 4) + t;
    e++;
   }
   if ( (e != 0U) && (!strpr_issym(src[e])) ){
    goto end_val;
   }
  }
 }

 /* Check for binary literal */

 if (src[0] == '0'){
  if (src[1] == 'b'){
   e = 2U;
   u = 0U;
   while ( (src[e] >= (uint8)('0')) && (src[e] <= (uint8)('1')) ){
    if (u > (0xFFFFFFFFU >> 1)){
     e = 0U;
     break;
    }
    u = (u << 1) + (src[e] - (uint8)('0'));
    e++;
   }
   if ( (e != 0U) && (!strpr_issym(src[e])) ){
    goto end_val;
   }
  }
 }

 /* Check for symbol. Assume so if there is at least one valid symbol char. */

 if (strpr_issym(src[0])){
  e = 0U;               /* Calculate symbol length */
  while (strpr_issym(src[e])){ e++; }
  if (pass2_getsymval(src, val, hnd)){
   r |= LITPR_VAL;
   goto end_sok;        /* Symbol was defined, OK */
  }
  r |= LITPR_UND;       /* Undefined symbol */
  goto end_sok;
 }

 /* No literal here */

 goto end_fault;

 /* Literal / symbol succesfully decoded, still need to check string validity
 ** (is there any invalid part after the literal?) */

end_val:

 r |= LITPR_VAL;
 *val = u;

end_sok:

 e = strpr_nextnw(src, e);
 *len = e;
 if ( (!strpr_isend(src[e])) &&
      (src[e] != (uint8)(',')) &&
      (src[e] != (uint8)('{')) &&
      (src[e] != (uint8)('}')) &&
      (src[e] != (uint8)(']')) ){ goto end_fault; }
 return r;

end_fault:

 *len = 0U;
 return LITPR_INV;

}



/* Checks for symbol definition on the beginning of the current line. If a
** valid symbol definition is found (either line label or equ) it is
** submitted to pass2 as new symbol value. The current offset is used for
** this when the symbol is a label. Returns nonzero if a symbol definition was
** found and added the following way: If it is a label, returns the source
** string position after the ':' (at least 2), if it is an equ, returns 1. */
auint litpr_symdefproc(compst_t* hnd)
{
 auint  i;
 auint  r;
 auint  t;
 uint32 v;
 uint8 const* s;

 s = compst_getsstr(hnd);

 i = 0U;
 while (strpr_issym(s[i])){ i++; }

 if (i == 0U){ return 0U; } /* No symbol here */

 /* Check for ':' or 'equ' */

 i = strpr_nextnw(&s[0], i);

 if (s[i] == ':'){   /* Line label: the symbol's value is the offset */
  compst_setgsym(hnd, &s[0]);      /* Add global symbol (if it is global) */
  pass2_addsymval(&s[0], compst_getoffw(hnd), hnd);
  return (i + 1U);   /* OK, found, added, done */
 }

 if (compst_issymequ(NULL, &(s[i]), (uint8 const*)("equ"))){
  i = strpr_nextnw(&s[0], i + 3U); /* Skip whites after equ */
  r = litpr_getval(&s[i], &t, &v, hnd);
  if ((r & LITPR_VAL) != 0U){      /* Valid literal, so can be stored */
   pass2_addsymval(&s[0], v, hnd);
   return 1U;
  }
 }

 /* No parseable label or equ found */

 return 0U;
}
