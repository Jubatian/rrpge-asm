/**
**  \file
**  \brief     Literal processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
*/


#ifndef LITPR_H
#define LITPR_H


#include "types.h"
#include "compst.h"


/* litpr_getval() outcomes */
#define LITPR_VAL 1U
#define LITPR_UND 2U
#define LITPR_STR 4U
#define LITPR_INV 0U


/* Tries to interpret and retrieve value of a literal in the given string. The
** following outcomes are possible:
** Valid literal found, value returned (LITPR_VAL).
** Not yet defined symbol found (LITPR_UND).
** String literal found (LITPR_STR).
** String is not interpretable as literal (LITPR_INV).
** LITPR_VAL may pair with LITPR_STR. If LITPR_STR is returned alone, it
** indicates long (>4 chars) string. The interpretation stops when the line
** terminates or a seperator (,), or a function ({ or }) or an addrssing mode
** bracket (]) is found. Returns the offset of stopping (pointing at the ',',
** ']', '{' or '}' if any) in 'len' (zero for LITPR_INV), and the value of the
** literal in 'val' (only for LITPR_VAL). If LITPR_UND is returned, the symbol
** should be appropriately submitted with pass2_addsymuse(). */
auint litpr_getval(uint8 const* src, auint* len, uint32* val, compst_t* hnd);


/* Checks for symbol definition on the beginning of the current line. If a
** valid symbol definition is found (either line label or equ) it is
** submitted to pass2 as new symbol value. The current offset is used for
** this when the symbol is a label. Returns nonzero if a symbol definition was
** found and added the following way: If it is a label, returns the source
** string position after the ':' (at least 2), if it is an equ, returns 1. */
auint litpr_symdefproc(compst_t* hnd);


#endif
