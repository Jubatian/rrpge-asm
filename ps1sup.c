/**
**  \file
**  \brief     Support routines for Pass 1
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.10.17
*/


#include "ps1sup.h"
#include "fault.h"
#include "litpr.h"
#include "strpr.h"
#include "valwr.h"



/* Attempts to process the source line as one of the followings:
** 'ds', 'db' or 'dw'.
** 'org'.
** 'section'.
** Provides the following returns:
** 0: Fault, compilation should stop, fault printed.
** 1: Succesfully parsed something.
** 2: No content usable, but other parsers may try.
** In the case of data allocations, also checks and reports overlaps. Note
** that it starts parsing the line at the last set char. position, so this way
** labels may be skipped (processed earlier using litpr_symdefproc()). */
auint ps1sup_parsmisc(symtab_t* stb)
{
 uint8        s[80];
 uint8 const* src = compst_getsstrcoff(hnd); /* Might not be first char (Labels!) */
 auint        sid = section_getsect(sec);
 auint        beg = strpr_nextnw(src, 0U);
 auint        u;
 auint        t;
 uint32       v;
 uint8        ste[LINE_MAX];
 compst_t*    cst = symtab_getcompst(stb);
 section_t*   sec = symtab_getsectob(stb);

 if (strpr_isend(src[beg])){
  return 1U;            /* No content on this line, so processed succesful */
 }


 /* Check for keywords */


 if       (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("section"))){

  /* Set section */

  beg = strpr_nextnw(src, beg + 7U);

  if       (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("code"))){
   section_setsect(sec, SECT_CODE);
  }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("data"))){
   section_setsect(sec, SECT_DATA);
  }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("head"))){
   section_setsect(sec, SECT_HEAD);
  }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("desc"))){
   section_setsect(sec, SECT_DESC);
  }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("zero"))){
   section_setsect(sec, SECT_ZERO);
  }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("file"))){
   section_setsect(sec, SECT_FILE);
  }else{
   goto fault_ins;
  }

  beg = strpr_nextnw(src, beg + 4U); /* End of string? */
  if (!strpr_isend(src[beg])){ goto fault_ins; }
  return 1U;                         /* All OK, section encoded */


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("org"))){

  /* Set origin. Note: no symbols allowed here! */

  beg = strpr_nextnw(src, beg + 3U);
  t = litpr_getval(&(src[beg]), &u, &v, stb);
  if ( (t != LITPR_VAL) || (v > 0xFFFFU) ){
   goto fault_ino;
  }
  section_setoffw(sec, v);

  beg = beg + u;                     /* End of string? */
  if (!strpr_isend(src[beg])){ goto fault_ins; }
  return 1U;                         /* All OK, origin encoded */


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("ds"))){

  /* Reserve data words. Only in ZERO section. Note: no symbols allowed as ds' parameter. */

  if (sid != SECT_ZERO){ goto fault_dsz; }

  beg = strpr_nextnw(src, beg + 2U);
  t = litpr_getval(&(src[beg]), &u, &v, stb);
  if ( (t != LITPR_VAL) || (v > 0xFFFFU) ){
   goto fault_ind;
  }
  while (v != 0U){
   if (section_pushw(sec, 0U) != 0U){ goto fault_ovr; }
   v --;
  }

  beg = beg + u;                     /* End of string? */
  if (!strpr_isend(src[beg])){ goto fault_ind; }
  return 1U;


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("db"))){

  /* Insert literal bytes. Only in code, data, head and desc sections */

  if ((sid == SECT_ZERO) || (sid == SECT_FILE)){ goto fault_dxs; }

  beg = strpr_nextnw(src, beg + 2U);
  while(1){
   t = litpr_getval(&(src[beg]), &u, &v, stb);

   if ((t & LITPR_STR) != 0U){       /* String (any) */
    if (strpr_extstr(&(ste[0]), &(src[beg]), LINE_MAX)==0){ goto fault_inx; }
    t = 0U;
    while (1){
     if (ste[t] == 0U){ break; }
     if (section_pushb(sec, ste[t]) != 0U){ goto fault_ovr; }
     t++;
    }

   }else if ((t & LITPR_VAL) != 0U){ /* Value */
    if (section_pushb(sec, v) != 0U){ goto fault_ovr; }

   }else if (t == LITPR_UND){        /* Undefined symbol - pass2 should do it */
    off = section_getoffb(sec);
    section_pushb(sec, 0);
    if ((off & 1U) == 0U){
     if (symtab_use(stb, v, off >> 1, VALWR_C8H)){ goto fault_oth; }
    }else{
     if (symtab_use(stb, v, off >> 1, VALWR_C8L)){ goto fault_oth; }
    }

   }else{                            /* Bad formatting */
    goto fault_inx;
   }

   beg += u;
   if (strpr_isend(src[beg])) break; /* End of string - done */
   if (src[beg] != (uint8)(',')){ goto fault_inx; }
   beg++;
   beg = strpr_nextnw(src, beg);
  }

  /* Note: No need to check string end here since it is done above */
  return 1U;


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("dw"))){

  /* Insert literal words. Only in code, data, head and desc sections */

  if ((sid == SECT_ZERO) || (sid == SECT_FILE)){ goto fault_dxs; }

  beg = strpr_nextnw(src, beg + 2U);
  while(1){
   t = litpr_getval(&(src[beg]), &u, &v, stb);

   if ((t & LITPR_VAL) != 0U){       /* Value */
    if (section_pushw(sec, v) != 0U){ goto fault_ovr; }

   }else if (t == LITPR_UND){        /* Undefined symbol - pass2 should do it */
    off = section_getoffw(sec);
    section_pushw(sec, 0);
    if (symtab_use(stb, v, off, VALWR_C16)){ goto fault_oth; }

   }else{                            /* Bad formatting */
    goto fault_inx;
   }

   beg += u;
   if (strpr_isend(src[beg])) break; /* End of string - done */
   if (src[beg] != (uint8)(',')){ goto fault_inx; }
   beg++;
   beg = strpr_nextnw(src, beg);
  }

  /* Note: No need to check string end here since it is done above */
  return 1U;


 }else{

  /* Some other type of content, maybe opcode. Not processed */
  return 2U;
 }


fault_ins:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed section specification");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_ino:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed origin");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_dsz:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "\'ds\' is only allowed in zero section");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_ind:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed \'ds\'");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_dxs:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "\'db\' or \'dw\' is only allowed in code, data, head or desc");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_inx:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed \'db\' or \'dw\'");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_ovr:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Overlap or out of section encountered");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_oth:

 return 0U;
}
