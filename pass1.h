/**
**  \file
**  \brief     First pass logic of the assembler
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
*/


#ifndef PASS1_H
#define PASS1_H


#include "types.h"
#include "typec.h"
#include "compst.h"



/* Executes the first pass. Uses the passed file handle for assembler source,
** processes it line by line generating code and header data (if necessary
** opening source includes as well), also filling up state for pass2 and
** pass3. Returns nonzero (TRUE) if failed (printing it's cause). */
auint pass1_run(FILE* sf, compout_t* cd, compst_t* hnd);


#endif
