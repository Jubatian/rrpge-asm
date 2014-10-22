/**
**  \file
**  \brief     Third pass logic of the assembler.
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.22
*/


#ifndef PASS3_H
#define PASS3_H


#include "types.h"
#include "bindata.h"
#include "symtab.h"


/* Executes the third pass. This combines the application components prepared
** in pass 2 with the FILE section binary data blocks into a new application
** binary file. Returns nonzero on failure. */
auint pass3_run(FILE* obi, symtab_t* stb, bindata_t* bdt);


#endif
