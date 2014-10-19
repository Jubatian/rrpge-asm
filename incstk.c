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


#include "incstk.h"


/* Include stack object structure - definition */
struct incstk_s{
 auint pos;               /* Position in stack */
 FILE* fpt[INCSTK_MAX];   /* File pointer stack */
 auint lin[INCSTK_MAX];   /* Line pointer stack */
 uint8 fnm[INCSTK_MAX][FILE_MAX]; /* File name stack */
};



/* Note: there is a problem in C with information hiding if it would be
** desirable to work it out without dynamic memory allocation. So a static
** object is supplied here for use, and no need for malloc. One object is
** enough. So this is not a "new" implementation, just a static object. */
static incstk_t incstk_obj;
incstk_t* incstk_getobj(void)
{
 return &incstk_obj;
}



/* Inits an include stack object as empty. */
void  incstk_init(incstk_t* hnd)
{
 memset(hnd, 0U, sizeof(hnd));
}



/* Pushes a new file onto the include stack. Across includes only the line
** positions (and of cource file handle & name) are preserved, so only these
** go on the stack. Returns nonzero (TRUE) if this is not possible. Note that
** no file management is performed here. */
auint incstk_push(incstk_t* hnd, compst_t* cst, FILE* fp)
{
 if ((hnd->pos) == INCSTK_MAX){ return 1U; }
 hnd->fpt[hnd->pos] = fp;
 hnd->lin[hnd->pos] = compst_getline(cst);
 memcpy(&(hnd->fnm[hnd->pos][0U]), compst_getfile(cst), FILE_MAX);
 (hnd->pos)++;
 return 0U;
}



/* Pops a file from the include stack. Restores previous file data in the
** compilation state. Returns nonzero (TRUE) if this is not possible (since
** the stack is already empty). Note that no file management is performed
** here. Note that also resets character offset in file. */
auint incstk_pop(incstk_t* hnd, compst_t* cst, FILE** fp)
{
 if ((hnd->pos) == 0U){ return 1U; }
 (hnd->pos)--;
 *fp = hnd->fpt[hnd->pos];
 compst_setfile(cst, &(hnd->fnm[hnd->pos][0U]));
 compst_setline(cst, hnd->lin[hnd->pos]);
 compst_setcoff(cst, 0U);
 return 0U;
}
