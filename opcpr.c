/**
**  \file
**  \brief     Opcode processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.02.27
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
 0x0000U, 0x0010U, 0x0020U, 0x0030U, 0x0040U, 0x0050U, 0x0060U, 0x0070U,
 0x0080U, 0x0090U, 0x00A0U, 0x00B0U, 0x00C0U, 0x00D0U, 0x00E0U, 0x00F0U,
 0x000FU, 0x001FU, 0x002FU, 0x003FU, 0x004FU, 0x005FU, 0x006FU, 0x007FU,
 0x008FU, 0x009FU, 0x00AFU, 0x00BFU, 0x00CFU, 0x00DFU, 0x00EFU, 0x00FFU,
 0xFF00U, 0xFF10U, 0xFF20U, 0xFF30U, 0xFF40U, 0xFF50U, 0xFF60U, 0xFF70U,
 0xFF80U, 0xFF90U, 0xFFA0U, 0xFFB0U, 0xFFC0U, 0xFFD0U, 0xFFE0U, 0xFFF0U,
 0xFF0FU, 0xFF1FU, 0xFF2FU, 0xFF3FU, 0xFF4FU, 0xFF5FU, 0xFF6FU, 0xFF7FU,
 0xFF8FU, 0xFF9FU, 0xFFAFU, 0xFFBFU, 0xFFCFU, 0xFFDFU, 0xFFEFU, 0xFFFFU};
/* MOV rx, imx table for 0100 011r rrpq iiii */
static const uint16 opcpr_mov_tb1[64] = {
 0x0000U, 0x0100U, 0x0200U, 0x0300U, 0x0400U, 0x0500U, 0x0600U, 0x0700U,
 0x0800U, 0x0900U, 0x0A00U, 0x0B00U, 0x0C00U, 0x0D00U, 0x0E00U, 0x0F00U,
 0x00FFU, 0x01FFU, 0x02FFU, 0x03FFU, 0x04FFU, 0x05FFU, 0x06FFU, 0x07FFU,
 0x08FFU, 0x09FFU, 0x0AFFU, 0x0BFFU, 0x0CFFU, 0x0DFFU, 0x0EFFU, 0x0FFFU,
 0xF000U, 0xF100U, 0xF200U, 0xF300U, 0xF400U, 0xF500U, 0xF600U, 0xF700U,
 0xF800U, 0xF900U, 0xFA00U, 0xFB00U, 0xFC00U, 0xFD00U, 0xFE00U, 0xFF00U,
 0xF0FFU, 0xF1FFU, 0xF2FFU, 0xF3FFU, 0xF4FFU, 0xF5FFU, 0xF6FFU, 0xF7FFU,
 0xF8FFU, 0xF9FFU, 0xFAFFU, 0xFBFFU, 0xFCFFU, 0xFDFFU, 0xFEFFU, 0xFFFFU};
/* MOV rx, imx table for 1100 011r rrpq iiii */
static const uint16 opcpr_mov_tb2[64] = {
 0x0000U, 0x1000U, 0x2000U, 0x3000U, 0x4000U, 0x5000U, 0x6000U, 0x7000U,
 0x8000U, 0x9000U, 0xA000U, 0xB000U, 0xC000U, 0xD000U, 0xE000U, 0xF000U,
 0x0FFFU, 0x1FFFU, 0x2FFFU, 0x3FFFU, 0x4FFFU, 0x5FFFU, 0x6FFFU, 0x7FFFU,
 0x8FFFU, 0x9FFFU, 0xAFFFU, 0xBFFFU, 0xCFFFU, 0xDFFFU, 0xEFFFU, 0xFFFFU,
 0x0010U, 0x0011U, 0x0012U, 0x0013U, 0x0014U, 0x0015U, 0x0016U, 0x0017U,
 0x0018U, 0x0019U, 0x001AU, 0x001BU, 0x001CU, 0x001DU, 0x001EU, 0x001FU,
 0x0280U, 0x0028U, 0x0064U, 0x0078U, 0x03E8U, 0x00C8U, 0x2710U, 0x0118U,
 0x0140U, 0x0168U, 0x0190U, 0x01B8U, 0x01E0U, 0x0208U, 0x0230U, 0x0258U};

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
 return opcpr_addr(stb, ods->op[0], use);
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
   default: break;
  }
 }

 if (r){ return PARSER_END; }
 else  { return PARSER_ERR; }
}
