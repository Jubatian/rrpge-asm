/**
**  \file
**  \brief     Opcode processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.03.13
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



/* MOV rx, imx table for 0000 011r rrpq iiii */
static const uint16 opcpr_mov_tb0[64] = {
 0x0280U, 0xFF0FU, 0xF0FFU, 0x0180U, 0x0300U, 0x01C0U, 0x0F00U, 0x0118U,
 0x0140U, 0x0168U, 0x0190U, 0x01B8U, 0x01E0U, 0x0208U, 0x0230U, 0x0258U,
 0x0010U, 0x0011U, 0x0012U, 0x0013U, 0x0014U, 0x0015U, 0x0016U, 0x0017U,
 0x0018U, 0x0019U, 0x001AU, 0x001BU, 0x001CU, 0x001DU, 0x001EU, 0x001FU,
 0x0020U, 0x0021U, 0x0022U, 0x0023U, 0x0024U, 0x0025U, 0x0026U, 0x0027U,
 0x0028U, 0x0029U, 0x002AU, 0x002BU, 0x002CU, 0x002DU, 0x002EU, 0x002FU,
 0x0030U, 0x0031U, 0x0032U, 0x0033U, 0x0034U, 0x0035U, 0x0036U, 0x0037U,
 0x0038U, 0x0039U, 0x003AU, 0x003BU, 0x003CU, 0x003DU, 0x003EU, 0x003FU};
/* MOV rx, imx table for 0100 011r rrpq iiii */
static const uint16 opcpr_mov_tb1[64] = {
 0x0040U, 0x0041U, 0x0042U, 0x0043U, 0x0044U, 0x0045U, 0x0046U, 0x0047U,
 0x0048U, 0x0049U, 0x004AU, 0x004BU, 0x004CU, 0x004DU, 0x004EU, 0x004FU,
 0x0050U, 0x0051U, 0x0052U, 0x0053U, 0x0054U, 0x0055U, 0x0056U, 0x0057U,
 0x0058U, 0x0059U, 0x005AU, 0x005BU, 0x005CU, 0x005DU, 0x005EU, 0x005FU,
 0x0060U, 0x0061U, 0x0062U, 0x0063U, 0x0064U, 0x0065U, 0x0066U, 0x0067U,
 0x0068U, 0x0069U, 0x006AU, 0x006BU, 0x006CU, 0x006DU, 0x006EU, 0x006FU,
 0x0070U, 0x0071U, 0x0072U, 0x0073U, 0x0074U, 0x0075U, 0x0076U, 0x0077U,
 0x0078U, 0x0079U, 0x007AU, 0x007BU, 0x007CU, 0x007DU, 0x007EU, 0x007FU};
/* MOV rx, imx table for 1100 011r rrpq iiii */
static const uint16 opcpr_mov_tb2[64] = {
 0x0080U, 0x0088U, 0x0090U, 0x0098U, 0x0010U, 0x0020U, 0x0040U, 0x0080U,
 0x0100U, 0x0200U, 0x0400U, 0x0800U, 0x1000U, 0x2000U, 0x4000U, 0x8000U,
 0x00A0U, 0x00A8U, 0x00B0U, 0x00B8U, 0xFFEFU, 0xFFDFU, 0xFFBFU, 0xFF7FU,
 0xFEFFU, 0xFDFFU, 0xFBFFU, 0xF7FFU, 0xEFFFU, 0xDFFFU, 0xBFFFU, 0x7FFFU,
 0x00C0U, 0x00C8U, 0x00D0U, 0x00D8U, 0xFFE0U, 0xFFC0U, 0xFF80U, 0xFF00U,
 0xFE00U, 0xFC00U, 0xF800U, 0xF000U, 0xE000U, 0xC000U, 0x8000U, 0x0000U,
 0x00E0U, 0x00E8U, 0x00F0U, 0x00F8U, 0x001FU, 0x003FU, 0x007FU, 0x00FFU,
 0x01FFU, 0x03FFU, 0x07FFU, 0x0FFFU, 0x1FFFU, 0x3FFFU, 0x7FFFU, 0xFFFFU};

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
 auint  i;

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

   for (i = 0U; i < 64U; i++){     /* MOV rx, imx (0000) */
    if (v == opcpr_mov_tb0[i]){
     section_setw(sec, off, 0x0600U | (((reg >> OPCDEC_S_ADR) & 0x7U) << 6) | i);
     return 1U;
    }
   }

   for (i = 0U; i < 64U; i++){     /* MOV rx, imx (0100) */
    if (v == opcpr_mov_tb1[i]){
     section_setw(sec, off, 0x4600U | (((reg >> OPCDEC_S_ADR) & 0x7U) << 6) | i);
     return 1U;
    }
   }

   for (i = 0U; i < 64U; i++){     /* MOV rx, imx (1100) */
    if (v == opcpr_mov_tb2[i]){
     section_setw(sec, off, 0x8600U | (((reg >> OPCDEC_S_ADR) & 0x7U) << 6) | i);
     return 1U;
    }
   }

  }

  /* Encode ordinary MOV */

  section_setw(sec, off, 0x0000U | (((reg >> OPCDEC_S_ADR) & 0x7U) << 6));
  return opcpr_addr(stb, adr, VALWR_A16);
 }

 /* Special register MOVs */

 if ((reg & (OPCDEC_E_SP)) != 0U){
  if ( (((adr >> OPCDEC_S_ADR) & 0x38U) == 0x20U) &&   /* Immediate: specials here */
       ((adr & OPCDEC_O_SYM) == 0U) ){                 /* Only if not symbol (so value present) */
   v = adr & 0xFFFFU;
   if (v < 128U){                  /* MOV SP, imm7 can be encoded */
    section_setw(sec, off, 0x8380U | v);
    return 1U;
   }
  }
 }

 if ((reg & (OPCDEC_E_SP | OPCDEC_E_XM | OPCDEC_E_XB)) != 0U){
  section_setw(sec, off, 0x8000U | ((reg & 0x7U) << 6));
  return opcpr_addr(stb, adr, VALWR_A16);
 }

 if ((reg & (OPCDEC_E_XM0 | OPCDEC_E_XM1 | OPCDEC_E_XM2 | OPCDEC_E_XM3 |
             OPCDEC_E_XB0 | OPCDEC_E_XB1 | OPCDEC_E_XB2 | OPCDEC_E_XB3)) != 0U){
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



/* Encode JNZ. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijnz(symtab_t* stb, opcdec_ds_t* ods)
{
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 auint  off = section_getoffw(sec);
 auint  op0 = (ods->op[0]);
 auint  op1 = (ods->op[1]);

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 2U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Check operand 0: must be a register */

 if (((op0 >> OPCDEC_S_ADR) & 0x38U) != 0x30U){
  fault_printat(FAULT_FAIL, (uint8 const*)("First operand must be register"), cst);
  return 0U;
 }

 /* Check operand 1: must be an immediate */

 if (((op1 >> OPCDEC_S_ADR) & 0x38U) != 0x20U){
  fault_printat(FAULT_FAIL, (uint8 const*)("Second operand must be immediate"), cst);
  return 0U;
 }

 /* Encode */

 if (opcpr_pushw(stb, 0x8800U | (((op0 >> OPCDEC_S_ADR) & 0x7U) << 6)) == 0U){ return 0U; }
 if ((op1 & OPCDEC_O_SYM) != 0U){
  if (symtab_use(stb, op1 & 0xFFFFFFU, off, VALWR_R7)){ return 0U; }
 }else{
  if (valwr_writecs(sec, op1 & 0xFFFFU, off, VALWR_R7, cst)){ return 0U; }
 }
 return 1U;
}



/* JMR / JMA common encoder */
static auint opcpr_ijmp(symtab_t* stb, opcdec_ds_t* ods, auint msk, auint use)
{
 compst_t*    cst = symtab_getcompst(stb);
 auint rt;

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* If it has 1 parameter, simply encode */

 if (ods->opc == 1U){
  if (opcpr_pushw(stb, msk) == 0U){ return 0U; }
  return opcpr_addr(stb, ods->op[0], use);
 }

 /* Must have 2 operands, and first one must be B, C or D */

 if (opcpr_opcount(stb, ods, 2U) == 0U){ return 0U; }
 rt = (ods->op[0] >> OPCDEC_S_ADR) & 0x3FU;
 if ((rt != 0x31U) && (rt != 0x32U) && (rt != 0x33U)){
  fault_printat(FAULT_FAIL, (uint8 const*)("Target must be B, C or D"), cst);
  return 0U;
 }

 /* Encode */

 if (opcpr_pushw(stb, msk | ((rt & 0x3U) << 6)) == 0U){ return 0U; }
 return opcpr_addr(stb, ods->op[1], use);
}



/* Encode JMR. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijmr(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ijmp(stb, ods, 0x8400U, VALWR_R16);
}



/* Encode JMA. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijma(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ijmp(stb, ods, 0x8500U, VALWR_A16);
}



/* Encode parameter list of a function. The opcode must be set up already,
** with off pointing at it. Returns nonzero (TRUE) on success. */
static auint opcpr_fnpar(symtab_t* stb, opcdec_ds_t* ods, auint off)
{
 section_t*   sec = symtab_getsectob(stb);
 auint  i;
 auint  v;

 for (i = 0U; i < (ods->prc); i++){
  off = section_getoffw(sec);
  if (opcpr_pushw(stb, 0xC000U) == 0U){ return 0U; }

  if ( (((ods->pr[i] >> OPCDEC_S_ADR) & 0x38U) == 0x20U) && /* Immediate: specials here */
       ((ods->pr[i] & OPCDEC_O_SYM) == 0U) ){               /* Only if not symbol (so value present) */

   v = ods->pr[i] & 0xFFFFU;           /* Value to encode */

   if       (v <  0x1000U){            /* Encode --1j jjjj jeii iiii */
    section_setw(sec, off, 0x2000U | ((v & 0x0FC0U) << 1) | (v & 0x003FU));
   }else if (v >= 0xFC00U){            /* Encode --00 1jjj jeii iiii */
    section_setw(sec, off, 0x0800U | ((v & 0x03C0U) << 1) | (v & 0x003FU));
   }else if ((v & 0x3FU) == 0x3FU){    /* Encode --01 1jjj jeii iiii */
    section_setw(sec, off, 0x1800U | ((v & 0xF000U) >> 5) | ((v & 0x0FC0U) >> 6));
   }else if ((v & 0x3FU) == 0x00U){    /* Encode --01 0jjj jeii iiii */
    section_setw(sec, off, 0x1000U | ((v & 0xF000U) >> 5) | ((v & 0x0FC0U) >> 6));
   }else if ((v & 0xFFU) == (v >> 8)){ /* Encode --00 01-j jeii iiii */
    section_setw(sec, off, 0x0400U | ((v & 0x00C0U) << 1) | (v & 0x003FU));
   }else{
    if (opcpr_addr(stb, ods->pr[i], VALWR_A16) == 0U){ return 0U; }
   }

  }else{
   if (opcpr_addr(stb, ods->pr[i], VALWR_A16) == 0U){ return 0U; }
  }
 }
 section_setw(sec, off, 0x0040U);      /* Terminate parameter list */
 return 1U;
}



/* JFR / JFA common encoder */
static auint opcpr_ijfn(symtab_t* stb, opcdec_ds_t* ods, auint msk, auint use)
{
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);

 /* Check count of parameters and operands */

 if (opcpr_opcount(stb, ods, 1U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode opcode */

 if (opcpr_pushw(stb, msk) == 0U){ return 0U; }
 if (opcpr_addr(stb, ods->op[0], use) == 0U){ return 0U; }

 /* Encode parameters */

 return opcpr_fnpar(stb, ods, off);
}



/* Encode JFR. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijfr(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ijfn(stb, ods, 0x4400U, VALWR_R16);
}



/* Encode JFA. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijfa(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ijfn(stb, ods, 0x4500U, VALWR_A16);
}



/* Encode JSV. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ijsv(symtab_t* stb, opcdec_ds_t* ods)
{
 compst_t*    cst = symtab_getcompst(stb);
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);
 auint  opv;

 /* Check count of parameters and operands */

 if (opcpr_opcount(stb, ods, 1U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Encode opcode */

 if (opcpr_pushw(stb, 0x4480U) == 0U){ return 0U; }
 opv = ods->op[0];
 if ((opv & OPCDEC_O_SYM) != 0U){
  if (symtab_use(stb, opv & 0xFFFFFFU, off, VALWR_S6)){ return 0U; }
 }else if (((opv >> OPCDEC_S_ADR) & 0x3FU) == 0x20U){
  if (valwr_writecs(sec, opv & 0xFFFFU, off, VALWR_S6, cst)){ return 0U; }
 }else{
  fault_printat(FAULT_FAIL, (const uint8*)("Invalid operand for JSV"), cst);
  return 0U;
 }

 /* Encode parameters */

 return opcpr_fnpar(stb, ods, off);
}



/* Encode RFN. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_irfn(symtab_t* stb, opcdec_ds_t* ods)
{
 compst_t*    cst = symtab_getcompst(stb);
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }

 /* If it has no operand, then encode RFN x3, x3 */

 if (ods->opc == 0U){ return opcpr_pushw(stb, 0x45B7U); }

 /* Otherwise needs 2 operands, first being x3 */

 if (opcpr_opcount(stb, ods, 2U) == 0U){ return 0U; }
 if (((ods->op[0] >> OPCDEC_S_ADR) & 0x3FU) != 0x37U){
  fault_printat(FAULT_FAIL, (uint8 const*)("Target must be X3"), cst);
  return 0U;
 }

 /* Encode */

 if (opcpr_pushw(stb, 0x4580U) == 0U){ return 0U; }
 if (((ods->id) & OPCDEC_I_C) != 0U){
  section_setw(sec, off, 0x0040U);
 }
 return opcpr_addr(stb, ods->op[1], VALWR_A16);
}



/* Skip common encoder for SP specials: msk0 is the normal opcode mask, msk1
** is the SP special mask, and swp indicates whether operands should be
** swapped. */
static auint opcpr_ixxx(symtab_t* stb, opcdec_ds_t* ods, auint msk0, auint msk1, auint swp)
{
 compst_t*    cst = symtab_getcompst(stb);
 section_t*   sec = symtab_getsectob(stb);
 auint  off = section_getoffw(sec);
 auint  reg;
 auint  adr;

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_opcount(stb, ods, 2U) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 reg = ods->op[1];
 adr = ods->op[0];

 /* Check for SP special */

 if ( (((reg >> OPCDEC_S_ADR) & 0x3FU) != OPCDEC_A_SPC) &&
      (((adr >> OPCDEC_S_ADR) & 0x3FU) != OPCDEC_A_SPC) ){ /* No special reg. */

  /* No special: Encode ordinary skip */

  if (opcpr_pushw(stb, msk0) == 0U){ return 0U; }
  return opcpr_eops(stb, ods, swp);

 }

 /* Pre-encode opcode */

 if (opcpr_pushw(stb, msk1) == 0U){ return 0U; }

 /* Select between the two possible orders */

 if ((reg >> OPCDEC_S_ADR) != OPCDEC_A_SPC){
  if (swp){ section_setw(sec, off, 0x0200U); }  /* "rx, adr" order */
  reg = ods->op[0];
  adr = ods->op[1];
  if ((reg & (OPCDEC_E_SP)) == 0U){
   fault_printat(FAULT_FAIL, (uint8 const*)("Special register operand must be SP"), cst);
   return 0U;
  }
 }

 /* Add address operand */

 return opcpr_addr(stb, adr, VALWR_A16);
}



/* Encode XEQ. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ixeq(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ixxx(stb, ods, 0xB800U, 0x8140U, 0U);
}



/* Encode XNE. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ixne(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ixxx(stb, ods, 0xBA00U, 0x8340U, 0U);
}



/* Encode XUG. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ixug(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ixxx(stb, ods, 0xBC00U, 0x8100U, 1U);
}



/* Common encoder for push and pop: msk is the opcode mask (the low 6 bits
** have to be clear so the registers can be added). */
static auint opcpr_ipp(symtab_t* stb, opcdec_ds_t* ods, auint msk)
{
 compst_t*    cst = symtab_getcompst(stb);
 auint  reg;
 auint  i;
 auint  t;

 /* Check count of parameters and operands */

 if (opcpr_nofunc(stb, ods) == 0U){ return 0U; }
 if (opcpr_nocy(stb, ods) == 0U){ return 0U; }

 /* Collect parameter registers */

 reg = 0U;

 if (ods->opc == 0U){
  fault_printat(FAULT_FAIL, (uint8 const*)("Needs at least one register parameter"), cst);
  return 0U;
 }

 for (i = 0U; i < ods->opc; i++){
  t = (ods->op[i] >> OPCDEC_S_ADR) & 0x3FU;
  if ((t & 0x38U) == 0x30U){    /* Normal register */
   switch (t & 0x7U){
    case 0U: reg |= 0x20U; break;
    case 1U: reg |= 0x10U; break;
    case 3U: reg |= 0x04U; break;
    case 4U: reg |= 0x02U; break;
    case 5U: reg |= 0x01U; break;
    case 6U: reg |= 0x08U; break;
    default:
     fault_printat(FAULT_FAIL, (uint8 const*)("Only registers A, B, D, X0, X1, X2, XM and XB can be used"), cst);
     return 0U;
   }
  }else if (ods->op[i] == OPCDEC_X_XM){
   reg |= 0x40U;
  }else if (ods->op[i] == OPCDEC_X_XB){
   reg |= 0x80U;
  }else{
   fault_printat(FAULT_FAIL, (uint8 const*)("Only registers A, B, D, X0, X1, X2, XM and XB can be used"), cst);
   return 0U;
  }
 }

 /* Check combinations */

 if ((reg & 0xC0U) != 0U){
  if (reg != 0xFFU){
   fault_printat(FAULT_FAIL, (uint8 const*)("XM and XB must be used in an all register operation"), cst);
   return 0U;
  }
  reg = 0U;
 }

 /* Write instruction */

 return opcpr_pushw(stb, msk | reg);
}



/* Encode PSH. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ipsh(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ipp(stb, ods, 0x80C0U);
}



/* Encode POP. May produce fault. Returns nonzero (TRUE) on success. */
static auint opcpr_ipop(symtab_t* stb, opcdec_ds_t* ods)
{
 return opcpr_ipp(stb, ods, 0x82C0U);
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
  switch (ods.id & OPCDEC_I_MASK){
   case OPCDEC_I_NOP: r = opcpr_inop(stb, &ods); break;
   case OPCDEC_I_MOV: r = opcpr_imov(stb, &ods); break;
   case OPCDEC_I_JMS: r = opcpr_ijms(stb, &ods); break;
   case OPCDEC_I_JMR: r = opcpr_ijmr(stb, &ods); break;
   case OPCDEC_I_JMA: r = opcpr_ijma(stb, &ods); break;
   case OPCDEC_I_JFR: r = opcpr_ijfr(stb, &ods); break;
   case OPCDEC_I_JFA: r = opcpr_ijfa(stb, &ods); break;
   case OPCDEC_I_JSV: r = opcpr_ijsv(stb, &ods); break;
   case OPCDEC_I_RFN: r = opcpr_irfn(stb, &ods); break;
   case OPCDEC_I_XEQ: r = opcpr_ixeq(stb, &ods); break;
   case OPCDEC_I_XNE: r = opcpr_ixne(stb, &ods); break;
   case OPCDEC_I_XUG: r = opcpr_ixug(stb, &ods); break;
   case OPCDEC_I_JNZ: r = opcpr_ijnz(stb, &ods); break;
   case OPCDEC_I_PSH: r = opcpr_ipsh(stb, &ods); break;
   case OPCDEC_I_POP: r = opcpr_ipop(stb, &ods); break;
   default: break;
  }
 }

 if (r){ return PARSER_END; }
 else  { return PARSER_ERR; }
}
