/**
**  \file
**  \brief     Second pass logic of the assembler
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.20
*/


#ifndef PASS2_H
#define PASS2_H


#include "types.h"
#include "symtab.h"



/* Executes the second pass. Finalizes the symbol table by adding section
** bases and resolving it. Autofills the head and desc sections where
** necessary to give the appropriate application binary structure. Returns
** nonzero if failed, fault code printed. */
auint pass2_run(symtab_t* stb);


#endif
