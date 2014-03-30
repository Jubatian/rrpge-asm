/**
**  \file
**  \brief     Support routines for Pass 1
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/


#ifndef PS1SUP_H
#define PS1SUP_H


#include "types.h"
#include "typec.h"
#include "compst.h"



/* Pushes occpuation data and checks for overlap. The previous offset is
** supplied, the current is grabbed from the compile state. Returns nonzero
** (TRUE) if an overlap is detected while reporting an appropriate fault. */
auint ps1sup_setocc(auint pof, compst_t* hnd, compout_t* cd);


/* Attempts to process the source line as one of the followings:
** 'ds', 'db' or 'dw'.
** 'org'.
** 'section'.
** In case of 'db' or 'dw', it outputs to the appropriate memory (code ROM or
** app. header). Provides the following returns:
** 0: Fault, compilation should stop, fault printed.
** 1: Succesfully parsed something.
** 2: No content usable, but other parsers may try.
** In the case of data allocations, also checks and reports overlaps. Note
** that it starts parsing the line at the last set char. position, so this way
** labels may be skipped (processed earlier using litpr_symdefproc()). */
auint ps1sup_parsmisc(compst_t* hnd, compout_t* cd);


#endif
