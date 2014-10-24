/**
**  \file
**  \brief     Support routines for Pass 1
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.24
*/


#ifndef PS1SUP_H
#define PS1SUP_H


#include "types.h"
#include "compst.h"
#include "section.h"
#include "symtab.h"


/* Attempts to process the source line as one of the followings:
** 'ds', 'db' and 'dw'.
** 'org'.
** 'section'.
** 'AppAuth', 'AppName', 'Version', 'EngSpec' and 'License'.
** Returns one of the defined PARSER return codes (defined in types.h). Note
** that it starts parsing the line at the last set char. position, so this way
** labels may be skipped (processed earlier using litpr_symdefproc()). */
auint ps1sup_parsmisc(symtab_t* stb);


#endif
