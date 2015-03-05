/**
**  \file
**  \brief     Opcode decoding logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2015, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2015.03.05
*/


#ifndef OPCDEC_H
#define OPCDEC_H


#include "types.h"
#include "symtab.h"


/* Opcode descriptor structure */
typedef struct{
 auint id;          /* Opcode identifier */
 auint op[2];       /* Operands (destination, source) */
 auint opc;         /* Count of operands */
 auint pr[16];      /* Parameters (function calls) */
 auint prc;         /* Count of parameters */
}opcdec_ds_t;


/* Opcode identifiers */

/* Regular opcode, encoding OR mask in low 16 bits. Used for all opcodes with
** no special cases. */
#define OPCDEC_I_R     0x10000U
/* Carry destination allowed for regular opcode flag. This case the carry bit
** of the encoding OR mask must be clear (bit 14). Used for all regular
** opcodes which have carry destination forms. */
#define OPCDEC_I_RC    0x20000U
/* Regular bit opcode (set, clear, skip), encoding OR mask in low 16 bits.
** OPCDEC_I_R is not set together with this. */
#define OPCDEC_I_RB    0x40000U
/* Regular symmetric opcode: operand order indifferent (some of the skips),
** encoding OR mask in low 16 bits. */
#define OPCDEC_I_RS    0x80000U
/* Carry destination request. Indicates that the instruction contained a carry
** target ("c:"). */
#define OPCDEC_I_C     0x100000U
/* Opcode mask */
#define OPCDEC_I_MASK  0xFFFFU
/* Null: no instruction to be encoded */
#define OPCDEC_I_NUL   0U
/* NOP */
#define OPCDEC_I_NOP   1U
/* MOV: lots of special cases */
#define OPCDEC_I_MOV   2U
/* JMS */
#define OPCDEC_I_JMS   3U
/* JMR */
#define OPCDEC_I_JMR   4U
/* JMA */
#define OPCDEC_I_JMA   5U
/* JFR */
#define OPCDEC_I_JFR   6U
/* JFA */
#define OPCDEC_I_JFA   7U
/* JSV */
#define OPCDEC_I_JSV   8U
/* RFN */
#define OPCDEC_I_RFN   9U
/* XEQ */
#define OPCDEC_I_XEQ   10U
/* XNE */
#define OPCDEC_I_XNE   11U
/* XUG */
#define OPCDEC_I_XUG   12U
/* JNZ */
#define OPCDEC_I_JNZ   13U


/* Operand and parameter formatting */

/* Symbol select: If set, a symbol definition is provided on bits 0 - 23 to be
** used as an immediate for this parameter / operand. */
#define OPCDEC_O_SYM   0x40000000U
/* Address encoding right shift: Moves down the 6 bits of address encoding to
** the low end. These address encoding bits are used to provide the "adr" part
** of an opcode, with no short forms, and in long forms, without immediate
** bits (those set zero). The range of the short forms are used to indicate
** special cases instead. */
#define OPCDEC_S_ADR   24U
/* Address encoding: Special register. */
#define OPCDEC_A_SPC   0U
/* On the low 16 or 24 bits of the operand, normally the immediate or symbol
** definition (if any) is provided. If OPCDEC_A_SPC is set, however, here the
** special register is indicated by the definitions below. Note that the low
** 3 bits of the definition values must correspond the register encoding they
** use (bits 6-8 in the opcode). The 'E' parts of the values can be used to
** check for the special registers by mask. */
/* Stack Pointer */
#define OPCDEC_E_SP    0x0040U
#define OPCDEC_X_SP    (0x2U | OPCDEC_E_SP)
/* Base Pointer (loadable as BP + 0 in an "adr" field) */
#define OPCDEC_E_BP    0x0080U
#define OPCDEC_X_BP    (0x3U | OPCDEC_E_BP)
/* Pointer specials. */
#define OPCDEC_E_XM    0x0010U
#define OPCDEC_X_XM    (0x0U | OPCDEC_E_XM)
#define OPCDEC_E_XB    0x0020U
#define OPCDEC_X_XB    (0x1U | OPCDEC_E_XM)
#define OPCDEC_E_XM0   0x0100U
#define OPCDEC_X_XM0   (0x0U | OPCDEC_E_XM0)
#define OPCDEC_E_XM1   0x0200U
#define OPCDEC_X_XM1   (0x1U | OPCDEC_E_XM1)
#define OPCDEC_E_XM2   0x0400U
#define OPCDEC_X_XM2   (0x2U | OPCDEC_E_XM2)
#define OPCDEC_E_XM3   0x0800U
#define OPCDEC_X_XM3   (0x3U | OPCDEC_E_XM3)
#define OPCDEC_E_XB0   0x1000U
#define OPCDEC_X_XB0   (0x4U | OPCDEC_E_XB0)
#define OPCDEC_E_XB1   0x2000U
#define OPCDEC_X_XB1   (0x5U | OPCDEC_E_XB1)
#define OPCDEC_E_XB2   0x4000U
#define OPCDEC_X_XB2   (0x6U | OPCDEC_E_XB2)
#define OPCDEC_E_XB3   0x8000U
#define OPCDEC_X_XB3   (0x7U | OPCDEC_E_XB3)



/* Attempts to decode an opcode beginning at the current position in the
** source line of the compile state. While processing it also deals with any
** symbol or literal encountered, evaulates them or submits them to pass2 as
** needed. Generates and outputs faults where necessary. Returns nonzero
** (TRUE) on success. If the line contains no opcode, OPCDEC_I_NUL is placed
** in ods->id. */
auint opcdec_proc(symtab_t* stb, opcdec_ds_t* ods);


#endif
