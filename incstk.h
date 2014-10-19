/**
**  \file
**  \brief     Include stack
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.19
*/


#ifndef INCSTK_H
#define INCSTK_H


#include "types.h"
#include "compst.h"


/* Include stack object structure */
typedef struct incstk_s incstk_t;


/* Size of include stack. This indicates how many files may be open at once on
** include chains. Not a terribly demanding resource. */
#define INCSTK_MAX  16U



/* Note: there is a problem in C with information hiding if it would be
** desirable to work it out without dynamic memory allocation. So a static
** object is supplied here for use, and no need for malloc. One object is
** enough. So this is not a "new" implementation, just a static object. */
incstk_t* incstk_getobj(void);


/* Inits an include stack object as empty. */
void  incstk_init(incstk_t* hnd);


/* Pushes a new file onto the include stack. Across includes only the line
** positions (and of cource file handle & name) are preserved, so only these
** go on the stack. Returns nonzero (TRUE) if this is not possible. Note that
** no file management is performed here. */
auint incstk_push(incstk_t* hnd, compst_t* cst, FILE* fp);


/* Pops a file from the include stack. Restores previous file data in the
** compilation state. Returns nonzero (TRUE) if this is not possible (since
** the stack is already empty). Note that no file management is performed
** here. Note that also resets character offset in file. */
auint incstk_pop(incstk_t* hnd, compst_t* cst, FILE** fp);


#endif
