/**
**  \file
**  \brief     Second pass logic of the assembler
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.19
*/


#ifndef PASS2_H
#define PASS2_H


#include "types.h"
#include "valwr.h"
#include "compst.h"



/* Executes the second pass. Takes the application header & code ROM prepared
** by the first pass, then using the internally generated symbol table it
** attempts to substitue symbols so a complete header & code ROM is produced
** and returned. Returns nonzero (TRUE) if failed (prints the reason of
** failure on the console). The app. header is 4KWords, the code ROM is
** 64KWords. */
auint pass2_run(uint16* apph, uint16* crom);


/* Clears all pass2 state (this is the symbol table). Initially the state is
** cleared. */
void  pass2_clear(void);


/* Adds a symbol value (definition) to the symbol table. Prints a fault if it
** is not possible for the symbol table being full. The symbol name can be
** discarded after addition. The name can be provided as text pointer into
** source fragment. */
void  pass2_addsymval(uint8 const* nam, uint32 val, compst_t* hnd);


/* Retrieves symbol value if any. Returns nonzero (TRUE) if the symbol is
** defined, and fills val in, zero (FALSE) otherwise, setting val zero. The
** name can be provided as text pointer into source fragment. */
auint pass2_getsymval(uint8 const* nam, uint32* val, compst_t* hnd);


/* Adds a symbol usage to the symbol table. The off and use parameters are
** formed according to valwr_write(). The name can be provided as text pointer
** into source fragment. */
void  pass2_addsymuse(uint8 const* nam, auint off, auint use, compst_t* hnd);


#endif
