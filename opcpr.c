/**
**  \file
**  \brief     Opcode processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.29
*/


#include "opcpr.h"
#include "fault.h"
#include "litpr.h"
#include "valwr.h"
#include "strpr.h"



/* Decodes pointer register name: x0, x1, x2 or x3. Returns the new offset in
** string, the register encoding (0-3) in enc. Returns 0 if there was no valid
** register at the offset. Skips white spaces if needed before the reg. */
static auint opcpr_rp(uint8 const* src, auint beg, auint* enc)
{
 beg = strpr_nextnw(src, beg);
 if (src[beg] == (uint8)('x')){
  beg++;
  if ( (src[beg] >= ((uint8)('0'))) && (src[beg] <= ((uint8)('3'))) ){
   if (!strpr_issym(src[beg + 1U])){
    *enc = src[beg] - (uint8)('0');
    return (beg + 1U);
   }
  }
 }
 return 0U;
}



/* Decodes RX register name: a, b, c, d, x0, x1, x2 or x3. Returns the new
** offset in string, the register encoding (0-7) in enc. Returns 0 if there
** was no valid register at the offset. Skips white spaces if needed before
** the register. */
static auint opcpr_rx(uint8 const* src, auint beg, auint* enc)
{
 auint r = opcpr_rp(src, beg, enc);
 if (r != 0U){ /* Pointer register found */
  *enc = *enc + 4U;
  return r;
 }
 /* No pointer register, check normal registers */
 beg = strpr_nextnw(src, beg);
 if ( (src[beg] >= ((uint8)('a'))) && (src[beg] <= ((uint8)('d'))) ){
  if (!strpr_issym(src[beg + 1U])){
   *enc = src[beg] - (uint8)('a');
   return (beg + 1U);
  }
 }
 return 0U;
}



/* Checks for bp relative addressing looking for 'bp+' in the string. Returns
** the new offset in string, or 0 if not found. Skips whitespaces before and
** after. */
static auint opcpr_bp(uint8 const* src, auint beg)
{
 beg = strpr_nextnw(src, beg);

 if (src[beg] == (uint8)('b')){
  beg++;
  if (src[beg] == (uint8)('p')){
   beg = strpr_nextnw(src, beg + 1U);
   if (src[beg] == (uint8)('+')){
    beg = strpr_nextnw(src, beg + 1U);
    return beg;
   }
  }
 }

 return 0U;
}



/* Checks for 'sp' register name. Returns the new offset in string, or 0 if
** not found. Skips whitespaces before. */
static auint opcpr_sp(uint8 const* src, auint beg)
{
 beg = strpr_nextnw(src, beg);
 if (src[beg] == (uint8)('s')){
  beg++;
  if (src[beg] == (uint8)('p')){
   beg++;
   if (!strpr_issym(src[beg])){
    return beg;
   }
  }
 }
 return 0U;
}



/* Decodes pointer mode register name: xm or xh. Returns the new offset in
** string, the register encoding (0-1) in enc. Returns 0 if there was no valid
** register at the offset. Skips white spaces if needed before the reg. */
static auint opcpr_xm(uint8 const* src, auint beg, auint* enc)
{
 beg = strpr_nextnw(src, beg);
 if (src[beg] == (uint8)('x')){
  beg++;
  if ( (src[beg] == ((uint8)('m'))) || (src[beg] == ((uint8)('h'))) ){
   if (!strpr_issym(src[beg + 1U])){
    if (src[beg] == ((uint8)('m'))){ *enc = 0U; }
    else                           { *enc = 1U; }
    return (beg + 1U);
   }
  }
 }
 return 0U;
}



/* Decodes pointer mode register part: xm0, xm1, xm2, xm3, xh0, xh1, xh2 or
** xh3. Returns the new offset in string, the register encoding (0-7) in enc.
** Returns 0 if there was no valid register at the offset. Skips white spaces
** if needed before the reg. */
static auint opcpr_x4(uint8 const* src, auint beg, auint* enc)
{
 beg = strpr_nextnw(src, beg);
 if (src[beg] == (uint8)('x')){
  beg++;
  if ( (src[beg] == ((uint8)('m'))) || (src[beg] == ((uint8)('h'))) ){
   if (src[beg] == ((uint8)('m'))){ *enc = 0U; }
   else                           { *enc = 4U; }
   beg++;
   if ( (src[beg] >= ((uint8)('0'))) && (src[beg] <= ((uint8)('3'))) ){
    if (!strpr_issym(src[beg + 1U])){
     *enc |= src[beg] - (uint8)('0');
     return (beg + 1U);
    }
   }
  }
 }
 return 0U;
}



/* Processes immediate for addressing mode. Returns 0 if the source line is
** improperly formatted for it, otherwise will return the size of the literal
** or symbol. Encodes the immediate into the setion as appropriate, also
** setting up for symbols if necessary. Sets offset according to the number of
** words output: Assumes first word already pushed, adds second word if
** necessary. Formats address according to immediate mode. The last parameter
** if nonzero forces long immediate coding. Outputs failure informations. */
static auint opcpr_addrimm(symtab_t* stb, auint lng)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  off = section_getoffw(sec) - 1U;
 auint  t;
 auint  u;
 auint  v;

 t = litpr_getval(&(src[0]), &u, &v, stb);
 if (t == LITPR_INV){ goto fault_ot9; }
 if (t == LITPR_STR){ goto fault_lii; }
 if (t == LITPR_UND){           /* Symbol */
  if (symtab_use(stb, v, off, VALWR_A16)){ goto fault_ot9; }
  section_setw(sec, off, 0x0020U); /* Large immediate */
  if (section_pushw(sec, 0xC000U) != 0U){ goto fault_ps9; } /* Prepare second NOP word */
 }else{                         /* Defined symbol, value extracted */
  v&= 0xFFFFU;
  if ((v < 16U) && (!lng)){     /* Small immediate */
   if (valwr_writecs(sec, v, off, VALWR_A4, cst)){ return 0U; }
  }else{                        /* Large immediate */
   section_setw(sec, off, 0x0020U); /* Large immediate */
   if (section_pushw(sec, 0xC000U) != 0U){ goto fault_ps9; } /* Prepare second NOP word */
   if (valwr_writecs(sec, v, off, VALWR_A16, cst)){ return 0U; }
  }
 }
 return u;

 /* Encoding faults */

fault_lii:

 snprintf((char*)(&s[0]), 80U, "Invalid literal in addressing mode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ps9:

 snprintf((char*)(&s[0]), 80U, "No space for second word of immediate");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ot9:

 return 0U;
}



/* Decodes addressing mode and outputs it into the passed section. It only
** alters the addressing mode bits of the first opcode word (must be pushed
** beforehands), and will encode the second as a NOP if necessary. Works
** around symbols if necessary, appropriately. May emit fault. Character
** position within the source line is updated. Returns nonzero (TRUE) on
** success. */
static auint opcpr_addr(symtab_t* stb)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  off = section_getoffw(sec) - 1U;
 auint  beg;
 auint  i;
 auint  e;

 beg = strpr_nextnw(src, 0U);

 /* Register or immediate modes */

 if (src[beg] != (uint8)('[')){ /* Not memory: register or immediate */
  i = opcpr_rx(src, beg, &e);
  if (i != 0U){                 /* Register mode */
   section_setw(sec, off, 0x0030U | e);
   beg = i;
  }else{                        /* Immediate mode */
   i = opcpr_bp(src, beg);
   if (i != 0U){                /* BP relative immediate */
    beg = i;
    section_setw(sec, off, 0x0004U | e);
    src = compst_setcoffrel(cst, beg);
    i = opcpr_addrimm(stb, 1U);
   }else{
    src = compst_setcoffrel(cst, beg);
    i = opcpr_addrimm(stb, 0U);
   }
   if (i == 0U){ goto fault_ot0; }
   beg = i;
   if (src[beg] == (uint8)(']')){ goto fault_in0; }
  }
  compst_setcoffrel(cst, beg);
  return 1U;
 }

 /* Memory modes, Stack */

 beg++;                         /* Pass the opening '[' */
 i = opcpr_bp(src, beg);
 if (i != 0U){                  /* Stack addressing */
  beg = i;
  i = opcpr_rp(src, beg, &e);
  if (i != 0U){                 /* Pointer mode */
   beg = strpr_nextnw(src, i);
   if (src[beg] != (uint8)(']')){ goto fault_par; }
   beg++;
   section_setw(sec, off, 0x003CU | e);
  }else{                        /* Immediate mode */
   src = compst_setcoffrel(cst, beg);
   i = opcpr_addrimm(stb, 0U);
   if (i == 0U){ goto fault_ot0; }
   beg = i;
   if (src[beg] != (uint8)(']')){ goto fault_par; }
   beg++;
   if (off == (section_getoffw(sec) - 1U)){
    section_setw(sec, off, 0x0010U); /* Fix up for stack addressing, imm4 */
   }else{
    section_setw(sec, off, 0x000CU); /* Fix up for stack addressing, imm16 */
   }
  }
  compst_setcoffrel(cst, beg);
  return 1U;
 }

 /* Memory modes, Data */

 beg = strpr_nextnw(src, beg);
 i = opcpr_rp(src, beg, &e);
 if (i != 0U){                  /* Pointer mode */
  beg = strpr_nextnw(src, i);
  if (src[beg] != (uint8)(']')){ goto fault_par; }
  beg++;
  section_setw(sec, off, 0x0038U | e);
 }else{                         /* Immediate mode */
  src = compst_setcoffrel(cst, beg);
  i = opcpr_addrimm(stb, 1U);
  if (i == 0U){ goto fault_ot0; }
  beg = i;
  if (src[beg] != (uint8)(']')){ goto fault_par; }
  beg++;
  section_setw(sec, off, 0x0008U); /* Fix up for data addressing */
 }
 compst_setcoffrel(cst, beg);
 return 1U;

 /* Encoding faults */

fault_par:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Excepted \']\' in addressing mode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_in0:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid operand format in addressing mode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ot0:

 return 0U;

}



/* Special operand modes: Carry "c:" mode */
#define OPCPR_CY    0x01U
/* Special operand modes: XM and XH registers */
#define OPCPR_XM    0x02U
/* Special operand modes: 4 bit parts of XM and XH registers */
#define OPCPR_X4    0x04U
/* Special operand modes: SP, read */
#define OPCPR_SR    0x08U
/* Special operand modes: SP, write */
#define OPCPR_SW    0x10U



/* Encodes register operand with specials. Returns new string offset if
** anything was encoded, zero otherwise. Encodes into the area passed by a
** pointer. */
static auint opcpr_aops_rx(uint8 const* src, auint beg, auint msp, auint* cptr)
{
 auint  i;
 auint  e;

 /* Normal register operand */
 i = opcpr_rx(src, beg, &e);
 if (i != 0U){                  /* Register is found */
  *cptr |= (e << 6);
  return i;
 }

 /* XM or XH register */
 if ((msp & OPCPR_XM) != 0U){
  i = opcpr_xm(src, beg, &e);
  if (i != 0U){                 /* XM or XH found */
   *cptr |= 0x8000U;            /* The special MOV this belongs to */
   *cptr |= (e << 6);
   return i;
  }
 }

 /* Parts of XM or XH register */
 if ((msp & OPCPR_X4) != 0U){
  i = opcpr_x4(src, beg, &e);
  if (i != 0U){                 /* Parts of XM or XH found */
   *cptr |= 0x0400U;            /* The special MOV this belongs to */
   *cptr |= (e << 6);
   return i;
  }
 }

 /* Stack pointer */
 if ((msp & (OPCPR_SR | OPCPR_SW)) != 0U){
  i = opcpr_sp(src, beg);
  if (i != 0U){                 /* Stack pointer found */
   *cptr |= 0x8080U;            /* The special format this belongs to */
   return i;
  }
 }

 return 0U;
}



/* Encodes arithmetic operation parameters. An arithmetic op. here is which is
** formatted according to most in the opcode bit15 = 0 group. Offset must be
** past the opcode. The msp parameter specifies which special operand modes
** are permitted here. The msk parameter provides an OR mask for the first
** opcode word which can be used to form the opcode (allowing for using this
** routine properly with special operands). Returns 0 (FALSE) if failed,
** nonzero (TRUE) if not. The string is required to end here. */
static auint opcpr_aops(symtab_t* stb, auint msp, auint msk)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  off = section_getoffw(sec);
 auint  beg;
 auint  i;
 auint  v = 0U;

 beg = strpr_nextnw(src, 0U);
 if (section_pushw(sec, msk) != 0U){ goto fault_ps1; }

 /* Check for "c:" operand mode */

 if (src[beg] == (uint8)('c')){ /* Note: May be a 'c' register as well! */
  i = strpr_nextnw(src, beg + 1U);
  if (src[i] == (uint8)(':')){  /* This makes it a carry operand mode */
   if ((msp & OPCPR_CY) == 0U){ goto fault_ncy; }
   section_setw(sec, off, 0x4000U); /* Carry operand mode */
   beg = strpr_nextnw(src, i + 1U);
  }
 }

 /* Check for register operand */

 i = opcpr_aops_rx(src, beg, msp & (~OPCPR_SR), &v);
 section_setw(sec, off, v);
 if (i != 0U){                  /* Register is assumed first */
  section_setw(sec, off, 0x0200U);  /* Encoding of reg. first */
  beg = strpr_nextnw(src, i);
  if (src[beg] != (uint8)(',')){ goto fault_com; }
  beg++;
  compst_setcoffrel(cst, beg);
  if (!opcpr_addr(stb)){ return 0U; } /* Failed on addressing mode */
  src = compst_getsstrcoff(cst);
  beg = strpr_nextnw(src, 0U);
  if (!strpr_isend(src[beg])){ goto fault_in1; }
  compst_setcoffrel(cst, beg);
  return 1U;                    /* All OK, encoded */
 }

 /* Check for addressing mode (it should come first then) */

 compst_setcoffrel(cst, beg);
 if (!opcpr_addr(stb)){ return 0U; } /* Failed on addressing mode */
 src = compst_getsstrcoff(cst);
 beg = strpr_nextnw(src, 0U);
 if (src[beg] != (uint8)(',')){ goto fault_com; }
 beg++;
 i = opcpr_aops_rx(src, beg, msp & (~OPCPR_SW), &v);
 section_setw(sec, off, v);
 if (i == 0U){ goto fault_in1; }
 beg = strpr_nextnw(src, i);
 if (!strpr_isend(src[beg])){ goto fault_in1; }
 compst_setcoffrel(cst, beg);
 return 1U;                     /* All OK, encoded */

 /* Encoding faults */

fault_ncy:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Carry operand mode is not allowed here");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_com:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Excepted \',\'");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_in1:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid operand format (arithmetic)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ps1:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "No space for opcode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;
}



/* Encodes bit operations. Here the address format is adr, imm4, encoding
** differently to normal arithmetic. Works similar to opcpr_aops(), except
** this has no special addressing modes. */
static auint opcpr_bops(symtab_t* stb, auint msk)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  off = section_getoffw(sec);
 auint  beg;
 auint  t;
 auint  u;
 uint32 v;

 beg = strpr_nextnw(src, 0U);
 if (section_pushw(sec, msk) != 0U){ goto fault_ps2; }

 /* Check & encode addressing mode (increments code offset) */

 compst_setcoffrel(cst, beg);
 if (!opcpr_addr(stb)){ return 0U; } /* Failed on addressing mode */
 src = compst_getsstrcoff(cst);
 beg = strpr_nextnw(src, 0U);
 if (src[beg] != (uint8)(',')){ goto fault_cm2; }
 beg++;
 beg = strpr_nextnw(src, beg);

 /* Check & encode literal and apply it (at the original code offset) */

 t = litpr_getval(&(src[beg]), &u, &v, stb);
 if (t == LITPR_INV){ goto fault_ot2; }
 if (t == LITPR_STR){ goto fault_in2; }
 if (t == LITPR_UND){           /* Symbol, undefined */
  if (symtab_use(stb, v, off, VALWR_B4)){ goto fault_ot2; }
 }else{                         /* Defined symbol, value extracted */
  src = compst_setcoffrel(cst, beg);
  beg = 0U;
  if (valwr_writecs(sec, v, off, VALWR_B4, cst)){ return 0U; }
 }

 /* Check end of string */

 beg = beg + u;
 if (!strpr_isend(src[beg])){ goto fault_in2; }
 return 1U;                     /* All OK, encoded */

fault_cm2:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Excepted \',\'");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_in2:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid operand format (bit)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ps2:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "No space for opcode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ot2:

 return 0U;
}



/* Encodes functions. Similar to the other encoders above. It properly goes
** through the parameter list. The 'sv' parameter if true indicates this is
** for a supervisor call, so there is no call address. */
static auint opcpr_cops(symtab_t* stb, auint sv, auint msk)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  pof = section_getoffw(sec);
 auint  beg;

 beg = strpr_nextnw(src, 0U);
 if (section_pushw(sec, msk) != 0U){ goto fault_ps3; }

 /* If it is not a supervisor call, encode call target */

 if (!sv){
  compst_setcoffrel(cst, beg);
  if (!opcpr_addr(stb)){ return 0U; } /* Failed on addressing mode */
  src = compst_getsstrcoff(cst);
  beg = strpr_nextnw(src, 0U);
 }

 /* From here encode parameters, as many as comes. Empty {} is also allowed */

 if (src[beg] == (uint8)('{')){
  beg = strpr_nextnw(src, beg + 1U);
  if (src[beg] != (uint8)('}')){
   while (1){
    pof = section_getoffw(sec);
    if (section_pushw(sec, 0xC000U) != 0U){ goto fault_ps3; } /* Parameters should form as NOPs */
    compst_setcoffrel(cst, beg);
    if (!opcpr_addr(stb)){ return 0U; } /* Failed on addressing mode */
    src = compst_getsstrcoff(cst);
    beg = strpr_nextnw(src, 0U);
    if (src[beg] != (uint8)(',')){
     if (src[beg] == (uint8)('}')){ break; }
     goto fault_cmb;
    }
    beg++;
    beg = strpr_nextnw(src, beg);
   };
  }
  beg++;
 }
 section_setw(sec, pof, 0x0040U); /* Terminate parameter list */

 /* Check end of string */

 beg = strpr_nextnw(src, beg);
 if (!strpr_isend(src[beg])){ goto fault_in3; }
 return 1U;                     /* All OK, encoded */

fault_cmb:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Excepted \',\' or \'}\'");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_in3:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid operand format (function)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ps3:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "No space for opcode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;
}



/* Encodes jump operands. Similar to the other encoders above. */
static auint opcpr_jops(symtab_t* stb, auint msk)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  beg;

 beg = strpr_nextnw(src, 0U);
 if (section_pushw(sec, msk) != 0U){ goto fault_ps4; }

 /* Note: Currently proper address translation for immediate relative jumps is
 ** not included (VALWR_R16 would be the way to implement it). */

 compst_setcoffrel(cst, beg);
 if (!opcpr_addr(stb)){ return 0U; } /* Failed on addressing mode */
 src = compst_getsstrcoff(cst);

 /* Check end of string */

 beg = strpr_nextnw(src, 0U);
 if (!strpr_isend(src[beg])){ goto fault_in4; }
 return 1U;                     /* All OK, encoded */

fault_in4:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid operand format (jump)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ps4:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "No space for opcode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;
}



/* Encodes short relative jump operand. Similar to the other encoders above. */
static auint opcpr_rops(symtab_t* stb, auint msk)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  off = section_getoffw(sec);
 auint  beg;
 auint  t;
 auint  u;
 uint32 v;

 beg = strpr_nextnw(src, 0U);
 if (section_pushw(sec, msk) != 0U){ goto fault_ps5; }

 t = litpr_getval(&(src[beg]), &u, &v, stb);
 if (t == LITPR_INV){ goto fault_ot5; }
 if (t == LITPR_STR){ goto fault_in5; }
 if (t == LITPR_UND){           /* Symbol, undefined */
  if (symtab_use(stb, v, off, VALWR_R10)){ goto fault_ot5; }
 }else{                         /* Defined symbol, value extracted */
  src = compst_setcoffrel(cst, beg);
  beg = 0U;
  if (valwr_writecs(sec, v, off, VALWR_R10, cst)){ return 0U; }
 }

 /* Check end of string */

 beg = strpr_nextnw(src, beg + u);
 if (!strpr_isend(src[beg])){ goto fault_in5; }
 return 1U;                     /* All OK, encoded */

fault_in5:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid operand format (relative jump)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ps5:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "No space for opcode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ot5:

 return 0U;
}



/* Encodes nop. Similar to the other encoders above. */
static auint opcpr_nops(symtab_t* stb, auint msk)
{
 uint8  s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  beg;

 beg = strpr_nextnw(src, 0U);
 if (section_pushw(sec, msk) != 0U){ goto fault_ps6; }

 /* Check end of string */

 if (!strpr_isend(src[beg])){ goto fault_in6; }
 return 1U;                     /* All OK, encoded */

fault_in6:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid operand format (nop)");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ps6:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "No space for opcode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;
}



/* Attempts to process an opcode beginning at the current position in the
** source line of the compile state. It uses the offset in the current section
** to write into the passed code memory block, and increments the offset
** afterwards. While processing it also deals with any symbol or literal
** encountered, encodes them or submits them to pass2 as needed. Generates and
** outputs faults where necessary. Returns one of the defined PARSER return
** codes (defined in types.h). */
auint opcpr_proc(symtab_t* stb)
{
 uint8        s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint        beg = strpr_nextnw(src, 0U);
 auint        r;

 if (section_getsect(sec) != SECT_CODE){
  snprintf((char*)(&s[0]), 80U, "Probable code in non code section");
  fault_printat(FAULT_FAIL, &s[0], cst);
  return PARSER_ERR;
 }

 if (strpr_isend(src[beg])){
  return PARSER_END; /* No content on this line */
 }

 /* Encode opcodes */

 if       (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("add"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY | OPCPR_SW, 0x0800U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("adc"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x1800U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("and"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0x4400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("asr"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x3400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("btc"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_bops(stb, 0xA000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("bts"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_bops(stb, 0xA800U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("div"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x1400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jfr"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_cops(stb, 0U, 0x8800U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jfa"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_cops(stb, 0U, 0x8900U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jmr"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_jops(stb, 0x8C00U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jma"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_jops(stb, 0x8D00U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jms"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_rops(stb, 0x8400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jsv"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_cops(stb, 1U, 0x8880U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("mac"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x3000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("mov"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY | OPCPR_XM | OPCPR_X4 | OPCPR_SR | OPCPR_SW, 0x0000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("mul"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x2000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("neg"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0x6400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("nop"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_nops(stb, 0xC000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("not"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0x2400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("or" ))){

  compst_setcoffrel(cst, beg + 2U);
  r = opcpr_aops(stb, 0U, 0x4000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("rfn"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_nops(stb, 0x8980U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("sbc"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x1C00U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("shl"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x2800U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("shr"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x2C00U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("slc"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x3800U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("src"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x3C00U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("sub"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, OPCPR_CY, 0x0C00U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xbc"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_bops(stb, 0xA400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xbs"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_bops(stb, 0xAC00U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xch"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0x1000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xeq"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0xB000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xne"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0xB800U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xor"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0x5000U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xsg"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0xB400U);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xug"))){

  compst_setcoffrel(cst, beg + 3U);
  r = opcpr_aops(stb, 0U, 0xBC00U);

 }else{

  compst_setcoffrel(cst, beg);
  snprintf((char*)(&s[0]), 80U, "Invalid opcode");
  fault_printat(FAULT_FAIL, &s[0], cst);
  r = 0U;

 }

 if (r != 0U){ return PARSER_END; }
 else        { return PARSER_ERR; }
}
