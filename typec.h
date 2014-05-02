/**
**  \file
**  \brief     Compilation related structures
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
*/


#ifndef TYPEC_H
#define TYPEC_H


#include "types.h"


/* Data structure for performing compilation. Collects the output, and the
** regions already populated (so overwriting can be identified). */
typedef struct{
 uint16 code[65536U];  /* Code memory */
 uint32 codu[ 2048U];  /* Code memory word usage map */
 uint16 cons[ 4096U];  /* Application header */
 uint32 conu[  128U];  /* Application header word usage map */
 uint32 datu[  128U];  /* Data memory word usage map */
}compout_t;


#endif
