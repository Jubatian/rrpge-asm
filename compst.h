/**
**  \file
**  \brief     Compilation state
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.19
*/


#ifndef COMPST_H
#define COMPST_H


#include "types.h"


/* Compilation state object structure */
typedef struct compst_s compst_t;


/* Maximal line size including terminator. The object won't accept more. */
#define LINE_MAX  256U

/* Maximal symbol id size with terminator. The object won't accept more. */
#define SYMB_MAX   32U

/* Maximal file name with terminator. The object won't accept more. */
#define FILE_MAX   80U



/* Get built-in singleton object handle. */
compst_t* compst_getobj(void);


/* Inits a compilation state object */
void  compst_init(compst_t* hnd);


/* Copies symbol name from any source, returns number of characters the symbol
** constists. Valid symbol chars are [a-z][A-Z][0-9][.][_]. If the symbol to
** copy begins with '.', uses the last global symbol to expand it to a valid
** symbol (the return value respects this extension). dst must be capable to
** hold SYMB_MAX bytes. If hnd is NULL, no extension will happen. */
auint compst_copysym(compst_t* hnd, uint8* dst, uint8 const* src);


/* Check symbol equivalence. Either may be a local symbol which is expanded if
** the compile state parameter is not NULL, and either may have extra chars
** after the symbol. Returns nonzero (TRUE) if the symbols match. */
auint compst_issymequ(compst_t* hnd, uint8 const* s0, uint8 const* s1);


/* Sets last global symbol. Will only have effect if the symbol provided does
** not begin with '.' (which is a local symbol), and it is an actual label
** (symbol terminating with ':'). The name is copied. */
void  compst_setgsym(compst_t* hnd, uint8 const* src);


/* Sets file name the compilation is performing from (copied in) */
void  compst_setfile(compst_t* hnd, uint8 const* src);


/* Gets file name the compilation is performing from. Persist only until
** setting a new filename. */
uint8 const* compst_getfile(compst_t* hnd);


/* Sets source line the compilation is performing from. */
void  compst_setline(compst_t* hnd, auint lin);


/* Gets source line the compilation is performing from. */
auint compst_getline(compst_t* hnd);


/* Sets the character the compilation is at. Returns new pointer into source
** string (which persists only until setting a new source line string). */
uint8 const* compst_setcoff(compst_t* hnd, auint cof);


/* Sets the character offset the compilation is at, relative to the previous
** offset. Returns new pointer into source string. */
uint8 const* compst_setcoffrel(compst_t* hnd, auint cof);


/* Gets the character the compilation is at. */
auint compst_getcoff(compst_t* hnd);


/* Sets source line string being compiled (copied in) */
void  compst_setsstr(compst_t* hnd, uint8 const* src);


/* Gets source line string being compiled. Persists only until setting a new
** source line string. */
uint8 const* compst_getsstr(compst_t* hnd);


/* Gets source string from current character position */
uint8 const* compst_getsstrcoff(compst_t* hnd);


#endif
