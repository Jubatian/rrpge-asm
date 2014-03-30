/**
**  \file
**  \brief     First pass logic of the assembler
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/



#include "pass1.h"
#include "pass2.h"
#include "pass3.h"
#include "ps1sup.h"
#include "strpr.h"
#include "litpr.h"
#include "opcpr.h"
#include "firead.h"
#include "incstk.h"



/* Number of distinct includes possible */
#define INC_MAX  256U

/* Include list for finding matching includes */
static uint8 pass1_inclist[INC_MAX][FILE_MAX];
/* Count of includes added */
static auint pass1_inccnt;



/* Attempts to find a match in the include table, returns nonzero (TRUE) if a
** match could be found. */
static auint pass1_findinc(uint8 const* src)
{
 auint i;
 for (i = 0U; i < pass1_inccnt; i++){
  if (strcmp((const char*)(src), (const char*)(&(pass1_inclist[i][0]))) == 0){
   return 1U;
  }
 }
 return 0U;
}



/* Unwinds include stack closing all files except bottommost, for fault
** handlers */
static void pass1_stkunw(incstk_t* ist, compst_t* hnd, FILE* cf)
{
 FILE* pf = cf;
 while (!incstk_pop(ist, hnd, &cf)){
  if (pf != NULL){ fclose(pf); }
  pf = cf;
 }
}



/* Executes the first pass. Uses the passed file handle for assembler source,
** processes it line by line generating code and header data (if necessary
** opening source includes as well), also filling up state for pass2 and
** pass3. Returns nonzero (TRUE) if failed (printing it's cause). */
auint pass1_run(FILE* sf, compout_t* cd, compst_t* hnd)
{
 uint8        s[80];
 uint8        ste[LINE_MAX];
 uint8 const* src;
 auint        beg;
 auint        i;
 incstk_t*    ist = incstk_getobj();
 FILE*        tf;

 incstk_init(ist);
 pass1_inccnt = 0U;

 /* Main compiling loop. Note that line 0 is already read in! */

 while (1){

  /* Check for includes and operate the include stack accordingly */

  src = compst_getsstr(hnd);
  beg = strpr_nextnw(src, 0);
  i   = 1U;              /* Marks if compile may continue for the line */

  if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("include"))){
   beg = strpr_nextnw(src, beg + 7U);
   i = strpr_extstr(&(ste[0]), &(src[beg]), LINE_MAX);
   if (i == 0){ goto fault_inc; }

   if (pass1_findinc(&(ste[0])) == 0U){ /* Not yet included */

    if (pass1_inccnt >= INC_MAX){ goto fault_imx; }
    strpr_copy(&(pass1_inclist[pass1_inccnt][0]), &(ste[0]), FILE_MAX);
    pass1_inccnt++;
    if (incstk_push(ist, hnd, sf)){ goto fault_ins; }
    if (firead_open(&(ste[0]), hnd, &sf)){ goto fault_oth; }
    beg = strpr_nextnw(src, beg + i);
    if (!strpr_isend(src[beg])){ goto fault_inc; }
    i = 1U;              /* Continue compiling with the newly read line from the include */

   }else{                /* Already included, nothing to do */
    i = 0U;              /* Don't continue compilation */
   }
  }

  /* Process the source line */

  if (i != 0U){

   i = litpr_symdefproc(hnd);
   if (i >= 2U){ compst_setcoff(hnd, i); } /* Was a label */
   if (i != 1U){         /* Further elements may follow */

    i = ps1sup_parsmisc(hnd, cd);
    if (i == 0U){ goto fault_oth; }
    if (i >= 2U){        /* Couldn't find suitable content, may other parsers try */

     i = pass3_procbindata(hnd);
     if (i == 0U){ goto fault_oth; }
     if (i >= 2U){       /* Couldn't find suitable content, may other parsers try */

      i = compst_getoffw(hnd);
      if (opcpr_proc(hnd, &(cd->code[0]))){ goto fault_oth; }
      if (ps1sup_setocc(i, hnd, cd)){ goto fault_oth; }

     }

    }

   }

  }

  /* Read next line, exit if eof reached and the include stack is clear */

  if (firead_read(hnd, sf)){ goto fault_oth; }
  if (firead_iseof(hnd, sf)){ /* File ended, try to pop include stack */
   tf = sf;
   if (incstk_pop(ist, hnd, &sf)){ break; } /* End of primary source */
   fclose(tf);           /* Close the include */
  }

 }

 /* OK, compilation over, only the primary source remained open */

 return 0U;


fault_imx:

 pass1_stkunw(ist, hnd, sf);
 compst_setcoff(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Too many includes");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;

fault_ins:

 pass1_stkunw(ist, hnd, sf);
 compst_setcoff(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Include stack size exceed");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;

fault_inc:

 pass1_stkunw(ist, hnd, sf);
 compst_setcoff(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed include");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;

fault_oth:

 pass1_stkunw(ist, hnd, sf);
 return 1U;

}
