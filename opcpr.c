/**
**  \file
**  \brief     Opcode processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.02.26
*/


#include "opcpr.h"
#include "opcdec.h"
#include "fault.h"
#include "valwr.h"



/* Try to push a value (section_pushw), printing a fault if not successful.
** Returns nonzero (TRUE) on success. */
static auint opcpr_pushw(symtab_t* stb, auint opv)
{
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 if (section_pushw(sec, opv) != 0U){
  fault_printat(FAULT_FAIL, (uint8 const*)("No space for opcode"), cst);
  return 0U;
 }
 return 1U;
}



/* Write out address by the passed opcode field. It only alters the addressing
** mode bits of the first opcode word (must be pushed beforehands), and will
** encode the second as a NOP if necessary. The "use" parameter selects the
** usage of the immediate encoded, can be VALWR_A16 or VALWR_R16. Works around
** symbols if necessary, appropriately. May emit fault. Returns nonzero (TRUE)
** on success. */
static auint opcpr_addr(symtab_t* stb, auint opv, auint use)
{
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 auint  off = section_getoffw(sec) - 1U;
 auint  adr = (opv >> OPCDEC_S_ADR) & 0x3FU;

 /* Check if it is a proper address */

 if (adr == OPCDEC_A_SPC){
  fault_printat(FAULT_FAIL, (uint8 const*)("Invalid operand format in addressing mode"), cst);
  return 0U;
 }

 /* Check for short forms and encode those */

 if ( (adr == 0x20U) ||         /* Immediate */
      (adr == 0x2CU) ){         /* Stack space */
  if ( ((opv & OPCDEC_O_SYM) == 0U) &&
       ((opv & 0xFFFFU) < 0x10U) &&
       (use != VALWR_R16) ){    /* Can actually encode short form */
   if (adr == 0x20U){ adr = 0x00U; }
   else             { adr = 0x10U; }
   adr |= opv & 0xFU;
   section_setw(sec, off, adr);
   return 1U;                   /* Successfully encoded it */
  }
 }

 /* Check for other non-imm (1 word) forms, and encode those */

 if ((adr & 0x30U) != 0x20U){
  section_setw(sec, off, adr);
  return 1U;                    /* Successfully encoded it */
 }

 /* Long immediate forms remained only. Try to encode those */

 section_setw(sec, off, adr);
 if (opcpr_pushw(stb, 0xC000U) == 0U){ return 0U; } /* Prepare second NOP word */
 if ((opv & OPCDEC_O_SYM) != 0U){
  if (symtab_use(stb, opv & 0xFFFFFFU, off, use)){ return 0U; }
 }else{
  if (valwr_writecs(sec, opv & 0xFFFFU, off, use, cst)){ return 0U; }
 }
 return 1U;                     /* Successfully encoded it */
}



/* Check parameter count for zero (non-function). Returns nonzero (TRUE) if
** it is zero, otherwise outputs an appropriate fault, too. */
static auint opcpr_nofunc(symtab_t* stb, opcdec_ds_t* ods)
{
 compst_t*    cst = symtab_getcompst(stb);
 if (ods->prc == 0U){ return 1U; }
 fault_printat(FAULT_FAIL, (uint8 const*)("Instruction is not a function call"), cst);
 return 0U;
}



/* Check operand count for the given value. Returns nonzero (TRUE) if it is
** OK, otherwise outputs an appropriate fault, too. */
static auint opcpr_opcount(symtab_t* stb, opcdec_ds_t* ods, auint cnt)
{
 uint8        s[80];
 compst_t*    cst = symtab_getcompst(stb);
 if (ods->opc == cnt){ return 1U; }
 snprintf((char*)(&s[0]), 80U, "Instruction requires %i operands", cnt);
 fault_printat(FAULT_FAIL, &s[0], cst);
 return 0U;
}



/* Check for carry request to reject it. Returns nonzero (TRUE) if there is
** no carry request, otherwise outputs an appropriate fault, too. */
static auint opcpr_nocy(symtab_t* stb, opcdec_ds_t* ods)
{
 compst_t*    cst = symtab_getcompst(stb);
 if ((ods->id & OPCDEC_I_C) == 0U){ return 1U; }
 fault_printat(FAULT_FAIL, (uint8 const*)("Instruction can not produce carry"), cst);
 return 0U;
}



/* Encode two operand instruction's operands selecting between "adr, rx" and
** "rx, adr" encodings appropriately if it is selectable (swp is nonzero). May
** produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_eops(symtab_t* stb, opcdec_ds_t* ods, auint swp)
{
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 auint  off = section_getoffw(sec) - 1U;
 auint  reg;
 auint  adr;

 /* Check count of operands: must be two */

 if (opcpr_opcount(stb, ods, 2U) == 0U){ return 0U; }

 /* Select between the two possible orders */

 reg = ods->op[1];
 adr = ods->op[0];
 if (((reg >> OPCDEC_S_ADR) & 0x38U) != 0x30U){ /* Source must be non-reg */
  if (swp){ section_setw(sec, off, 0x0200U); }  /* "rx, adr" order */
  reg = ods->op[0];
  adr = ods->op[1];
  if (((reg >> OPCDEC_S_ADR) & 0x38U) != 0x30U){
   fault_printat(FAULT_FAIL, (uint8 const*)("One of the operands must be register"), cst);
   return 0U;
  }
 }

 /* Encode */

 section_setw(sec, off, ((reg >> OPCDEC_S_ADR) & 0x7U) << 6); /* Add the register */
 return opcpr_addr(stb, adr, VALWR_A16);
}



/* Process Regular and Regular with Carry instructions. May produce fault.
** Returns nonzero (TRUE) on success. */
static auint opcpr_ir(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);

 /* Check count of parameters: must be zero */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }

 /* Add opcode */

 if (opcpr_pushw(stb, (ods->id) & 0xFFFFU) == 0U){ return 0U; }

 /* Add carry if requested */

 if (((ods->id) & OPCDEC_I_RC) == 0U){
  if (opcpr_nocy(stb, ods) == 0U){ return 0U; }
 }
 if (((ods->id) & OPCDEC_I_C) != 0U){
  section_setw(sec, off, 0x4000U);
 }

 /* Encode operands */

 return opcpr_eops(stb, ods, 1U);
}



/* Process Regular bit instructions. May produce fault. Returns nonzero (TRUE)
** on success. */
static auint opcpr_irb(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 auint  off = section_getoffw(sec);
 auint  opv;

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 2U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Add opcode */

 if (opcpr_pushw(stb, (ods->id) & 0xFFFFU) == 0U){ return 0U; }

 /* Second operand is the bit to select, encode it */

 opv = ods->op[1];
 if ((opv & OPCDEC_O_SYM) != 0U){
  if (symtab_use(stb, opv & 0xFFFFFFU, off, VALWR_B4)){ return 0U; }
 }else if (((opv >> OPCDEC_S_ADR) & 0x3FU) == 0x20U){
  if (valwr_writecs(sec, opv & 0xFFFFU, off, VALWR_B4, cst)){ return 0U; }
 }else{
  fault_printat(FAULT_FAIL, (const uint8*)("Invalid operand for bit select"), cst);
  return 0U;
 }

 /* First operand is the address */

 return opcpr_addr(stb, ods->op[0], VALWR_A16);
}



/* Process Regular symmetric instructions. May produce fault. Returns nonzero
** (TRUE) on success. */
static auint opcpr_irs(symtab_t* stb, opcdec_ds_t* ods)
{
 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Add opcode */

 if (opcpr_pushw(stb, (ods->id) & 0xFFFFU) == 0U){ return 0U; }

 /* Encode operands */

 return opcpr_eops(stb, ods, 0U);
}



/* Encode NOP. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_inop(symtab_t* stb, opcdec_ds_t* ods)
{
 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 0U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode */

 return opcpr_pushw(stb, 0xC000U);
}



/* Encode MOV. Many special cases. May produce fault. Returns nonzero (TRUE)
** on success. */
static auint opcpr_imov(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 auint  off = section_getoffw(sec);
 auint  reg;
 auint  adr;
 auint  v;

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 2U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Push empty for now, add MOV parts further on */

 if (opcpr_pushw(stb, 0x0000U) == 0U){ return 0U; }

 /* One of the operands always must be an addressing mode (which may branch
 ** out for special immediate encodings, but technically could be encoded as
 ** a normal immediate). Isolate it, so the other operand may be observed
 ** for register. Also apply opcode order select. */

 reg = ods->op[1];
 adr = ods->op[0];
 if ( (((adr >> OPCDEC_S_ADR) & 0x3FU) == OPCDEC_A_SPC) ||   /* Adr is a spec. register */
      ( (((reg >> OPCDEC_S_ADR) & 0x3FU) != OPCDEC_A_SPC) && /* Reg is not special reg. */
        (((reg >> OPCDEC_S_ADR) & 0x38U) != 0x30U) ) ){      /* Reg is neither normal reg. */
  section_setw(sec, off, 0x0200U); /* "rx, adr" order */
  reg = ods->op[0];
  adr = ods->op[1];
 }else{                            /* "adr, rx" order */
  /* Note: technically instructions where the target is an immediate are NOPs,
  ** but supporting them would complicate special immediate encoding, and they
  ** are not practical, anyway. So bail out. */
  if (((adr >> OPCDEC_S_ADR) & 0x38U) == 0x20U){
   fault_printat(FAULT_FAIL, (uint8 const*)("Immediate as target in MOV is not supported"), cst);
   return 0U;
  }
  /* If reg is not a register (or special), then this can not be encoded. */
  if ( (((reg >> OPCDEC_S_ADR) & 0x3FU) != OPCDEC_A_SPC) &&  /* Special reg. */
       (((reg >> OPCDEC_S_ADR) & 0x38U) != 0x30U) ){         /* Normal reg. */
   fault_printat(FAULT_FAIL, (uint8 const*)("One of the operands must be register"), cst);
   return 0U;
  }
 }
 /* If adr remained a special register, then both operands were special regs.
 ** which is invalid. */
 if (((adr >> OPCDEC_S_ADR) & 0x3FU) == OPCDEC_A_SPC){ /* Special reg. */
  fault_printat(FAULT_FAIL, (uint8 const*)("Both operands can not be special registers"), cst);
  return 0U;
 }

 /* It is possible to encode the MOV now: adr contains an addressing mode (so
 ** opcpr_addr can encode it), and reg either a normal or a spec. register. */

 if (((reg >> OPCDEC_S_ADR) & 0x38U) == 0x30U){        /* Normal reg. */

  if ( (((adr >> OPCDEC_S_ADR) & 0x38U) == 0x20U) &&   /* Immediate: specials here */
       ((adr & OPCDEC_O_SYM) == 0U) ){                 /* Only if not symbol (so value present) */

   v = adr & 0xFFFFU;              /* Value to encode */

   if (v >= 0xFFF0U){              /* NOT rx, adr */
    section_setw(sec, off, 0x2000U | (((reg >> OPCDEC_S_ADR) & 0x7U) << 6) | (~v));
    return 1U;
   }

  }

  /* Encode ordinary MOV */

  section_setw(sec, off, 0x0000U | (((reg >> OPCDEC_S_ADR) & 0x7U) << 6));
  return opcpr_addr(stb, adr, VALWR_A16);
 }

 /* Special register MOVs */

 if ((reg & (OPCDEC_E_SP | OPCDEC_E_XM | OPCDEC_E_XH)) != 0U){
  section_setw(sec, off, 0x8000U | ((reg & 0x7U) << 6));
  return opcpr_addr(stb, adr, VALWR_A16);
 }

 if ((reg & (OPCDEC_E_XM0 | OPCDEC_E_XM1 | OPCDEC_E_XM2 | OPCDEC_E_XM3 |
             OPCDEC_E_XH0 | OPCDEC_E_XH1 | OPCDEC_E_XH2 | OPCDEC_E_XH3)) != 0U){
  section_setw(sec, off, 0x4000U | ((reg & 0x7U) << 6));
  return opcpr_addr(stb, adr, VALWR_A16);
 }

 fault_printat(FAULT_FAIL, (uint8 const*)("Invalid MOV"), cst);
 return 0U;
}



/* Encode JMS. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijms(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 auint  off = section_getoffw(sec);
 auint  opv = (ods->op[0]);

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 1U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Check operand: must be an immediate */

 if (((opv >> OPCDEC_S_ADR) & 0x38U) != 0x20U){
  fault_printat(FAULT_FAIL, (uint8 const*)("Operand must be immediate"), cst);
  return 0U;
 }

 /* Encode */

 if (opcpr_pushw(stb, 0x8C00U) == 0U){ return 0U; }
 if ((opv & OPCDEC_O_SYM) != 0U){
  if (symtab_use(stb, opv & 0xFFFFFFU, off, VALWR_R10)){ return 0U; }
 }else{
  if (valwr_writecs(sec, opv & 0xFFFFU, off, VALWR_R10, cst)){ return 0U; }
 }
 return 1U;
}



/* Encode JMR. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijmr(symtab_t* stb, opcdec_ds_t* ods)
{
 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 1U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode */

 if (opcpr_pushw(stb, 0x8400U) == 0U){ return 0U; }
 return opcpr_addr(stb, ods->op[0], VALWR_R16);
}



/* Encode JMA. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijma(symtab_t* stb, opcdec_ds_t* ods)
{
 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 1U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode */

 if (opcpr_pushw(stb, 0x8500U) == 0U){ return 0U; }
 return opcpr_addr(stb, ods->op[0], VALWR_A16);
}



/* Encode parameter list of a function. The opcode must be set up already,
** with off pointing at it. Returns nonzero (TRUE) on success. */
static auint opcpr_fnpar(symtab_t* stb, opcdec_ds_t* ods, auint off)
{
 section_t*   sec = symtab_getsectob(stb);
 auint  i;

 for (i = 0U; i < (ods->prc); i++){
  off = section_getoffw(sec);
  if (opcpr_pushw(stb, 0xC000U) == 0U){ return 0U; }
  if (opcpr_addr(stb, ods->pr[i], VALWR_A16) == 0U){ return 0U; }
 }
 section_setw(sec, off, 0x0040U); /* Terminate parameter list */
 return 1U;
}



/* Encode JFR. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijfr(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);

 /* Check count of parameters and operands */

 if (opcpr_opcount(stb, ods, 1U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode opcode */

 if (opcpr_pushw(stb, 0x4400U) == 0U){ return 0U; }
 if (opcpr_addr(stb, ods->op[0], VALWR_R16) == 0U){ return 0U; }

 /* Encode parameters */

 return opcpr_fnpar(stb, ods, off);
}



/* Encode JFA. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijfa(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);

 /* Check count of parameters and operands */

 if (opcpr_opcount(stb, ods, 1U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode opcode */

 if (opcpr_pushw(stb, 0x4500U) == 0U){ return 0U; }
 if (opcpr_addr(stb, ods->op[0], VALWR_A16) == 0U){ return 0U; }

 /* Encode parameters */

 return opcpr_fnpar(stb, ods, off);
}



/* Encode JSV. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijsv(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);

 /* Check count of parameters and operands */

 if (opcpr_opcount(stb, ods, 0U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode opcode */

 if (opcpr_pushw(stb, 0x4480U) == 0U){ return 0U; }

 /* Encode parameters */

 return opcpr_fnpar(stb, ods, off);
}



/* Encode RFN. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_irfn(symtab_t* stb, opcdec_ds_t* ods)
{
 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 0U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode */

 return opcpr_pushw(stb, 0x4580U);
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
 opcdec_ds_t ods;
 auint       r = 1U;

 if (opcdec_proc(stb, &ods) == 0){
  return PARSER_ERR;
 }

 if       ((ods.id & OPCDEC_I_R) != 0U){
  r = opcpr_ir (stb, &ods);
 }else if ((ods.id & OPCDEC_I_RB) != 0U){
  r = opcpr_irb(stb, &ods);
 }else if ((ods.id & OPCDEC_I_RS) != 0U){
  r = opcpr_irs(stb, &ods);
 }else{
  switch (ods.id){
   case OPCDEC_I_NOP: r = opcpr_inop(stb, &ods); break;
   case OPCDEC_I_MOV: r = opcpr_imov(stb, &ods); break;
   case OPCDEC_I_JMS: r = opcpr_ijms(stb, &ods); break;
   case OPCDEC_I_JMR: r = opcpr_ijmr(stb, &ods); break;
   case OPCDEC_I_JMA: r = opcpr_ijma(stb, &ods); break;
   case OPCDEC_I_JFR: r = opcpr_ijfr(stb, &ods); break;
   case OPCDEC_I_JFA: r = opcpr_ijfa(stb, &ods); break;
   case OPCDEC_I_JSV: r = opcpr_ijsv(stb, &ods); break;
   case OPCDEC_I_RFN: r = opcpr_irfn(stb, &ods); break;
   default: break;
  }
 }

 if (r){ return PARSER_END; }
 else  { return PARSER_ERR; }
}
