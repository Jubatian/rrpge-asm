/**
**  \file
**  \brief     Support routines for Pass 1
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/


#include "ps1sup.h"
#include "fault.h"
#include "litpr.h"
#include "strpr.h"
#include "pass2.h"
#include "valwr.h"



/* Internal to check and set occupation on a 32bit unit of the occ. bitmap */
static auint ps1sup_csocc(compout_t* cd, auint sec, auint off, auint m)
{
 if       (sec == SECT_CODE){
  if ((cd->codu[off >> 5] & m) != 0U){ return 1U; }
  cd->codu[off >> 5] |= m;
 }else if (sec == SECT_CONS){
  if ((cd->conu[off >> 5] & m) != 0U){ return 1U; }
  cd->conu[off >> 5] |= m;
 }else{
  if ((cd->datu[off >> 5] & m) != 0U){ return 1U; }
  cd->datu[off >> 5] |= m;
 }
 return 0U;
}



/* Pushes occpuation data and checks for overlap. The previous offset is
** supplied, the current is grabbed from the compile state. Returns nonzero
** (TRUE) if an overlap is detected while reporting an appropriate fault. */
auint ps1sup_setocc(auint pof, compst_t* hnd, compout_t* cd)
{
 uint8 s[80];
 uint8 sna[5];
 auint off = compst_getoffw(hnd);
 auint sec = compst_getsect(hnd);
 auint len = off - pof;
 auint m;
 auint ofm;

 /* Note: pof might be larger than off, be prepared for this case as well */

 if (sec == SECT_CODE){
  ofm  = 0xFFFFU;
 }else{
  ofm  = 0x0FFFU;
 }
 off &= ofm;
 pof &= ofm;
 len &= ofm;

 if ((off & 0xFFE0U) == (pof & 0xFFE0U)){ /* Within the same 32bit word in the occ. mask */
  m  =   0xFFFFFFFFU << (pof & 0x1FU);
  m &= ~(0xFFFFFFFFU << (off & 0x1FU));
  if (ps1sup_csocc(cd, sec, pof, m)){ goto fault_overlap; }
  return 0U;
 }

 m =   0xFFFFFFFFU << (pof & 0x1FU);
 if (ps1sup_csocc(cd, sec, pof, m)){ goto fault_overlap; }
 m = ~(0xFFFFFFFFU << (off & 0x1FU));
 if (ps1sup_csocc(cd, sec, off, m)){ goto fault_overlap; }

 pof >>= 5;
 off >>= 5;
 off = (off - 1U) & ofm;
 m = 0xFFFFFFFFU;
 while (pof != off){
  if (ps1sup_csocc(cd, sec, off << 5, m)){ pof <<= 5; goto fault_overlap; }
  off = (off - 1U) & ofm;
 }
 return 0U;

fault_overlap:

 if       (sec == SECT_DATA){
  snprintf((char*)(&sna[0]), 5U, "data");
  pof |= 0x3000U; /* It is in page 3 */
 }else if (sec == SECT_CODE){
  snprintf((char*)(&sna[0]), 5U, "code");
 }else{
  snprintf((char*)(&sna[0]), 5U, "cons");
 }

 snprintf((char*)(&s[0]), 80U, "Chunk at %4X of length %d in section \'%s\' overlaps", pof, len, (char*)(&sna[0]));
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;
}



/* Attempts to process the source line as one of the followings:
** 'ds', 'db' or 'dw'.
** 'org'.
** 'section'.
** In case of 'db' or 'dw', it outputs to the appropriate memory (code ROM or
** app. header). Provides the following returns:
** 0: Fault, compilation should stop, fault printed.
** 1: Succesfully parsed something.
** 2: No content usable, but other parsers may try.
** In the case of data allocations, also checks and reports overlaps. Note
** that it starts parsing the line at the last set char. position, so this way
** labels may be skipped (processed earlier using litpr_symdefproc()). */
auint ps1sup_parsmisc(compst_t* hnd, compout_t* cd)
{
 uint8        s[80];
 uint8 const* src = compst_getsstrcoff(hnd); /* Might not be first char (Labels!) */
 auint        pof;
 auint        off = compst_getoffw(hnd);
 auint        beg = strpr_nextnw(src, 0U);
 auint        sec = compst_getsect(hnd);
 uint16*      d16;
 auint        u;
 auint        t;
 uint32       v;
 uint8        ste[LINE_MAX];

 if (strpr_isend(src[beg])){
  return 1U;            /* No content on this line, so processed succesful */
 }

 if (sec == SECT_CODE){ /* For db and dw, target area */
  d16 = &(cd->code[0]);
 }else{
  d16 = &(cd->cons[0]);
 }

 /* Check for keywords */


 if       (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("section"))){

  /* Set section */

  beg = strpr_nextnw(src, beg + 7U);

  if       (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("code"))){
   compst_setsect(hnd, SECT_CODE);
  }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("cons"))){
   compst_setsect(hnd, SECT_CONS);
  }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("data"))){
   compst_setsect(hnd, SECT_DATA);
  }else{
   goto fault_ins;
  }

  beg = strpr_nextnw(src, beg + 4U); /* End of string? */
  if (!strpr_isend(src[beg])){ goto fault_ins; }
  return 1U;                         /* All OK, section encoded */


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("org"))){

  /* Set origin. Note: no undefined symbols allowed here! */

  beg = strpr_nextnw(src, beg + 3U);
  t = litpr_getval(&(src[beg]), &u, &v, hnd);
  if ( (t != LITPR_VAL) || (v > 0xFFFFU) ){
   goto fault_ino;
  }
  compst_setoffw(hnd, v);

  beg = beg + u;                     /* End of string? */
  if (!strpr_isend(src[beg])){ goto fault_ins; }
  return 1U;                         /* All OK, origin encoded */


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("ds"))){

  /* Reserve data words. Only in data section */

  if (sec != SECT_DATA){ goto fault_dss; }

  beg = strpr_nextnw(src, beg + 2U);
  t = litpr_getval(&(src[beg]), &u, &v, hnd);
  if ( (t != LITPR_VAL) || (v > 0x0FFFU) ){
   goto fault_ind;
  }
  compst_setoffw(hnd, off + v);

  beg = beg + u;                     /* End of string? */
  if (!strpr_isend(src[beg])){ goto fault_ind; }
  return (auint)(!ps1sup_setocc(off, hnd, cd));


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("db"))){

  /* Insert literal bytes. Only in cons and code sections */

  if (sec == SECT_DATA){ goto fault_dxs; }

  beg = strpr_nextnw(src, beg + 2U);
  pof = off; /* For occupation test */
  off <<= 1; /* Byte offset */
  while(1){
   t = litpr_getval(&(src[beg]), &u, &v, hnd);

   if ((t & LITPR_STR) != 0U){       /* String (any) */
    if (strpr_extstr(&(ste[0]), &(src[beg]), LINE_MAX)==0){ goto fault_inx; }
    t = 0U;
    while (1){
     if (ste[t] == 0U){ break; }
     if ((off & 1U) == 0U){
      d16[off >> 1] = (d16[off >> 1] & 0x00FFU) | ((auint)(ste[t]) << 8);
     }else{
      d16[off >> 1] = (d16[off >> 1] & 0xFF00U) | ((auint)(ste[t]));
     }
     off = compst_incoffb(hnd, 1U);
     t++;
    }

   }else if ((t & LITPR_VAL) != 0U){ /* Value */
    if ((off & 1U) == 0U){
     d16[off >> 1] = (d16[off >> 1] & 0x00FFU) | ((v & 0xFFU) << 8);
    }else{
     d16[off >> 1] = (d16[off >> 1] & 0xFF00U) | ((v & 0xFFU));
    }
    off = compst_incoffb(hnd, 1U);

   }else if (t == LITPR_UND){        /* Undefined symbol - pass2 should do it */
    if (sec == SECT_CONS){
     t = (off >> 1) | VALWR_APPH;
    }else{
     t = (off >> 1);
    }
    if ((off & 1U) == 0U){
     pass2_addsymuse(&(src[beg]), t, VALWR_C8H, hnd);
    }else{
     pass2_addsymuse(&(src[beg]), t, VALWR_C8L, hnd);
    }
    off = compst_incoffb(hnd, 1U);

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
  return (auint)(!ps1sup_setocc(pof, hnd, cd));


 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("dw"))){

  /* Insert literal words. Only in cons and code sections */

  if (sec == SECT_DATA){ goto fault_dxs; }

  beg = strpr_nextnw(src, beg + 2U);
  pof = off; /* For occupation test */
  while(1){
   t = litpr_getval(&(src[beg]), &u, &v, hnd);

   if ((t & LITPR_VAL) != 0U){       /* Value */
    d16[off] = v & 0xFFFFU;
    off = compst_incoffw(hnd, 1U);

   }else if (t == LITPR_UND){        /* Undefined symbol - pass2 should do it */
    if (sec == SECT_CONS){
     t = off | VALWR_APPH;
    }else{
     t = off;
    }
    pass2_addsymuse(&(src[beg]), t, VALWR_C16, hnd);
    off = compst_incoffw(hnd, 1U);

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
  return (auint)(!ps1sup_setocc(pof, hnd, cd));


 }else{

  /* Some other type of content, maybe opcode. Not processed */
  return 2U;
 }


fault_ins:

 compst_setcoffrel(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed section specification");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_ino:

 compst_setcoffrel(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed origin");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_dss:

 compst_setcoffrel(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "\'ds\' is only allowed in data section");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_ind:

 compst_setcoffrel(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed \'ds\'");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_dxs:

 compst_setcoffrel(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "\'db\' or \'dw\' is only allowed in code or cons section");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

fault_inx:

 compst_setcoffrel(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed \'db\' or \'dw\'");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 0U;

}
