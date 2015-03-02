/**
**  \file
**  \brief     Opcode decoding logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.03.02
*/


#include "opcdec.h"
#include "fault.h"
#include "litpr.h"
#include "strpr.h"



/* Decodes pointer register name: x0, x1, x2 or x3. Returns the new offset in
** string, the register encoding (0-3) in enc. Returns 0 if there was no valid
** register at the offset. Skips white spaces before and after the reg. */
static auint opcdec_rp(uint8 const* src, auint beg, auint* enc)
{
 beg = strpr_nextnw(src, beg);
 if (src[beg] == (uint8)('x')){
  beg++;
  if ( (src[beg] >= ((uint8)('0'))) && (src[beg] <= ((uint8)('3'))) ){
   if (!strpr_issym(src[beg + 1U])){
    *enc = src[beg] - (uint8)('0');
    return (strpr_nextnw(src, beg + 1U));
   }
  }
 }
 return 0U;
}



/* Decodes RX register name: a, b, c, d, x0, x1, x2 or x3. Returns the new
** offset in string, the register encoding (0-7) in enc. Returns 0 if there
** was no valid register at the offset. Skips white spaces before and after
** the register. */
static auint opcdec_rx(uint8 const* src, auint beg, auint* enc)
{
 auint r = opcdec_rp(src, beg, enc);
 if (r != 0U){ /* Pointer register found */
  *enc = *enc + 4U;
  return r;
 }
 /* No pointer register, check normal registers */
 beg = strpr_nextnw(src, beg);
 if ( (src[beg] >= ((uint8)('a'))) && (src[beg] <= ((uint8)('d'))) ){
  if (!strpr_issym(src[beg + 1U])){
   *enc = src[beg] - (uint8)('a');
   return (strpr_nextnw(src, beg + 1U));
  }
 }
 return 0U;
}



/* Checks for bp relative addressing looking for 'bp+' or '$' in the string.
** Returns the new offset in string, or 0 if not found. Skips whitespaces
** before and after. */
static auint opcdec_bp(uint8 const* src, auint beg)
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

 }else{

  if (src[beg] == (uint8)('$')){
   beg = strpr_nextnw(src, beg + 1U);
   return beg;
  }

 }

 return 0U;
}



/* Checks for 'sp' register name. Returns the new offset in string, or 0 if
** not found. Skips whitespaces before and after. */
static auint opcdec_sp(uint8 const* src, auint beg)
{
 beg = strpr_nextnw(src, beg);
 if (src[beg] == (uint8)('s')){
  beg++;
  if (src[beg] == (uint8)('p')){
   beg++;
   if (!strpr_issym(src[beg])){
    return (strpr_nextnw(src, beg));
   }
  }
 }
 return 0U;
}



/* Decodes pointer mode register name: xm or xh. Returns the new offset in
** string, the register encoding (0-1) in enc. Returns 0 if there was no valid
** register at the offset. Skips white spaces before and after the reg. */
static auint opcdec_xm(uint8 const* src, auint beg, auint* enc)
{
 beg = strpr_nextnw(src, beg);
 if (src[beg] == (uint8)('x')){
  beg++;
  if ( (src[beg] == ((uint8)('m'))) || (src[beg] == ((uint8)('h'))) ){
   if (!strpr_issym(src[beg + 1U])){
    if (src[beg] == ((uint8)('m'))){ *enc = 0U; }
    else                           { *enc = 1U; }
    return (strpr_nextnw(src, beg + 1U));
   }
  }
 }
 return 0U;
}



/* Decodes pointer mode register part: xm0, xm1, xm2, xm3, xh0, xh1, xh2 or
** xh3. Returns the new offset in string, the register encoding (0-7) in enc.
** Returns 0 if there was no valid register at the offset. Skips white spaces
** before and after the reg. */
static auint opcdec_x4(uint8 const* src, auint beg, auint* enc)
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
     return (strpr_nextnw(src, beg + 1U));
    }
   }
  }
 }
 return 0U;
}



/* Processes immediate for addressing mode. Returns 0 if the source line is
** improperly formatted for it, otherwise will return the string size of the
** literal or symbol, trailing whitespaces stripped. Adds the result to the
** passed operand value, setting OPCDEC_O_SYM as needed, and the low 24 bits
** according to the immediate or symbol definition. Other bits are not
** changed. Outputs failure informations. */
static auint opcdec_addrimm(symtab_t* stb, auint* opv)
{
 uint8  s[80];
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  t;
 auint  u;
 auint  v;

 *opv &= ~(OPCDEC_O_SYM | 0xFFFFFFU); /* Clear bits which will be set here */

 t = litpr_getval(&(src[0]), &u, &v, stb);
 if (t == LITPR_INV){ goto fault_ot9; }
 if (t == LITPR_STR){ goto fault_lii; }
 if (t == LITPR_UND){           /* Symbol */
  *opv |= OPCDEC_O_SYM | v;
 }else{                         /* Defined symbol, value extracted */
  *opv |= v & 0xFFFFU;
 }
 return (strpr_nextnw(src, u));

 /* Encoding faults */

fault_lii:

 snprintf((char*)(&s[0]), 80U, "Invalid literal in addressing mode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ot9:

 return 0U;
}



/* Decodes addressing mode and generates operand value accordingly,
** completely. May emit fault. Character position within the source line is
** updated (any trailing whitespaces are skimmed over). Returns nonzero (TRUE)
** on success. */
static auint opcdec_addr(symtab_t* stb, auint* opv)
{
 uint8  s[80];
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  beg;
 auint  i;
 auint  e;

 beg = strpr_nextnw(src, 0U);
 *opv = 0U;

 /* Register or immediate modes */

 if (src[beg] != (uint8)('[')){ /* Not memory: register or immediate */

  i = opcdec_rx(src, beg, &e);
  if (i != 0U){                 /* Register mode */
   *opv = (0x30U | e) << OPCDEC_S_ADR;
  }else{                        /* Immediate mode */
   i = opcdec_bp(src, beg);
   if (i != 0U){                /* BP relative immediate */
    *opv = (0x24U    ) << OPCDEC_S_ADR;
   }else{
    *opv = (0x20U    ) << OPCDEC_S_ADR;
   }
   beg = i;
   src = compst_setcoffrel(cst, beg);
   i = opcdec_addrimm(stb, opv);
   if (i == 0U){ goto fault_ot0; }
  }
  beg = i;
  if (src[beg] == (uint8)(']')){ goto fault_brx; }

 }else{                         /* Memory addressing */

  beg++;                        /* Pass the opening '[' */
  i = opcdec_bp(src, beg);
  if (i != 0U){                 /* Stack addressing */

   beg = i;
   i = opcdec_rp(src, beg, &e);
   if (i != 0U){                /* Pointer mode */
    *opv = (0x3CU | e) << OPCDEC_S_ADR;
   }else{                       /* Immediate mode */
    *opv = (0x2CU    ) << OPCDEC_S_ADR;
    src = compst_setcoffrel(cst, beg);
    i = opcdec_addrimm(stb, opv);
    if (i == 0U){ goto fault_ot0; }
   }

  }else{                        /* Data addressing */

   i = opcdec_rp(src, beg, &e);
   if (i != 0U){                /* Pointer mode */
    *opv = (0x38U | e) << OPCDEC_S_ADR;
   }else{                       /* Immediate mode */
    *opv = (0x28U    ) << OPCDEC_S_ADR;
    src = compst_setcoffrel(cst, beg);
    i = opcdec_addrimm(stb, opv);
    if (i == 0U){ goto fault_ot0; }
   }

  }

  beg = i;
  if (src[beg] != (uint8)(']')){ goto fault_bre; }
  beg = strpr_nextnw(src, beg + 1U);

 }

 compst_setcoffrel(cst, beg);
 return 1U;

 /* Encoding faults */

fault_bre:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Excepted \']\' in addressing mode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_brx:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Invalid \']\' in addressing mode");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_ot0:

 return 0U;

}



/* Decodes addressing mode with support for special registers, and generates
** operand value accordingly, completely. May emit fault. Character position
** within the source line is updated (any trailing whitespaces are skimmed
** over). Returns nonzero (TRUE) on success. */
static auint opcdec_addrx(symtab_t* stb, auint* opv)
{
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  beg;
 auint  i;
 auint  e;

 beg = strpr_nextnw(src, 0U);
 *opv = 0U;

 /* XM or XH register */
 i = opcdec_xm(src, beg, &e);
 if (i != 0U){                 /* XM or XH found */
  *opv = OPCDEC_X_XM | e;
 }else{

  /* Parts of XM or XH register */
  i = opcdec_x4(src, beg, &e);
  if (i != 0U){                /* Parts of XM or XH found */
   *opv = OPCDEC_X_XM0 | e;
  }else{

   /* Stack pointer */
   i = opcdec_sp(src, beg);
   if (i != 0U){               /* Stack pointer found */
    *opv = OPCDEC_X_SP;
   }else{

    /* Normal address */
    return opcdec_addr(stb, opv);

   }
  }
 }

 beg = i;
 compst_setcoffrel(cst, beg);
 return 1U;
}



/* Decodes operand and parameter list. Simply goes through the remaining
** length of the source line, taking up to 2 operands and 16 function
** parameters, adding those to the opcode descriptor structure. May emit
** fault. Returns nonzero (TRUE) on success. */
static auint opcdec_oplist(symtab_t* stb, opcdec_ds_t* ods)
{
 uint8  s[80];
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  beg;
 auint  i;

 beg = strpr_nextnw(src, 0U);

 /* Decode up to two operands */

 i = 0U;
 if ( (!strpr_isend(src[beg])) &&
      (src[beg] != (uint8)('{')) ){
  while (1){
   if (!opcdec_addrx(stb, &(ods->op[i]))){ return 0U; } /* Failed */
   src = compst_getsstrcoff(cst);
   beg = 0U;
   i++;
   if ( (strpr_isend(src[beg])) ||
        (src[beg] == (uint8)('{')) ){ break; }
   if (src[beg] != (uint8)(',')){ goto fault_cma; }
   if (i >= 2U){                  goto fault_opm; }
   beg = strpr_nextnw(src, beg + 1U);
   compst_setcoffrel(cst, beg);
  }
 }
 ods->opc = i;

 /* Decode up to 16 function parameters */

 i = 0U;
 if (src[beg] == (uint8)('{')){
  beg = strpr_nextnw(src, beg + 1U);
  compst_setcoffrel(cst, beg);
  if (src[beg] != (uint8)('}')){
   while (1){
    if (!opcdec_addr(stb, &(ods->pr[i]))){ return 0U; } /* Failed */
    src = compst_getsstrcoff(cst);
    beg = 0U;
    i++;
    if (src[beg] == (uint8)('}')){ break; }
    if (src[beg] != (uint8)(',')){ goto fault_cma; }
    if (i >= 16U){                 goto fault_prm; }
    beg = strpr_nextnw(src, beg + 1U);
    compst_setcoffrel(cst, beg);
   }
  }
  beg = strpr_nextnw(src, beg + 1U);
 }
 ods->prc = i;

 /* Check end of string */

 if (!strpr_isend(src[beg])){ goto fault_in3; }
 return 1U;                     /* All OK, encoded */

fault_cma:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Excepted \',\'");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_opm:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Too many operands");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_prm:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Too many parameters");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

fault_in3:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Excess content in instruction");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;

}



/* Decodes an instruction after the opcode was determined. The opcode ID is
** to be provided in the "id" parameter. Assumes that the source line position
** is still at the opcode, which it skips. May emit fault. Returns nonzero
** (TRUE) on success. */
static auint opcdec_dec(symtab_t* stb, opcdec_ds_t* ods, auint id)
{
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint  beg;
 auint  i;

 beg = strpr_nextnw(src, 0U);
 while (src[beg] > (uint8)(' ')){ beg++; }  /* Skip opcode */
 beg = strpr_nextnw(src, beg);
 ods->id = id;

 /* Check for "c:" operand mode */

 if (src[beg] == (uint8)('c')){ /* Note: May be a 'c' register as well! */
  i = strpr_nextnw(src, beg + 1U);
  if (src[i] == (uint8)(':')){  /* This makes it a carry operand mode */
   ods->id |= OPCDEC_I_C;
   beg = strpr_nextnw(src, i + 1U);
  }
 }

 /* Normal parameter list processing */

 compst_setcoffrel(cst, beg);
 return opcdec_oplist(stb, ods);
}



/* Attempts to decode an opcode beginning at the current position in the
** source line of the compile state. While processing it also deals with any
** symbol or literal encountered, evaulates them or submits them to pass2 as
** needed. Generates and outputs faults where necessary. Returns nonzero
** (TRUE) on success. If the line contains no opcode, OPCDEC_I_NUL is placed
** in ods->id. */
auint opcdec_proc(symtab_t* stb, opcdec_ds_t* ods)
{
 uint8        s[80];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint        beg = strpr_nextnw(src, 0U);
 auint        r;
 auint        t;

 ods->id = OPCDEC_I_NUL; /* No instruction */

 if (section_getsect(sec) != SECT_CODE){
  snprintf((char*)(&s[0]), 80U, "Probable code in non code section");
  fault_printat(FAULT_FAIL, &s[0], cst);
  return 1U;
 }

 if (strpr_isend(src[beg])){
  return 0U;             /* No content on this line */
 }

 /* Encode opcodes */

 if       (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("add"))){

  r = opcdec_dec(stb, ods, 0x0800U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("adc"))){

  r = opcdec_dec(stb, ods, 0x1800U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("and"))){

  r = opcdec_dec(stb, ods, 0x8800U | OPCDEC_I_R);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("asr"))){

  r = opcdec_dec(stb, ods, 0x3000U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("btc"))){

  r = opcdec_dec(stb, ods, 0xA000U | OPCDEC_I_RB);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("bts"))){

  r = opcdec_dec(stb, ods, 0xA800U | OPCDEC_I_RB);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("div"))){

  r = opcdec_dec(stb, ods, 0x1400U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jfr"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_JFR);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jfa"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_JFA);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jmr"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_JMR);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jma"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_JMA);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jms"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_JMS);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("jsv"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_JSV);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("mac"))){

  r = opcdec_dec(stb, ods, 0x3400U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("mov"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_MOV);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("mul"))){

  r = opcdec_dec(stb, ods, 0x2400U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("neg"))){

  r = opcdec_dec(stb, ods, 0x6000U | OPCDEC_I_R);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("nop"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_NOP);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("not"))){

  r = opcdec_dec(stb, ods, 0x2000U | OPCDEC_I_R);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("or" ))){

  r = opcdec_dec(stb, ods, 0x1000U | OPCDEC_I_R);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("rfn"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_RFN);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("sbc"))){

  r = opcdec_dec(stb, ods, 0x1C00U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("shl"))){

  r = opcdec_dec(stb, ods, 0x2C00U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("shr"))){

  r = opcdec_dec(stb, ods, 0x2800U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("slc"))){

  r = opcdec_dec(stb, ods, 0x3C00U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("src"))){

  r = opcdec_dec(stb, ods, 0x3800U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("sub"))){

  r = opcdec_dec(stb, ods, 0x0C00U | OPCDEC_I_R | OPCDEC_I_RC);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xbc"))){

  r = opcdec_dec(stb, ods, 0xA400U | OPCDEC_I_RB);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xbs"))){

  r = opcdec_dec(stb, ods, 0xAC00U | OPCDEC_I_RB);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xch"))){

  r = opcdec_dec(stb, ods, 0x0400U | OPCDEC_I_RS);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xeq"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_XEQ);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xne"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_XNE);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xns"))){

  r = opcdec_dec(stb, ods, 0xB200U | OPCDEC_I_RS);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xor"))){

  r = opcdec_dec(stb, ods, 0x5000U | OPCDEC_I_R);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xsg"))){

  r = opcdec_dec(stb, ods, 0xB400U | OPCDEC_I_R);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xsl"))){

  r = opcdec_dec(stb, ods, 0xB400U | OPCDEC_I_R);
  t = ods->op[0];          /* Swap operands (inverse of "xsg") */
  ods->op[0] = ods->op[1];
  ods->op[1] = t;

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xst"))){

  r = opcdec_dec(stb, ods, 0xB000U | OPCDEC_I_RS);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xug"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_XUG);

 }else if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("xul"))){

  r = opcdec_dec(stb, ods, OPCDEC_I_XUG);
  t = ods->op[0];          /* Swap operands (inverse of "xug") */
  ods->op[0] = ods->op[1];
  ods->op[1] = t;

 }else{

  compst_setcoffrel(cst, beg);
  snprintf((char*)(&s[0]), 80U, "Invalid opcode");
  fault_printat(FAULT_FAIL, &s[0], cst);
  r = 0U;

 }

 return r;
}
