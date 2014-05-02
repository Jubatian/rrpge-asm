/**
**  \file
**  \brief     Third pass logic of the assembler.
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
**
**
** This processes incbins building up the complete binary.
*/


#ifndef PASS3_H
#define PASS3_H


#include "types.h"
#include "compst.h"


/* Executes the third pass. Takes the file descriptor of the output binary and
** populates it beginning from it's current seek position (so it has to be
** positioned beforehands!) page by page with the binary data acquired from
** incbins, listed in an internal structure (it processes application header
** incbins in a seperate pass after this). Returns nonzero (TRUE) if failed
** printing the reason of failure. When populating the app. header it also
** checks for collisions, using the app. header usage map. The last parameter
** is the count of binary pages, used for checking and padding the binary. */
auint pass3_run(FILE* obi, uint32 const* aphu, auint bpages);


/* Clears all pass3 state */
void  pass3_clear(void);


/* Checks current source line at current position for a valid bindata include.
** If so, it is submitted to the internal bindata list for pass3. Provides the
** following returns:
** 0: Fault, compilation should stop, fault printed.
** 1: Succesfully parsed something.
** 2: No content usable, but other parsers may try. */
auint pass3_procbindata(compst_t* hnd);


#endif
