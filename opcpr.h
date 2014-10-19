/**
**  \file
**  \brief     Opcode processing logic
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.19
*/


#ifndef OPCPR_H
#define OPCPR_H


#include "types.h"
#include "compst.h"
#include "section.h"
#include "symtab.h"


/* Attempts to process an opcode beginning at the current position in the
** source line of the compile state. It uses the offset in the current section
** to write into the passed code memory block, and increments the offset
** afterwards. While processing it also deals with any symbol or literal
** encountered, encodes them or submits them to pass2 as needed. Generates and
** outputs faults where necessary. Returns nonzero (TRUE) on any serious fault
** where the compilation should stop. */
auint opcpr_proc(symtab_t* stb);


#endif
