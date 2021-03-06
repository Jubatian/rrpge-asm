/**
**  \file
**  \brief     First pass logic of the assembler
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.22
*/


#ifndef PASS1_H
#define PASS1_H


#include "types.h"
#include "symtab.h"
#include "bindata.h"



/* Executes the first pass. Uses the passed file handle for assembler source,
** processes it line by line generating code and header data (if necessary
** opening source includes as well), also filling up state for pass2 and
** pass3. Returns nonzero (TRUE) if failed (printing it's cause). */
auint pass1_run(FILE* sf, symtab_t* stb, bindata_t* bdt);


#endif
