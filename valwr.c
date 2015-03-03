/**
**  \file
**  \brief     Value write-out logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.03.03
*/


#include "valwr.h"



/* Attempts to write out value at a given offset in the current section. In
** 'use' supply the form by which to write out. Returns nonzero (TRUE) if some
** severe failure arises where compilation shouldn't continue. Before writing
** the value, the area should be occupied by normal section data pushes. */
auint valwr_write(section_t* dst, uint32 val, auint off, auint use, fault_off_t const* fof)
{
 uint8 s[80];
 auint t;

 switch (use){

  case VALWR_C16:
   section_setw(dst, off, val & 0xFFFFU);
   break;

  case VALWR_C8H:
   section_setw(dst, off, val & 0xFF00U);
   break;

  case VALWR_C8L:
   section_setw(dst, off, val & 0x00FFU);
   break;

  case VALWR_A4:
   if (val > 0xFU){
    snprintf((char*)(&s[0]), 80U, "Value is too large, truncated");
    fault_print(FAULT_NOTE, &s[0], fof);
   }
   section_setw(dst, off, val & 0x000FU);
   break;

  case VALWR_A16:
   section_setw(dst, off, (val >> 14) & 0x3U);
   section_setw(dst, off + 1U, val & 0x3FFFU);
   break;

  case VALWR_B4:
   if (val > 0xFU){
    snprintf((char*)(&s[0]), 80U, "Value is too large, truncated");
    fault_print(FAULT_NOTE, &s[0], fof);
   }
   section_setw(dst, off, (val & 0xFU) << 6);
   break;

  case VALWR_S6:
   if (val > 0x3FU){
    snprintf((char*)(&s[0]), 80U, "Operand value is too large for JSV");
    fault_print(FAULT_FAIL, &s[0], fof);
    return 1U;
   }
   section_setw(dst, off, val & 0x3FU);
   break;

  case VALWR_R16:
   /* This is for relative jumps, 16 bit offset (no range check necessary). */
   t = (val - off) & 0xFFFFU;
   section_setw(dst, off, (t >> 14) & 0x3U);
   section_setw(dst, off + 1U, t & 0x3FFFU);
   break;

  case VALWR_R10:
   /* This is for relative jumps. It has to be determined whether the jump
   ** from 'o' to 'val' is within the relative jump's range (-512 - 511). */
   t = (val - off) & 0xFFFFU;
   if ( (val > 0xFFFFU) ||
        ( (t > 0x01FFU) && (t < 0xFE00U) ) ){
    snprintf((char*)(&s[0]), 80U, "Relative jump target is out of range");
    fault_print(FAULT_FAIL, &s[0], fof);
    return 1U;
   }
   section_setw(dst, off, t & 0x03FFU);
   break;

  case VALWR_R7:
   /* This is for nonzero jumps. It has to be determined whether the jump from
   ** 'o' to 'val' is within the relative jump's range (-64 - 63). */
   t = (val - off) & 0xFFFFU;
   if ( (val > 0xFFFFU) ||
        ( (t > 0x003FU) && (t < 0xFFC0U) ) ){
    snprintf((char*)(&s[0]), 80U, "Nonzero jump target is out of range");
    fault_print(FAULT_FAIL, &s[0], fof);
    return 1U;
   }
   section_setw(dst, off, (t & 0x003FU) | ((t & 0x0040U) << 3));
   break;

  default:
   snprintf((char*)(&s[0]), 80U, "Illegal use of value");
   fault_print(FAULT_FAIL, &s[0], fof);
   return 1U;

 }

 return 0U;
}



/* Like valwr_write(), but uses the current compile state for outputting
** fault messages. */
auint valwr_writecs(section_t* dst, uint32 val, auint off, auint use, compst_t* cof)
{
 uint8       fil[FILE_MAX];
 fault_off_t fof;

 fault_fofget(&fof, cof, &fil[0]);
 return valwr_write(dst, val, off, use, &fof);
}
