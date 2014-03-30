/**
**  \file
**  \brief     Value write-out logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/


#include "valwr.h"



/* Attempts to write out value at a given offset. On 'off' use VALWR_APPH for
** writing by application header logic. In 'use' supply the form by which to
** write out. When using the application header the offset is truncated to the
** low 12 bits, otherwise the low 16 bits. Returns nonzero (TRUE) if some
** severe failure arises where compilation shouldn't continue. */
auint valwr_write(uint16* dst, uint32 val, auint off, auint use, fault_off_t const* fof)
{
 uint8 s[80];
 auint o;
 auint t;

 /* Value write in the app. header */
 if ((off & VALWR_APPH) != 0){

  o = off & 0x0FFFU;
  switch (use){

   case VALWR_C16:
    dst[o] = val & 0xFFFFU;
    break;

   case VALWR_C8H:
    dst[o] = (dst[o] & 0x00FFU) | ((val & 0xFFU) << 8);
    break;

   case VALWR_C8L:
    dst[o] = (dst[o] & 0xFF00U) | ((val & 0xFFU)     );
    break;

   default:
    snprintf((char*)(&s[0]), 80U, "Illegal use of value in app. header");
    fault_print(FAULT_FAIL, &s[0], fof);
    return 1U;

  }

 /* Value write in the code memory */
 }else{

  o = off & 0xFFFFU;
  switch (use){

   case VALWR_C16:
    dst[o] = val & 0xFFFFU;
    break;

   case VALWR_C8H:
    dst[o] = (dst[o] & 0x00FFU) | ((val & 0xFFU) << 8);
    break;

   case VALWR_C8L:
    dst[o] = (dst[o] & 0xFF00U) | ((val & 0xFFU)     );
    break;

   case VALWR_A4:
    if (val > 0xFU){
     snprintf((char*)(&s[0]), 80U, "Value is too large, truncated");
     fault_print(FAULT_NOTE, &s[0], fof);
    }
    dst[o] = (dst[o] & 0xFFF0U) | (val & 0x0FU);
    break;

   case VALWR_A16:
    dst[o] = (dst[o] & 0xFFFCU) | ((val >> 14) & 0x3U);
    o = (o + 1U) & 0xFFFFU;
    dst[o] = (dst[o] & 0xC000U) | (val & 0x3FFFU);
    break;

   case VALWR_B4:
    if (val > 0xFU){
     snprintf((char*)(&s[0]), 80U, "Value is too large, truncated");
     fault_print(FAULT_NOTE, &s[0], fof);
    }
    dst[o] = (dst[o] & 0xFC3FU) | ((val & 0x0FU) << 6);
    break;

   case VALWR_J12:
    /* This is for page local jumps. Here the same page of the source and
    ** target has to be checked, making sure the jump actually ends up on the
    ** targeted address. 'o' is where it jumps from, 'val' is where it wants
    ** to end up. */
    if ( (val > 0xFFFFU) ||
         (((o ^ val) & 0xF000U) != 0U) ){
     snprintf((char*)(&s[0]), 80U, "Page local jump or call target is out of range");
     fault_print(FAULT_FAIL, &s[0], fof);
     return 1U;
    }
    o = (o + 1U) & 0xFFFFU;
    dst[o] = (dst[o] & 0xF000U) | (val & 0x0FFFU);
    break;

   case VALWR_J16:
    if ( (val > 0xFFFFU) ){
     snprintf((char*)(&s[0]), 80U, "Jump or call target is out of range");
     fault_print(FAULT_FAIL, &s[0], fof);
     return 1U;
    }
    dst[o] = (dst[o] & 0xFFFCU) | ((val >> 14) & 0x3U);
    o = (o + 1U) & 0xFFFFU;
    dst[o] = (dst[o] & 0xC000U) | (val & 0x3FFFU);
    break;

   case VALWR_R10:
    /* This is for relative jumps. It has to be determined whether the jump
    ** from 'o' to 'val' is within the relative jump's range (-512 - 511). */
    t = (val - o) & 0xFFFFU;
    if ( (val > 0xFFFFU) ||
         ( (t > 0x01FFU) && (t < 0xFE00U) ) ){
     snprintf((char*)(&s[0]), 80U, "Relative jump target is out of range");
     fault_print(FAULT_FAIL, &s[0], fof);
     return 1U;
    }
    dst[o] = (dst[o] & 0xFC00U) | (t & 0x03FFU);
    break;

   default:
    snprintf((char*)(&s[0]), 80U, "Illegal use of value in code memory");
    fault_print(FAULT_FAIL, &s[0], fof);
    return 1U;

  }

 }

 return 0U;
}



/* Like valwr_writeoff(), but uses the current compile state for offset &
** outputting fault messages. Works with word offsets (will pad to word
** boundary when use), but otherwise it does not alter the offset. */
auint valwr_writeat(uint16* dst, uint32 val, auint use, compst_t* cof)
{
 uint8       fil[FILE_MAX];
 fault_off_t fof;
 auint       off = compst_getoffw(cof);

 fault_fofget(&fof, cof, &fil[0]);
 return valwr_write(dst, val, off, use, &fof);
}



/* Like valwr_writeoff(), but uses the current compile state for outputting
** fault messages, but not the offset. This way an earlier offset from the
** compile state may be supplied if necessary. */
auint valwr_writeatoff(uint16* dst, uint32 val, auint off, auint use, compst_t* cof)
{
 uint8       fil[FILE_MAX];
 fault_off_t fof;

 fault_fofget(&fof, cof, &fil[0]);
 return valwr_write(dst, val, off, use, &fof);
}
