/**
**  \file
**  \brief     Literal processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.21
*/


#ifndef LITPR_H
#define LITPR_H


#include "types.h"
#include "compst.h"
#include "symtab.h"
#include "section.h"


/* litpr_getval() outcomes */
#define LITPR_VAL 1U
#define LITPR_UND 2U
#define LITPR_STR 4U
#define LITPR_INV 0U


/* Tries to interpret and retrieve value of a literal in the given string. The
** following outcomes are possible:
**
** LITPR_VAL: Valid literal found, it's value is returned in 'val'.
** LITPR_UND: An aggregate containing symbols found. The aggregate's members
**            are submitted appropriately to the symbol table, in 'val' the
**            top member's ID is returned (to be used with symtab_use()).
** LITPR_STR: String literal is found. If LITPR_VAL is set along with it, the
**            value of the literal is returned in 'val', otherwise not (the
**            string is longer than 4 ASCII characters).
** LITPR_INV: Error. An appropriate fault code is printed.
**
** The interpretation stops when the line terminates or a seperator (,), or a
** function ({ or }) or an addrssing mode bracket (]) is found. Returns the
** offset of stopping (pointing at the ',', ']', '{' or '}' if any) in 'len'
** (zero for LITPR_INV). */
auint litpr_getval(uint8 const* src, auint* len, auint* val, symtab_t* stb);


/* Checks for symbol definition on the beginning of the current line. If a
** valid symbol definition is found (either line label or equ) it is
** submitted to the symbol table as new symbol value. Returns one of the
** defined PARSER return codes (defined in types.h). */
auint litpr_symdefproc(symtab_t* stb);


#endif
