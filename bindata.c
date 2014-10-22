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


#include "bindata.h"
#include "strpr.h"


/* Bindata definition structure for FILE section bindatas */
/* Note: Like in symtab, quite inefficient memory-wise due to the large "fil"
** arrays. Some file name pool needed later. */
typedef struct{
 uint8 bfi[FILE_MAX];   /* File name of bindata */
 auint siz;             /* Size of bindata in words */
 uint8 fil[FILE_MAX];   /* File name for fault */
 fault_off_t fof;       /* Location of definition for fault */
}bindata_def_t;


/* Bindata manager object */
struct bindata_s{
 bindata_def_t* def;    /* Bindata definition */
 auint          dct;    /* Count of definitions */
 auint          dsi;    /* Size of definition array */
};



/* Built-in singleton object's components */
static bindata_def_t bindata_def[BINDATA_MAX];
static bindata_t     bindata_tab = {
 bindata_def, 0U, BINDATA_MAX
};



/* Get built-in singleton object handle. */
bindata_t* bindata_getobj(void)
{
 return &bindata_tab;
}


/* Initializes or resets a bindata object. */
void  bindata_init(bindata_t* hnd)
{
 hnd->dct = 0U;
}



/* Checks the current source line at the current position for a valid bindata
** include. Then, depending on the section, either processes it directly into
** the selected section or if it is the FILE section, puts it on the bindata
** manager's list for later processing (in pass3). Returns one of the defined
** PARSER return codes (defined in types.h). */
auint bindata_proc(bindata_t* hnd, symtab_t* stb)
{
 uint8        s[80];
 uint8        e[80];
 uint8        ste[LINE_MAX];
 section_t*   sec = symtab_getsectob(stb);
 compst_t*    cst = symtab_getcompst(stb);
 uint8 const* src = compst_getsstrcoff(cst);
 auint        beg = strpr_nextnw(src, 0U);
 FILE*        bif;
 size_t       frv;
 auint        i;
 uint8        c;

 /* Check if it is a bindata */

 if ((compst_issymequ(NULL, &(src[beg]), (uint8 const*)("bindata"))) == 0U){
  return PARSER_OK; /* Not a bindata, something else may try */
 }

 /* Extract file name */

 beg = strpr_nextnw(src, beg + 7U);
 i   = strpr_extstr(&(ste[0]), &src[beg], LINE_MAX);
 if (i == 0U){ goto fault_in0; }
 beg = strpr_nextnw(src, beg + i);
 if (!strpr_isend(src[beg])){ goto fault_in0; }
 compst_setcoffrel(cst, beg);

 /* Depending on section, process it */

 if (section_getsect(sec) != SECT_FILE){ /* Direct processing */

  if (section_getsect(sec) == SECT_ZERO){ goto fault_zr0; }

  bif = fopen((char const*)(&(ste[0])), "rb");
  if (bif == NULL){ goto fault_op0; }

  while (1){
   frv = fread(&c, 1U, 1U, bif);
   if (frv != (size_t)(1U)){
    if (feof(bif)){ break; } /* End of bindata */
    else{ goto fault_rd0; }
   }
   if (section_pushb(sec, (auint)(c)) != 0U){ goto fault_se0; }
  }

  fclose(bif);               /* It was read only, don't care for errors here */

 }else{                      /* Process into table */

  goto fault_uns;

 }

 /* Done */

 return PARSER_END;

fault_in0:

 compst_setcoffrel(cst, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed bindata");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return PARSER_ERR;

fault_zr0:

 snprintf((char*)(&s[0]), 80U, "Bindata in ZERO section");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return PARSER_ERR;

fault_op0:

 strerror_r(errno, (char*)(&e[0]), 80U);
 e[79] = 0U;
 fclose(bif);
 snprintf((char*)(&s[0]), 80U, "Unable to open %s: %s", (char const*)(&ste[0]), (char const*)(&e[0]));
 fault_printat(FAULT_FAIL, &s[0], cst);
 return PARSER_ERR;

fault_rd0:

 strerror_r(errno, (char*)(&e[0]), 80U);
 e[79] = 0U;
 fclose(bif);
 snprintf((char*)(&s[0]), 80U, "Unable to read %s: %s", (char const*)(&ste[0]), (char const*)(&e[0]));
 fault_printat(FAULT_FAIL, &s[0], cst);
 return PARSER_ERR;

fault_se0:

 fclose(bif);
 snprintf((char*)(&s[0]), 80U, "Overlap or out of section encountered");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return PARSER_ERR;

fault_uns:

 snprintf((char*)(&s[0]), 80U, "No FILE section support yet");
 fault_printat(FAULT_FAIL, &s[0], cst);
 return PARSER_ERR;
}



/* Processes bindata table, and produces the according output in the passed
** file. The output is sequential, no seeking is performed on the target file.
** Returns nonzero on failure, fault code printed. */
auint bindata_out(bindata_t* hnd, FILE* ofl)
{
 /* No FILE section support yet, so no problem. */

 /* Implementation may come after literal arithmetic (since arithmetic is
 ** necessary to work with 32 bit start offsets). Within proc, the size needs
 ** to be determined to figure out any label symbol's value, then here, the
 ** contents have to be added to the application binary. */

 return 0U;
}
