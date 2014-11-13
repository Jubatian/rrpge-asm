/**
**  \file
**  \brief     Symbol table
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.11.13
*/


#include "symtab.h"
#include "fault.h"



/* Maximal hop count to pass during resolving */
#define MAX_HOPS 16U


/* Symbol definition structure */
/* Note: currently memory-wise this is quite inefficient due to the large
** "fil" arrays. Later this may be fixed by creating a file name pool to be
** referred by fault offsets (maybe part of fault, maybe elsewhere). */
typedef struct{
 auint cmd;             /* Command for combining the sources */
 auint s0i;             /* Source 0 value / ID / string index */
 auint s1i;             /* Source 1 value / ID / string index */
 auint bdi;             /* String bind index (0: no binding) */
 uint8 fil[FILE_MAX];   /* File name for fault */
 fault_off_t fof;       /* Location of definition for fault */
}symtab_def_t;

/* Symbol usage definition structure */
typedef struct{
 auint sec;             /* Section of data */
 auint off;             /* Offset of data within the section */
 auint use;             /* Data type (as defined in valwr.h) */
 auint bdi;             /* Definition bind index */
 uint8 fil[FILE_MAX];   /* File name for fault */
 fault_off_t fof;       /* Location of definition for fault */
}symtab_use_t;


/* Symbol table data structure */
struct symtab_s{
 section_t*    sec;     /* Bound section */
 compst_t*     cst;     /* Bound compile state */
 symtab_def_t* def;     /* Symbol definitions */
 auint         dct;     /* Count of definitions */
 auint         dsi;     /* Size of definition array */
 symtab_use_t* use;     /* Symbol use definitions */
 auint         uct;     /* Count of symbol uses */
 auint         usi;     /* Size of usage array */
 uint8*        str;     /* String pool */
 auint         spt;     /* Free slot index within string pool */
 auint         ssi;     /* String pool size */
};



/* Built-in singleton object's components */
static symtab_def_t symtab_def[SYMTAB_DEF_SIZE];
static symtab_use_t symtab_usd[SYMTAB_USE_SIZE];
static uint8        symtab_str[SYMTAB_STR_SIZE];
static symtab_t     symtab_tab = {
 NULL, NULL,
 symtab_def, 0U, SYMTAB_DEF_SIZE,
 symtab_usd, 0U, SYMTAB_USE_SIZE,
 symtab_str, 0U, SYMTAB_STR_SIZE};



/* Finds a symbol in the symbol table. Returns the offset, or zero if not
** found. */
static auint symtab_snfind(symtab_t* hnd, uint8 const* nam)
{
 auint i = 1U;
 auint p = 0U;

 while (i < (hnd->spt)){

  if ((p == 0U) && (hnd->str[i] != 0U)){ /* New symbol name starts */
   if (compst_issymequ(hnd->cst, nam, &(hnd->str[i]))){ return i; }
  }
  p = hnd->str[i];

  i++;
 }

 return 0U;
}



/* Adds new string to symbol table. Returns it's index, or zero if failed
** (fault printed) */
static auint symtab_snadd(symtab_t* hnd, uint8 const* nam)
{
 uint8 s[80];
 auint r = hnd->spt;
 auint t;

 if (((hnd->spt) + SYMB_MAX) >= (hnd->ssi)){ goto fault_sne; }
 t = compst_copysym(hnd->cst, &(hnd->str[hnd->spt]), nam) + 1U;
 if (t == SYMB_MAX){ /* Notification only, may pass without problems */
  snprintf((char*)(&s[0]), 80U, "Symbol %s might be too long", (char const*)(nam));
  fault_printat(FAULT_NOTE, &s[0], hnd->cst);
 }
 hnd->spt += t;
 return r;

fault_sne:

 snprintf((char*)(&s[0]), 80U, "Symbol name pool exhausted");
 fault_printat(FAULT_FAIL, &s[0], hnd->cst);
 return 0U;
}



/* Tries to find, and if not found, attempt to add a symbol name string.
** Returns string index, or zero if it was not possible to do it (fault
** printed). */
static auint symtab_snfindadd(symtab_t* hnd, uint8 const* nam)
{
 auint i = symtab_snfind(hnd, nam);
 if (i == 0U){
  i = symtab_snadd(hnd, nam);
 }
 return i;
}



/* Get built-in singleton object handle. */
symtab_t* symtab_getobj(void)
{
 return &symtab_tab;
}



/* Initialize or resets a symbol table object (size is unchanged). The given
** section and compile state object is bound to the symbol table. */
void  symtab_init(symtab_t* hnd, section_t* sec, compst_t* cst)
{
 hnd->sec = sec;
 hnd->cst = cst;
 hnd->dct = 1U; /* Zero in all specify non-valid */
 hnd->uct = 1U;
 hnd->spt = 1U;
 hnd->str[0] = 0U;
}



/* Get bound compilation state object. */
compst_t*  symtab_getcompst(symtab_t* hnd)
{
 return hnd->cst;
}


/* Get bound section object. */
section_t* symtab_getsectob(symtab_t* hnd)
{
 return hnd->sec;
}



/* Add new symbol definition to the table. Prints fault if the symbol
** definition can not be added. Returns ID of the definition (nonzero), or
** zero (!) if it was not possible to add. Parameters: 'cmd': The command to
** use for generating the value of the symbol, 's0v', 's0n': Source 0, 's1v',
** 's1n': Source 1. The name is used only if 'cmd' requests so (then a symbol
** usage entry is generated). */
auint symtab_addsymdef(symtab_t* hnd, auint cmd,
                       auint s0v, uint8 const* s0n,
                       auint s1v, uint8 const* s1n)
{
 uint8 s[80];

 /* First process strings if any was requested by command */

 if ((cmd & SYMTAB_CMD_S0N) != 0U){ /* Source 0 */
  s0v = symtab_snfindadd(hnd, s0n);
  if (s0v == 0U){ goto fault_ot0; } /* Failed */
 }
 if ((cmd & SYMTAB_CMD_S1N) != 0U){ /* Source 1 */
  s1v = symtab_snfindadd(hnd, s1n);
  if (s1v == 0U){ goto fault_ot0; } /* Failed */
 }

 /* Check ID validity for ID parameters */

 if ((cmd & SYMTAB_CMD_S0I) != 0U){ /* Source 0 */
  if ( (s0v == 0U) ||
       (s0v >= hnd->dct) ){ goto fault_idi; }
 }
 if ((cmd & SYMTAB_CMD_S1I) != 0U){ /* Source 1 */
  if ( (s1v == 0U) ||
       (s1v >= hnd->dct) ){ goto fault_idi; }
 }

 /* Add the new symbol definition */

 if ((hnd->dct) == (hnd->dsi)){ goto fault_sde; }
 hnd->def[hnd->dct].cmd = cmd;
 hnd->def[hnd->dct].s0i = s0v;
 hnd->def[hnd->dct].s1i = s1v;
 hnd->def[hnd->dct].bdi = 0U;
 fault_fofget(&(hnd->def[hnd->dct].fof), hnd->cst, &(hnd->def[hnd->dct].fil[0]));
 hnd->dct ++;
 return (hnd->dct - 1U);

fault_sde:

 snprintf((char*)(&s[0]), 80U, "Symbol definition table exhausted");
 fault_printat(FAULT_FAIL, &s[0], hnd->cst);
 return 0U;

fault_idi:

 snprintf((char*)(&s[0]), 80U, "Symbol definition ID invalid");
 fault_printat(FAULT_FAIL, &s[0], hnd->cst);
 return 0U;

fault_ot0:

 return 0U;
}



/* Gets a symbol definition by symbol name. If the symbol definition does not
** exists, it creates a "dangling" definition referring the given name, and
** returns that. Returns ID of the definition (nonzero), or zero (!) if it is
** not possible to do this (fault code printed). */
auint symtab_getsymdef(symtab_t* hnd, uint8 const* nam)
{
 auint i;
 auint j;
 auint r = 0U;

 i = symtab_snfind(hnd, nam);

 if (i != 0U){ /* String already exists, check for related definition */

  for (j = 1U; j < (hnd->dct); j++){
   if ((hnd->def[j].bdi) == i){ /* OK, appropriate definition found */
    r = j;
    break;
   }
  }

 }

 if (r == 0U){ /* Does not exist: add "dangling" symbol definition */

  r = symtab_addsymdef(hnd, SYMTAB_CMD_MOV | SYMTAB_CMD_S0N, 0, nam, 0, NULL);

 }

 return r;
}



/* Add symbol name string binding to a symbol definition. Prints fault if it
** can not be done. Returns nonzero on failure (symbol name space exhausted
** or redefinition). The name terminates with a white character, and does not
** need to be preserved after addition. */
auint symtab_bind(symtab_t* hnd, uint8 const* nam, auint id)
{
 uint8 s[80];
 auint i;
 auint j;

 i = symtab_snfind(hnd, nam);

 if (i != 0U){ /* String already exists, check for multiple definitions */

  for (j = 1U; j < (hnd->dct); j++){
   if ((hnd->def[j].bdi) == i){ goto fault_rdf; }
  }

 }else{        /* String does not exist, add it */

  i = symtab_snadd(hnd, nam);
  if (i == 0U){ goto fault_ot1; } /* Failed to add */

 }

 hnd->def[id].bdi = i; /* Bind it */
 return 0U;

fault_rdf:

 snprintf((char*)(&s[0]), 80U, "Redefinition of symbol %s", (char const*)(&(hnd->str[hnd->def[j].bdi])));
 fault_printat(FAULT_FAIL, &s[0], hnd->cst);
 snprintf((char*)(&s[0]), 80U, "Location of previous definition");
 fault_print(FAULT_NOTE, &s[0], &(hnd->def[j].fof));
 return 1U;

fault_ot1:

 return 1U;
}



/* Add symbol usage. The 'off' parameter supplies the offset within the
** section where the symbol will have to be resolved. The 'use' parameter
** gives the usage as defined in valwr.h. A symbol definition has to be passed
** to this function, however it is possible to submit usage for a not-yet
** defined symbol by creating a "dangling" definition (with a MOV command,
** symbol name source) to bind to. Returns nonzero and prints fault if it is
** not possible to add this. */
auint symtab_use(symtab_t* hnd, auint def, auint off, auint use)
{
 uint8 s[80];

 /* Check ID validity */

 if ( (def == 0U) ||
      (def >= hnd->dct) ){ goto fault_idi; }

 /* Add the new symbol usage definition */

 if ((hnd->uct) == (hnd->usi)){ goto fault_sus; }
 hnd->use[hnd->uct].sec = section_getsect(hnd->sec);
 hnd->use[hnd->uct].off = off;
 hnd->use[hnd->uct].use = use;
 hnd->use[hnd->uct].bdi = def;
 fault_fofget(&(hnd->use[hnd->uct].fof), hnd->cst, &(hnd->use[hnd->uct].fil[0]));
 hnd->uct ++;
 return 0U;

fault_sus:

 snprintf((char*)(&s[0]), 80U, "Symbol usage table exhausted");
 fault_printat(FAULT_FAIL, &s[0], hnd->cst);
 return 1U;

fault_idi:

 snprintf((char*)(&s[0]), 80U, "Symbol definition ID invalid");
 fault_printat(FAULT_FAIL, &s[0], hnd->cst);
 return 0U;
}



/* Internal recursive resolver. Hops is the hop count passed during resolving,
** used to break infinite loops. Returns nonzero on failure. Resolved value is
** generated into 'v'. 'i' is the definition index to resolve in the
** definition table, 'dct' is the size of the table. 'hops' should be started
** with 0, it is the iteration count guard. Return is 2 + offset (in string
** pool) if an undefined symbol is encountered, no fault is printed this case.
** For other faults the return is 1, fault is printed. */
static auint symtab_recres(symtab_def_t* def, auint i, auint dct, auint hops, auint* v)
{
 uint8 s[80];
 auint r;
 auint j;
 auint t;

 if (hops >= MAX_HOPS){ goto fault_hop; }

 /* Converts by-string symbol connections to by-id connections */

 if ((def[i].cmd & SYMTAB_CMD_S0N) != 0U){ /* Source 0 is string, need to convert it */
  t = def[i].s0i;
  for (j = 1U; j < dct; j++){
   if ((def[j].bdi) == t){
    def[i].s0i = j;
    break;
   }
  }
  if (j == dct){ t += 2U; goto fault_uds; }
  def[i].cmd |=  (auint)(SYMTAB_CMD_S0I);
  def[i].cmd &= ~(auint)(SYMTAB_CMD_S0N);  /* Convert to ID bind */
 }
 if ((def[i].cmd & SYMTAB_CMD_S1N) != 0U){ /* Source 1 is string, need to convert it */
  t = def[i].s1i;
  for (j = 1U; j < dct; j++){
   if ((def[j].bdi) == t){
    def[i].s1i = j;
    break;
   }
  }
  if (j == dct){ t += 2U; goto fault_uds; }
  def[i].cmd |=  (auint)(SYMTAB_CMD_S1I);
  def[i].cmd &= ~(auint)(SYMTAB_CMD_S1N);  /* Convert to ID bind */
 }

 /* Resolve by-id connections, removing any references to further symbols */

 if ((def[i].cmd & SYMTAB_CMD_S0I) != 0U){ /* Need to resolve Source 0 */
  t = symtab_recres(def, def[i].s0i, dct, hops + 1U, &r);
  if      (t == 0U){ def[i].s0i = r; }
  else if (t >= 2U){ goto fault_uds; }
  else             { goto fault_ot3; }
 }
 if ((def[i].cmd & SYMTAB_CMD_S1I) != 0U){ /* Need to resolve Source 1 */
  t = symtab_recres(def, def[i].s1i, dct, hops + 1U, &r);
  if      (t == 0U){ def[i].s1i = r; }
  else if (t >= 2U){ goto fault_uds; }
  else             { goto fault_ot3; }
 }
 def[i].cmd &= 0xFFU;                      /* Remove any high bits from command (processed) */

 /* Evaulate operation between the two source values, converting the symbol to
 ** a simple MOV symbol, so value can be returned. */

 switch (def[i].cmd){

  case SYMTAB_CMD_ADD: r = def[i].s0i +  def[i].s1i; break;
  case SYMTAB_CMD_SUB: r = def[i].s0i -  def[i].s1i; break;
  case SYMTAB_CMD_MUL: r = def[i].s0i *  def[i].s1i; break;
  case SYMTAB_CMD_DIV:
   if (def[i].s1i == 0U){ goto fault_div; }
   r = def[i].s0i / def[i].s1i;
   break;
  case SYMTAB_CMD_MOD:
   if (def[i].s1i == 0U){ goto fault_div; }
   r = def[i].s0i % def[i].s1i;
   break;
  case SYMTAB_CMD_AND: r = def[i].s0i &  def[i].s1i; break;
  case SYMTAB_CMD_OR:  r = def[i].s0i |  def[i].s1i; break;
  case SYMTAB_CMD_XOR: r = def[i].s0i ^  def[i].s1i; break;
  case SYMTAB_CMD_SHR: r = def[i].s0i >> (def[i].s1i & 31U); break;
  case SYMTAB_CMD_SHL: r = def[i].s0i << (def[i].s1i & 31U); break;
  default:             r = def[i].s0i; break;

 }

 def[i].cmd = SYMTAB_CMD_MOV;              /* Resolved */
 def[i].s0i = r;
 *v         = r;

 return 0U;

fault_hop:

 snprintf((char*)(&s[0]), 80U, "Hop count (%i) during resolution exceed", (auint)(MAX_HOPS));
 fault_print(FAULT_FAIL, &s[0], &(def[i].fof));
 return 1U;

fault_div:

 snprintf((char*)(&s[0]), 80U, "Divison by zero");
 fault_print(FAULT_FAIL, &s[0], &(def[i].fof));
 return 1U;

fault_uds:

 return t; /* Contains string pool offset + 2U */

fault_ot3:

 return 1U;
}



/* Attempts to resolve a given symbol to it's value. Returns nonzero on
** success, filling in 'val' with the value of the symbol. Otherwise it
** returns zero, and does not alter 'val'. This may be used to attempt to
** pre-resolve symbols for optimizing instruction sizes where possible. */
auint symtab_resolvesym(symtab_t* hnd, auint id, auint* val)
{
 auint r;

 /* Note: faults printed during resolution will be ignored here, just causing
 ** the symbol not being resolved. They will halt the compile later when the
 ** full resolution is attempted. Not nice, but passes. */

 auint t = symtab_recres(hnd->def, id, hnd->dct, 0U, &r);
 if (t == 0U){
  *val = r;
  return 1U;
 }else{
  return 0U;
 }
}



/* Resolves the symbol table into the bound section. Prints fault and returns
** nonzero if it is not possible to resolve. */
auint symtab_resolve(symtab_t* hnd)
{
 uint8 s[80];
 auint i;
 auint dum;
 auint t;
 symtab_use_t* use = hnd->use;
 symtab_def_t* def = hnd->def;
 auint uct = hnd->uct;
 auint dct = hnd->dct;

 /* Resolve all symbol definitions into MOVs */

 for (i = 1U; i < dct; i++){
  t = symtab_recres(def, i, dct, 0U, &dum);
  if (t == 1U){ goto fault_ot4; } /* Other fault, fault printed, just leave */
  if (t >= 2U){ goto fault_udd; } /* Undefined symbol: string pool offset + 2U in 't' */
 }

 /* Resolve symbol usages into the appropriate section:offset locations */

 for (i = 1U; i < uct; i++){
  section_setsect(hnd->sec, use[i].sec);
  if (valwr_write(hnd->sec, def[use[i].bdi].s0i, use[i].off, use[i].use, &(use[i].fof))){
   goto fault_ot4;
  }
 }

 /* All done */

 return 0U;

fault_udd:

 snprintf((char*)(&s[0]), 80U, "Undefined symbol: %s", (char const*)(&(hnd->str[t - 2U])));
 fault_print(FAULT_FAIL, &s[0], &(def[i].fof));
 return 1U;

fault_ot4:

 return 1U;
}
