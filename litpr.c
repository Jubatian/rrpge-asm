/**
**  \file
**  \brief     Literal processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.10.16
*/



#include "litpr.h"
#include "pass2.h"
#include "strpr.h"



/* Check whether LITPR_INV is zero. Don't mess with that! */
#if (LITPR_INV != 0U)
#error "LITPR_INV must be 0"
#endif



/* Section base symbols. Temporarily here, will need to be re-organized later
** into some appropriate module. */
uint8 const* litpr_secbs[6] = {
 (uint8 const*)("$.code"),
 (uint8 const*)("$.data"),
 (uint8 const*)("$.head"),
 (uint8 const*)("$.desc"),
 (uint8 const*)("$.zero"),
 (uint8 const*)("$.file") };



/* Tries to interpret and retrieve value of a literal in the given string. The
** following outcomes are possible:
**
** LITPR_VAL: Valid literal found, it's value is returned in 'val'.
** LITPR_UND: An aggregate containing symbols found. The aggregate's members
**            are submitted appropriately to the symbol table, in 'val' the
**            top member's ID is returned (to be used with symtab_use()).
** LITPR_STR: String literal is found. If LITPR_VAL is set along with it, the
**            value of the literal is returned in 'val', otherwise not (the
**            string is longer than 4 ASCII characters).
** LITPR_INV: Error. An appropriate fault code is printed.
**
** The interpretation stops when the line terminates or a seperator (,), or a
** function ({ or }) or an addrssing mode bracket (]) is found. Returns the
** offset of stopping (pointing at the ',', ']', '{' or '}' if any) in 'len'
** (zero for LITPR_INV). */
auint litpr_getval(uint8 const* src, auint* len, auint* val, compst_t* hnd, symtab_t* stb)
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
  val = symtab_getsymdef(stb, src);
  if (val == 0U){ goto end_fault; }
  r |= LITPR_UND;       /* Symbol aggregate */
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
** submitted to the symbol table as new symbol value. The following outcomes
** are possible:
**
** LITPR_EQU: An 'equ'. The line is processed appropriately, creating symbol
**            table entries if necessary.
** LITPR_LBL: A label. In 'len' the source string position after the ':' is
**            returned. An symbol aggregate appropriate to the section type
**            ('$.code', '$.data', '$.head', '$.desc', '$.zero' or '$.file')
**            is created in the symbol table.
** LITPR_INV: An error was encountered during processing, fault code was
**            printed.
**
** In the case of LITPR_LBL, the line should be processed further from the
** position returned in 'len'. If there is no processable content on the line,
** LITPR_LBL is returned with 'len' set to zero, so other processors may pick
** up working with the line. */
auint litpr_symdefproc(auint* len, compst_t* hnd, section_t* sec, symtab_t* stb)
{
 auint  i;
 auint  r;
 auint  t;
 uint32 v;
 uint8 const* s;

 *len = 0U; /* No processable content case */
 s = compst_getsstr(hnd);

 i = 0U;
 while (strpr_issym(s[i])){ i++; }

 if (i == 0U){ return LITPR_LBL; } /* No symbol here */

 /* Check for ':' or 'equ' */

 i = strpr_nextnw(&s[0], i);

 if (s[i] == ':'){   /* Line label: the symbol's value is the offset */
  compst_setgsym(hnd, &s[0]);      /* Add global symbol (if it is global) */
  *len = i + 1U;
  i = symtab_addsymdef(stb, SYMTAB_CMD_ADD | SYMTAB_CMD_S1N,
                       section_getoffw(sec), NULL,
                       0U, litpr_secbs[section_getsect(sec)]);
  if (i == 0U){ return LITPR_INV; }
  i = symtab_bind(stb, &s[0], i);
  if (i == 0U){ return LITPR_INV; }
  return LITPR_LBL;  /* OK, found, added, done */
 }

 if (compst_issymequ(NULL, &(s[i]), (uint8 const*)("equ"))){
  i = strpr_nextnw(&s[0], i + 3U); /* Skip whites after equ */
  r = litpr_getval(&s[i], &t, &v, hnd, stb);
  if ((r & LITPR_VAL) != 0U){      /* Valid literal, so can be stored */
   v = symtab_addsymdef(stb, SYMTAB_CMD_MOV, v, NULL, 0U, NULL);
   if (v == 0U){ return LITPR_INV; }
  }
  if ( ((r & LITPR_VAL) != 0U)
       ((r & LITPR_UND) != 0U) ){  /* Valid literal or symbol aggregate */
   i = symtab_bind(stb, &s[0], v);
   if (i == 0U){ return LITPR_INV; }
   return LITPR_EQU;
  }
 }

 /* No parseable label or equ found: maybe something else will parse it */

 return LITPR_LBL;
}
