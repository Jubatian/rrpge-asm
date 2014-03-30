/**
**  \file
**  \brief     Second pass logic of the assembler
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv1 (version 1 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv1 in the project root.
**  \date      2014.03.23
*/


#include "pass2.h"
#include "strpr.h"



/* Maximal number of symbol table entries */
#define SYMTABLE_SIZE  4096U
/* Maximal number of usage entries */
#define SYMUSE_SIZE   16384U



/* Symbol table usage entry */
typedef struct pass2_symu_s{
 auint  off;                 /* Offset of first word where it occurs */
 auint  use;                 /* Usage type */
 uint8  fil[FILE_MAX];       /* File name for fault */
 fault_off_t   fof;          /* Location of definition for fault */
 struct pass2_symu_s* nxt;
}pass2_symu_t;


/* Symbol table header */
typedef struct{
 uint8  nam[SYMB_MAX];       /* Symbol name, padded with zeros */
 uint32 val;                 /* Value of symbol */
 auint  def;                 /* Nonzero (TRUE) if the symbol is defined (value is valid) */
 uint8  fil[FILE_MAX];       /* File name for fault */
 fault_off_t   fof;          /* Location of definition for fault */
 pass2_symu_t* use;          /* Usage list */
}pass2_symt_t;



static pass2_symt_t pass2_symt[SYMTABLE_SIZE]; /* Symbol table headers */
static pass2_symu_t pass2_symu[SYMUSE_SIZE];   /* Symbol table usages */
static auint        pass2_tcnt = 0U;           /* Number of symbols used */
static auint        pass2_ucnt = 0U;           /* Number of usage table entries consumed */



/* Finds a symbol in the symbol table. Returns the offset, or the current
** size of the symbol table (pass2_tcnt) if not found. The compile state may
** be NULL to ignore global extension. */
static auint pass2_snfind(uint8 const* nam, compst_t* hnd)
{
 auint i;
 for (i = 0U; i < pass2_tcnt; i++){
  if (compst_issymequ(hnd, nam, &(pass2_symt[i].nam[0]))){ break; }
 }
 return i;
}



/* Executes the second pass. Takes the application header & code ROM prepared
** by the first pass, then using the internally generated symbol table it
** attempts to substitue symbols so a complete header & code ROM is produced
** and returned. Returns nonzero (TRUE) if failed (prints the reason of
** failure on the console). The app. header is 4KWords, the code ROM is
** 64KWords. */
auint pass2_run(uint16* apph, uint16* crom)
{
 uint8   s[80];
 auint   i;
 uint32  v;
 uint16 *p;
 pass2_symu_t* u;

 /* Go through all symbols */
 for (i = 0; i < pass2_tcnt; i++){

  /* Check if symbol is defined, and produce a warning if not */
  if ((pass2_symt[i].def == 0U) && (pass2_symt[i].use != NULL)){
   snprintf((char*)(&s[0]), 80U, "Symbol %s is used, but not defined", (char const*)(&(pass2_symt[i].nam[0])));
   fault_print(FAULT_WARN, &s[0], &(pass2_symt[i].fof));
  }

  /* Substitue all it's occurences */
  v = pass2_symt[i].val;
  u = pass2_symt[i].use;
  while (u != NULL){

   if (((u->off) & VALWR_APPH) != 0){ p = apph; }     /* App header */
   else                             { p = crom; }     /* Code */
   if (valwr_write(p, v, u->off, u->use, &(u->fof))){ /* Failed? */
    return 1U;                                        /* If so, exit, can't compile */
   }

   u = u->nxt;
  }

 }

 /* Everything could be compiled in, so OK */
 return 0U;
}



/* Clears all pass2 state (this is the symbol table). Initially the state is
** cleared. */
void  pass2_clear(void)
{
 pass2_tcnt = 0U;
 pass2_ucnt = 0U;
}



/* Creates new symbol at the end of the symbol table if possible. Returns
** nonzero (TRUE) if it is not possible, faults properly reported. Needs valid
** compile state. */
static auint pass2_crsym(uint8 const* nam, compst_t* hnd)
{
 uint8 s[80];

 if (pass2_tcnt == SYMTABLE_SIZE){
  snprintf((char*)(&s[0]), 80U, "Can not add symbol %s since table is full", (char const*)(nam));
  fault_printat(FAULT_FAIL, &s[0], hnd);
  return 1U;
 }
 if (compst_copysym(hnd, &(pass2_symt[pass2_tcnt].nam[0]), nam) == (SYMB_MAX - 1U)){
  snprintf((char*)(&s[0]), 80U, "Symbol %s might be too long", (char const*)(nam));
  fault_printat(FAULT_NOTE, &s[0], hnd);
 }
 pass2_symt[pass2_tcnt].def = 0U;
 pass2_symt[pass2_tcnt].val = 0U; /* Undefined symbols should be substitued to zero */
 pass2_symt[pass2_tcnt].use = NULL;
 pass2_tcnt++;
 return 0U;
}



/* Adds a symbol value (definition) to the symbol table. Prints a fault if it
** is not possible for the symbol table being full. The symbol name can be
** discarded after addition. The name can be provided as text pointer into
** source fragment. */
void  pass2_addsymval(uint8 const* nam, uint32 val, compst_t* hnd)
{
 auint i;
 uint8 s[80];

 /* Try to find name in the table. If the symbol already had usages, it will
 ** be found (also will be found if it was already defined). */
 i = pass2_snfind(nam, hnd);
 if (i < pass2_tcnt){
  if (pass2_symt[i].def){ /* Symbol already defined - warning! */
   snprintf((char*)(&s[0]), 80U, "Redefinition of symbol %s", (char const*)(nam));
   fault_printat(FAULT_WARN, &s[0], hnd);
   snprintf((char*)(&s[0]), 80U, "Location of previous definition");
   fault_print(FAULT_NOTE, &s[0], &(pass2_symt[i].fof));
  }
 }else{
  if (pass2_crsym(nam, hnd)){ return; }
 }

 /* Add the symbol value & offset */
 pass2_symt[i].val = val;
 pass2_symt[i].def = 1U;
 fault_fofget(&(pass2_symt[i].fof), hnd, &(pass2_symt[i].fil[0]));
}



/* Retrieves symbol value if any. Returns nonzero (TRUE) if the symbol is
** defined, and fills val in, zero (FALSE) otherwise, setting val zero. The
** name can be provided as text pointer into source fragment. */
auint pass2_getsymval(uint8 const* nam, uint32* val, compst_t* hnd)
{
 auint i;

 /* Try to find name in the table. If the symbol already had usages or is
 ** really defined, it will be found. */
 i = pass2_snfind(nam, hnd);
 if (i < pass2_tcnt){
  if (pass2_symt[i].def){ /* Symbol defined, so return */
   *val = pass2_symt[i].val;
   return 1U;
  }
 }

 *val = 0U;
 return 0U; /* Symbol not defined */
}



/* Adds a symbol usage to the symbol table. The off and use parameters are
** formed according to valwr_write(). The name can be provided as text pointer
** into source fragment. */
void  pass2_addsymuse(uint8 const* nam, auint off, auint use, compst_t* hnd)
{
 auint i;
 uint8 s[80];

 /* Try to find name in the table. If the symbol already had usages or is
 ** really defined, it will be found. */
 i = pass2_snfind(nam, hnd);
 if (i == pass2_tcnt){ /* Symbol not found - need to create it */
  if (pass2_crsym(nam, hnd)){ return; }
 }

 /* Try to add new usage. */
 if (pass2_ucnt == SYMUSE_SIZE){
  snprintf((char*)(&s[0]), 80U, "Can not add symbol %s since table is full", (char const*)(nam));
  fault_printat(FAULT_FAIL, &s[0], hnd);
  return;
 }
 pass2_symu[pass2_ucnt].off = off;
 pass2_symu[pass2_ucnt].use = use;
 pass2_symu[pass2_ucnt].nxt = pass2_symt[i].use;
 pass2_symt[i].use = &(pass2_symu[pass2_ucnt]);
 fault_fofget(&(pass2_symu[pass2_ucnt].fof), hnd, &(pass2_symu[pass2_ucnt].fil[0]));
 pass2_ucnt++;

 /* If the symbol is undefined, add the current location as it's fault offset.
 ** So if it remains undefined, some offset may be printed to help looking
 ** for the problem. */
 if (pass2_symt[i].def == 0U){
  fault_fofget(&(pass2_symt[i].fof), hnd, &(pass2_symt[i].fil[0]));
 }
}
