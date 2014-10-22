/**
**  \file
**  \brief     Binary data processor
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.22
**
**  Manages the processing of "bindata" directives, creating a list of binary
**  includes, to be evaulated in pass3. This also includes performing the
**  necessary operations on the binary include files. Currently singleton, but
**  designed so it is possible to extend later.
*/


#ifndef BINDATA_H
#define BINDATA_H


#include "types.h"
#include "symtab.h"



/* Bindata manager object structure */
typedef struct bindata_s bindata_t;


/* Maximal number of bindata (in FILE section) definitions */
#define BINDATA_MAX 4096U



/* Get built-in singleton object handle. */
bindata_t* bindata_getobj(void);


/* Initializes or resets a bindata object. */
void  bindata_init(bindata_t* hnd);


/* Checks the current source line at the current position for a valid bindata
** include. Then, depending on the section, either processes it directly into
** the selected section or if it is the FILE section, puts it on the bindata
** manager's list for later processing (in pass3). Returns one of the defined
** PARSER return codes (defined in types.h). */
auint bindata_proc(bindata_t* hnd, symtab_t* stb);


/* Processes bindata table, and produces the according output in the passed
** file. The output is sequential, no seeking is performed on the target file.
** Returns nonzero on failure, fault code printed. */
auint bindata_out(bindata_t* hnd, FILE* ofl);


#endif
