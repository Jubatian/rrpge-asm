/**
**  \file
**  \brief     Third pass logic of the assembler.
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEv2 (version 2 of the RRPGE License):
**             see LICENSE.GPLv3 and LICENSE.RRPGEv2 in the project root.
**  \date      2014.05.02
**
**
** This processes incbins building up the complete binary.
*/



#include "pass3.h"
#include "strpr.h"
#include "compst.h"
#include "litpr.h"
#include "fault.h"



/* Size of bindata storage */
#define BINDTB_SIZE  4096U


/* Filename string copy maximum size */
#if (FILE_MAX > LINE_MAX)
#define COPY_MAX LINE_MAX
#else
#define COPY_MAX FILE_MAX
#endif


/* Bindata table entry */
typedef struct{
 auint use;                 /* 0: unused, 1: normal binary, 2: app. header */
 auint pg;                  /* Page to include bindata at (not used for app. head) */
 auint off;                 /* Offset within page to start at */
 uint8 inc[FILE_MAX];       /* File name to include */
 uint8 fil[FILE_MAX];       /* File name of bindata definition source */
 fault_off_t fof;           /* Location of definition for fault */
}pass3_bine_t;

/* The bindata table */
static pass3_bine_t pass3_bint[BINDTB_SIZE];

/* Simple occupation map for app. header. bit0: Occupied from pass1, bit1:
** occupied from here. */
static uint8 pass3_apho[0x1000U];

/* Number of bindata entries placed in the bindata table */
static auint pass3_cnt = 0U;



/* Executes the third pass. Takes the file descriptor of the output binary and
** populates it beginning from it's current seek position (so it has to be
** positioned beforehands!) page by page with the binary data acquired from
** incbins, listed in an internal structure (it processes application header
** incbins in a seperate pass after this). Returns nonzero (TRUE) if failed
** printing the reason of failure. When populating the app. header it also
** checks for collisions, using the app. header usage map. The last parameter
** is the count of binary pages, used for checking and padding the binary. */
auint pass3_run(FILE* obi, uint32 const* aphu, auint bpages)
{
 uint8  s[80];
 uint8  serrn[80];
 auint  i;
 auint  fp = 0U;            /* Position in binary data (word) */
 auint  tfp = 0U;
 auint  tof;
 auint  min;
 uint8  w[2];
 FILE*  bif;
 size_t frv;

 /* General algorithm:
 ** This pass is performed in two: First the binary data is populated
 ** incrementally. This means picking the smallest page:offset binary,
 ** inserting it into the file, clearing usage, and continuing. This way
 ** overlaps can be detected on the run. The second pass deals with the app.
 ** header. It is done sequentally by the order of header bindatas as the
 ** occupation map can be filled in the process. */

 while (1){

  /* Find next smallest binary offset */

  min = 0xFFFFFFFFU;
  tof = pass3_cnt;
  for (i = 0U; i < pass3_cnt; i++){
   if (pass3_bint[i].use == 1U){  /* A data binary */
    if (min > ((pass3_bint[i].pg << 12) + pass3_bint[i].off)){
     tof = i;
     min = (pass3_bint[i].pg << 12) + pass3_bint[i].off;
    }
   }
  }
  if (tof == pass3_cnt){ break; } /* No more to include */
  tfp = tof;

  /* Pad file until it's position */

  if (fp > min){ goto fault_ovr; }
  w[0] = 0U;
  w[1] = 0U;
  for (; fp < min; fp++){
   if (fwrite(&w[0], 1U, 2U, obi) != 2U){ goto fault_wrt; }
  }

  /* Output binary file until it's eof. Note that necessarily words are written */

  bif = fopen((char const*)(&pass3_bint[tof].inc[0]), "rb");
  if (bif == NULL){ goto fault_opn; }
  while (1){
   frv = fread(&w[0], 1U, 2U, bif);
   if (frv != (size_t)(2)){
    if (feof(bif)){
     if (frv == (size_t)(0)){ break; }  /* End of binary data at word boundary */
    }else{
     goto fault_red;
    }
   }
   if (fwrite(&w[0], 1U, 2U, obi) != (size_t)(2)){
    fclose(bif);
    goto fault_wrt;
   }
   fp ++;
   if (frv != (size_t)(2)){ break; }    /* Binary data ended at non-word boundary */
  }
  fclose(bif);                    /* (Don't care for return, it was read only) */

  /* Done with binary include, go on for next, clear this */

  pass3_bint[tof].use = 0U;

  /* Check if this binary data overflowed the binary */

  if (fp > (bpages << 12)){ goto fault_big; }

 }

 /* Pad application binary to the specified size */

 tof = tfp;                       /* Last output binary data (if any, may need to fix later) */
 w[0] = 0U;
 w[1] = 0U;
 while (fp < (bpages << 12)){
  if (fwrite(&w[0], 1U, 2U, obi) != 2U){ goto fault_wrt; }
  fp ++;
 }

 /* Binary data is done, need to process app. header. It is a bit messier
 ** since occupation data has to be cared for as well */

 memset(&pass3_apho[0], 0U, sizeof(pass3_apho));
 for (i = 0U; i < 0x1000U; i++){
  if ((aphu[i >> 5] & (1U << i)) != 0U){
   pass3_apho[i] |= 1U;
  }
 }

 for (tof = 0U; tof < pass3_cnt; tof++){
  if (pass3_bint[tof].use == 2U){ /* A header incbin */

   /* Set up start position */

   fp = pass3_bint[tof].off;
   if (fseek(obi, (fp << 1), SEEK_SET)){ goto fault_sek; } /* Back to the app. header */

   /* Open it, and attempt to copy it in hoping it won't collide */

   bif = fopen((char const*)(&pass3_bint[tof].inc[0]), "rb");
   if (bif == NULL){ goto fault_opn; }
   while (1){
    frv = fread(&w[0], 1U, 2U, bif);
    if (frv != (size_t)(2)){
     if (feof(bif)){
      if (frv == (size_t)(0)){ break; } /* End of binary data at word boundary */
     }else{
      goto fault_red;
     }
    }
    if (fp >= 0x1000U){ goto fault_hof; }
    if (pass3_apho[fp] != 0U){ goto fault_col; }
    if (fwrite(&w[0], 1U, 2U, obi) != (size_t)(2)){
     fclose(bif);
     goto fault_wrt;
    }
    pass3_apho[fp] |= 2U;
    fp ++;
    if (frv != (size_t)(2)){ break; }   /* Binary data ended at non-word boundary */
   }
   fclose(bif);                   /* (Don't care for return, it was read only) */

  }
 }

 /* Everything done succesfully */

 pass3_clear();
 return 0U;


fault_sek:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Unable seek taget binary: %s", (char const*)(&serrn[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;

fault_col:

 fclose(bif);
 snprintf((char*)(&s[0]), 80U, "Header data overlap while processing %s", (char const*)(&pass3_bint[tof].inc[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;

fault_hof:

 fclose(bif);
 snprintf((char*)(&s[0]), 80U, "Header space exceed while processing %s", (char const*)(&pass3_bint[tof].inc[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;

fault_red:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 fclose(bif);
 snprintf((char*)(&s[0]), 80U, "Unable to read %s: %s", (char const*)(&pass3_bint[tof].inc[0]), (char const*)(&serrn[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;

fault_opn:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Unable to open %s: %s", (char const*)(&pass3_bint[tof].inc[0]), (char const*)(&serrn[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;

fault_ovr:

 snprintf((char*)(&s[0]), 80U, "Binary data %s overlaps with previous", (char const*)(&pass3_bint[tof].inc[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;

fault_big:

 snprintf((char*)(&s[0]), 80U, "Binary data became larger than specified at %s", (char const*)(&pass3_bint[tof].inc[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;

fault_wrt:

 strerror_r(errno, (char*)(&serrn[0]), 80U);
 serrn[79] = 0U;
 snprintf((char*)(&s[0]), 80U, "Failed to write target binary: %s", (char const*)(&serrn[0]));
 fault_print(FAULT_FAIL, &s[0], &pass3_bint[tof].fof);
 return 1U;
}



/* Clears all pass3 state */
void  pass3_clear(void)
{
 memset(&(pass3_bint[0]), 0U, sizeof(pass3_bint));
 memset(&(pass3_apho[0]), 0U, sizeof(pass3_apho));
 pass3_cnt = 0U;
}



/* Checks current source line at current position for a valid bindata include.
** If so, it is submitted to the internal bindata list for pass3. Provides the
** following returns:
** 0: Fault, compilation should stop, fault printed.
** 1: Succesfully parsed something.
** 2: No content usable, but other parsers may try. */
auint pass3_procbindata(compst_t* hnd)
{
 uint8  s[80];
 uint8  ste[LINE_MAX];
 uint8 const* src = compst_getsstr(hnd) + compst_getcoff(hnd);
 auint  i;
 auint  u;
 auint  t;
 uint32 v;
 auint  beg = strpr_nextnw(src, 0U);

 /* Check if it is a bindata */

 if (compst_issymequ(NULL, &(src[beg]), (uint8 const*)("bindata"))){

  if (pass3_cnt == BINDTB_SIZE){ goto fault_lrg; }

  beg = strpr_nextnw(src, beg + 7U);
  i = strpr_extstr(&(ste[0]), &src[beg], LINE_MAX);
  if (i == 0){ goto fault_inv; }
  strpr_copy(&(pass3_bint[pass3_cnt].inc[0]), &ste[0], COPY_MAX);

  /* Page specification: Need to catch 'h' here for app. head */

  beg = strpr_nextnw(src, beg + i);
  if ( (src[beg] == (uint8)('h')) &&
       (!strpr_issym(src[beg + 1U])) ){ /* App. header */
   pass3_bint[pass3_cnt].use = 2U;      /* Set usage accordingly */
   beg = strpr_nextnw(src, beg + 1U);
  }else{                                /* Binary data */
   pass3_bint[pass3_cnt].use = 1U;      /* Set usage accordingly */
   t = litpr_getval(&(src[beg]), &u, &v, hnd);
   if (t != LITPR_VAL){ goto fault_inv; }
   pass3_bint[pass3_cnt].pg  = v;
   beg = beg + u;
  }

  /* Offset specification if any */

  pass3_bint[pass3_cnt].off = 0U;
  if (src[beg] == (uint8)(',')){        /* There is an offset */
   beg = strpr_nextnw(src, beg + 1U);
   t = litpr_getval(&(src[beg]), &u, &v, hnd);
   if (t != LITPR_VAL){ goto fault_inv; }
   pass3_bint[pass3_cnt].off = v;
   beg = beg + u;
  }

  /* Check string end */

  if (!strpr_isend(src[beg])){ goto fault_inv; }

  /* Finish building the bindata table entry */

  fault_fofget(&(pass3_bint[pass3_cnt].fof), hnd, &(pass3_bint[pass3_cnt].fil[0]));
  pass3_cnt++;
  return 1U;                            /* Succesfully parsed */

 }

 return 2U; /* Not a bindata, but OK */


fault_inv:

 compst_setcoff(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Malformed bindata");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;

fault_lrg:

 compst_setcoff(hnd, beg);
 snprintf((char*)(&s[0]), 80U, "Too many bindata entries, can not compile");
 fault_printat(FAULT_FAIL, &s[0], hnd);
 return 1U;

}
