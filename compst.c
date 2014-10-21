/**
**  \file
**  \brief     Compilation state
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.21
*/


#include "compst.h"
#include "strpr.h"


/* Compilation state object structure - definition */
struct compst_s{
 uint8 fil[FILE_MAX];  /* Currently parsed file */
 auint lin;            /* Line within file */
 auint chr;            /* Character within line */
 uint8 sls[LINE_MAX];  /* Source line under compilation */
 uint8 lgb[SYMB_MAX];  /* Last global label symbol */
};


/* Built-in singleton object */
static compst_t compst_obj;



/* Get built-in singleton object handle. */
compst_t* compst_getobj(void)
{
 return &compst_obj;
}



/* Inits a compilation state object */
void  compst_init(compst_t* hnd)
{
 memset(hnd, 0U, sizeof(hnd));         /* This is almost all fine for it */
 hnd->off[SECT_DATA] = (0x3000U << 1); /* Data however starts at this offset */
}



/* Copies symbol name from any source, returns number of characters the symbol
** constists. Valid symbol chars are [a-z][A-Z][0-9][.][_]. If the symbol to
** copy begins with '.', uses the last global symbol to expand it to a valid
** symbol (the return value respects this extension). dst must be capable to
** hold SYMB_MAX bytes. If hnd is NULL, no extension will happen. */
auint compst_copysym(compst_t* hnd, uint8* dst, uint8 const* src)
{
 auint i = 0U; /* Source offset */
 auint j = 0U; /* Destination offset */

 /* Check for local symbol, and prepend global if so */
 if (src[0] == '.'){ /* Local symbol */
  if (hnd != NULL){  /* There may be a global symbol stored */
   j = strpr_copy(dst, &(hnd->lgb[0]), SYMB_MAX);
  }
 }

 /* Copy source (j is at most SYMB_MAX - 1, otherwise lgb wouldn't have
 ** terminated properly).  */
 while (strpr_issym(src[i])){
  if (j == (SYMB_MAX - 1U)){ break; }
  dst[j] = src[i];
  i++;
  j++;
 }
 dst[j] = 0U;

 return j;
}



/* Check symbol equivalence. Either may be a local symbol which is expanded if
** the compile state parameter is not NULL, and either may have extra chars
** after the symbol. Returns nonzero (TRUE) if the symbols match. */
auint compst_issymequ(compst_t* hnd, uint8 const* s0, uint8 const* s1)
{
 uint8 const* sc0;
 uint8 const* sc1;
 auint i0 = 0U;
 auint i1 = 0U;

 /* Make sure s0 is the local symbol if only one local is provided */
 if (s1[0] == '.'){
  sc0 = s1;
  sc1 = s0;
 }else{
  sc0 = s0;
  sc1 = s1;
 }

 /* Test for local <=> global compare & do it if so */
 if ( (sc0[0] == '.') && (sc1[0] != '.') ){
  if (hnd == NULL) return 0U; /* Sure no match */

  /* Do local <=> global compare. sc0 is prepended with the global. */
  while (hnd->lgb[i1] != 0U){
   if (hnd->lgb[i1] != sc1[i1]){ return 0U; } /* Does not match */
   i1++;
  };
 }

 /* From this point the two either matches right or not. */
 do{
  if ( (!strpr_issym(sc0[i0])) &&
       (!strpr_issym(sc1[i1])) ){ break; } /* They match */
  if (sc0[i0] != sc1[i1]){ return 0U; }    /* Does not match */
  i0++;
  i1++;
 }while(1U);

 /* OK, on this exit path they match */
 return 1U;
}



/* Sets last global symbol. Will only have effect if the symbol provided does
** not begin with '.' (which is a local symbol), and it is an actual label
** (symbol terminating with ':'). The name is copied. */
void  compst_setgsym(compst_t* hnd, uint8 const* src)
{
 auint i = 0U;

 if (src[0] == '.'){ return; } /* Local label symbol */

 /* Check for ':' termination */
 while (strpr_issym(src[i])){ i++; }
 if (src[i] != ':'){ return; } /* Not a label specification */

 compst_copysym(NULL, &(hnd->lgb[0]), src);
}



/* Sets file name the compilation is performing from */
void  compst_setfile(compst_t* hnd, uint8 const* src)
{
 strpr_copy(&(hnd->fil[0]), src, FILE_MAX);
 hnd->lin = 0U;
 hnd->chr = 0U;
}



/* Gets file name the compilation is performing from. Persist only until
** setting a new filename. */
uint8 const* compst_getfile(compst_t* hnd)
{
 return &(hnd->fil[0]);
}



/* Sets source line the compilation is performing from. */
void  compst_setline(compst_t* hnd, auint lin)
{
 hnd->lin = lin;
 hnd->chr = 0U;
}



/* Gets source line the compilation is performing from. */
auint compst_getline(compst_t* hnd)
{
 return hnd->lin;
}



/* Sets the character the compilation is at. Returns new pointer into source
** string (which persists only until setting a new source line string). */
uint8 const* compst_setcoff(compst_t* hnd, auint cof)
{
 hnd->chr = cof;
 return &(hnd->sls[hnd->chr]);
}



/* Sets the character offset the compilation is at, relative to the previous
** offset. Returns new pointer into source string. */
uint8 const* compst_setcoffrel(compst_t* hnd, auint cof)
{
 hnd->chr += cof;
 return &(hnd->sls[hnd->chr]);
}



/* Gets the character the compilation is at. */
auint compst_getcoff(compst_t* hnd)
{
 return hnd->chr;
}



/* Sets source line string being compiled (copied in) */
void  compst_setsstr(compst_t* hnd, uint8 const* src)
{
 strpr_copy(&(hnd->sls[0]), src, LINE_MAX);
 hnd->chr = 0U;
}



/* Gets source line string being compiled. Persists only until setting a new
** source line string. */
uint8 const* compst_getsstr(compst_t* hnd)
{
 return &(hnd->sls[0]);
}



/* Gets source string from current character position */
uint8 const* compst_getsstrcoff(compst_t* hnd)
{
 return &(hnd->sls[hnd->chr]);
}
