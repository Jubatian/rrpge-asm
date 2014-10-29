/**
**  \file
**  \brief     Section & Data management
**  \author    Sandor Zsuga (Jubatian)
**  \copyright 2013 - 2014, GNU GPLv3 (version 3 of the GNU General Public
**             License) extended as RRPGEvt (temporary version of the RRPGE
**             License): see LICENSE.GPLv3 and LICENSE.RRPGEvt in the project
**             root.
**  \date      2014.10.22
**
**  Manages the sections and their data during the compilation. Currently
**  singleton, but desinged so it is possible to extend later.
*/



#include "section.h"



/* Just a compilation test to ensure section values stay as-is.
** (Some hard-coded numerics are necessary to distinguish special sections) */
#if ((SECT_CODE != 0U) || (SECT_DATA != 1U) || (SECT_HEAD != 2U) || (SECT_DESC != 3U) || (SECT_ZERO != 4U) || (SECT_FILE != 5U))
#error "Don't tamper with section definition values!!!"
#endif



/* Section sizes and start offsets in a combined data block */
#define CODE_S 65536U
#define DATA_S SECT_MAXRAM
#define HEAD_S 65536U
#define DESC_S 32U
#define ZERO_S SECT_MAXRAM
#define CODE_O (0U)
#define DATA_O (CODE_O + CODE_S)
#define HEAD_O (DATA_O + DATA_S)
#define DESC_O (HEAD_O + HEAD_S)
#define ZERO_O (DESC_O + DESC_S)

/* Maximal section size */
#define SECT_MAX 65536U

/* Maximal section ID which has data */
#define SECT_IDM_D SECT_DESC
/* Maximal section ID which has map */
#define SECT_IDM_M SECT_ZERO
/* Maximal section ID */
#define SECT_IDM   SECT_FILE



/* Section object structure - definition */
struct section_s{
 auint  s;              /* Currently selected section */
 auint  p[6];           /* Current pointer within section */
 auint  b[6];           /* Byte sub-offset (0 for high byte, 1 for low) */
 auint  a[6];           /* Base offset */
 uint16 d[(CODE_S + DATA_S + HEAD_S + DESC_S)];               /* Section data */
 uint32 o[(CODE_S + DATA_S + HEAD_S + DESC_S + ZERO_S) >> 5]; /* Section occupation map */
};



/* Section maximal sizes */
static auint const section_s[5] = {
 CODE_S,                /* Code */
 DATA_S,                /* Data */
 HEAD_S,                /* Head */
 DESC_S,                /* Desc */
 ZERO_S};               /* Zero */

/* Section start offsets within section data */
static auint const section_o[5] = {
 CODE_O,                /* Code */
 DATA_O,                /* Data */
 HEAD_O,                /* Head */
 DESC_O,                /* Desc */
 ZERO_O};               /* Zero (in map only) */



/* Section base symbols */
static uint8 const* section_secbs[6] = {
 (uint8 const*)("@.code"),
 (uint8 const*)("@.data"),
 (uint8 const*)("@.head"),
 (uint8 const*)("@.desc"),
 (uint8 const*)("@.zero"),
 (uint8 const*)("@.file") };




/* Internal section object */
static section_t section_obj;



/* Internal set function for an occupation map */
static void  section_i_setocc(auint off, uint32* map, auint siz)
{
 if (siz > off){
  map[off >> 5] |= (auint)(1U) << (off & 0x1FU);
 }
}



/* Internal get function for an occupation map */
static auint section_i_getocc(auint off, uint32 const* map, auint siz)
{
 if (siz > off){
  return ((map[off >> 5] >> (off & 0x1FU)) & 1U);
 }
 return 0U;
}



/* Get built-in singleton object handle. */
section_t* section_getobj(void)
{
 return &section_obj;
}



/* Retrieves string for identifying the section base symbol. These are:
** '@.code', '@.data', '@.head', '@.desc', '@.zero', and '@.file'. */
uint8 const* section_getsbstr(auint sec)
{
 if (sec <= SECT_IDM){
  return (section_secbs[sec]);
 }else{
  return NULL;
 }
}



/* Initializes or resets a section object. */
void  section_init(section_t* hnd)
{
 memset(hnd, 0U, sizeof(section_t));
 hnd->s = SECT_CODE; /* Select code section initially */
}



/* Sets section to perform operations on. */
void  section_setsect(section_t* hnd, auint sect)
{
 if (sect <= SECT_IDM){
  hnd->s = sect;
 }
}



/* Gets currently selected section. */
auint section_getsect(section_t* hnd)
{
 return (hnd->s);
}



/* Sets offset within section, word granularity. */
void  section_setoffw(section_t* hnd, auint off)
{
 hnd->p[hnd->s] = off;
 hnd->b[hnd->s] = 0U;
}



/* Gets word offset for a subsequent use with section_setw(). */
auint section_getoffw(section_t* hnd)
{
 return (hnd->p[hnd->s]); /* Byte offset simply ignored */
}



/* Gets byte offset for a subsequent use with section_setb(). */
auint section_getoffb(section_t* hnd)
{
 return (((hnd->p[hnd->s]) << 1) + (hnd->b[hnd->s]));
}



/* Pushes word data to the section. If necessary, aligns offset to word
** boundary first. Returns error code on failure (overlap or out of section's
** allowed size). */
auint section_pushw(section_t* hnd, auint data)
{
 auint s = hnd->s;

 if (hnd->b[s] != 0U){ /* Need to advance to next word */
  hnd->p[s] ++;
  hnd->b[s] = 0U;
 }

 if (s <= SECT_IDM_M){    /* Sections with map: test */

  if (section_s[s] <= (hnd->p[s])){
   return SECT_ERR_OVF;   /* Offset too large for section */
  }
  if (section_i_getocc(hnd->p[s], &(hnd->o[section_o[s] >> 5]), section_s[s]) != 0U){
   return SECT_ERR_OVR;   /* Already occupied */
  }

  section_i_setocc(hnd->p[s], &(hnd->o[section_o[s] >> 5]), section_s[s]);
  if (s <= SECT_IDM_D){   /* Sections with data: add it */
   hnd->d[section_o[s] + hnd->p[s]] |= (uint16)(data);
  }

 }

 hnd->p[s] ++;
 return SECT_ERR_OK;
}



/* Pushes byte data to the section. Returns error code on failure (overlap or
** out of section's allowed size). */
auint section_pushb(section_t* hnd, auint data)
{
 auint s = hnd->s;

 if (s <= SECT_IDM_M){    /* Sections with map: test */

  if (section_s[s] <= (hnd->p[s])){
   return SECT_ERR_OVF;   /* Offset too large for section */
  }
  if ((hnd->b[s]) == 0U){ /* Only check overlap on boundary */
   if (section_i_getocc(hnd->p[s], &(hnd->o[section_o[s] >> 5]), section_s[s]) != 0U){
    return SECT_ERR_OVR;  /* Already occupied */
   }
  }

  section_i_setocc(hnd->p[s], &(hnd->o[section_o[s] >> 5]), section_s[s]);
  if (s <= SECT_IDM_D){   /* Sections with data: add it */
   hnd->d[section_o[s] + hnd->p[s]] |= (uint16)((data & 0xFFU) << ((1U ^ (hnd->b[s])) << 3));
  }

 }

 if (hnd->b[s] != 0U){
  hnd->p[s] ++;
  hnd->b[s] = 0U;
 }else{
  hnd->b[s] = 1U;
 }

 return SECT_ERR_OK;
}



/* Changes an unit of word data at a given offset. This is meant to be used by
** second pass to substitue values which could not be resolved earlier. Will
** only have effect in areas already occupied. OR combines. */
void  section_setw(section_t* hnd, auint off, auint data)
{
 auint s = hnd->s;

 if (s <= SECT_IDM_D){    /* Only for sections with data */

  if (section_i_getocc(off, &(hnd->o[section_o[s] >> 5]), section_s[s]) != 0U){
   hnd->d[section_o[s] + off] |= (uint16)(data);
  }

 }
}



/* Changes an unit of byte data at a given (byte) offset. This is meant to be
** used by second pass to substitue values which could not be resolved
** earlier. Will only have effect in areas already occupied. OR combines. */
void  section_setb(section_t* hnd, auint off, auint data)
{
 auint s = hnd->s;

 if (s <= SECT_IDM_D){    /* Only for sections with data */

  if (section_i_getocc(off >> 1, &(hnd->o[section_o[s] >> 5]), section_s[s]) != 0U){
   hnd->d[section_o[s] + (off >> 1)] |= (uint16)((data & 0xFFU) << ((1U ^ (off & 1U)) << 3));
  }

 }
}



/* Forces an unit of word data into the section, overriding if anything is
** there. This is meant to be used by autofills. Sets occupation for the
** forced word. */
void  section_fsetw(section_t* hnd, auint off, auint data)
{
 auint s = hnd->s;

 if (s <= SECT_IDM_D){    /* Only for sections with data */

  if (off < section_s[s]){
   hnd->d[section_o[s] + off] = (uint16)(data);
   section_i_setocc(off, &(hnd->o[section_o[s] >> 5]), section_s[s]);
  }

 }
}



/* Adds padding 0x20 (space) character if necessary: overrides zeros in
** occupied areas (string terminator), and writes any unoccupied area. Uses
** word offsets, applying the padding to both bytes as necessary. */
void  section_strpad(section_t* hnd, auint off)
{
 auint s = hnd->s;

 if (s <= SECT_IDM_D){    /* Only for sections with data */

  if (off < section_s[s]){

   if (section_i_getocc(off, &(hnd->o[section_o[s] >> 5]), section_s[s]) == 0U){

    hnd->d[section_o[s] + off] = 0x2020U;
    section_i_setocc(off, &(hnd->o[section_o[s] >> 5]), section_s[s]);

   }else{

    if ((hnd->d[section_o[s] + off] & 0x00FFU) == 0U){
     hnd->d[section_o[s] + off] |= 0x0020U;
    }
    if ((hnd->d[section_o[s] + off] & 0xFF00U) == 0U){
     hnd->d[section_o[s] + off] |= 0x2000U;
    }

   }

  }

 }
}



/* Set section base offset. */
void  section_setbase(section_t* hnd, auint base)
{
 hnd->a[hnd->s] = base;
}



/* Get aligned word address for an offset within the section (that is, base
** added to it). */
auint section_getaddr(section_t* hnd, auint off)
{
 return (off + hnd->a[hnd->s]);
}



/* Get section's size in words. This is calculated from the last occupied word
** of the section. The FILE section always returns zero size. */
auint section_getsize(section_t* hnd)
{
 auint i = SECT_MAX;
 auint s = hnd->s;

 if (s <= SECT_IDM_M){    /* Possible only for sections having map */
  while (i != 0U){
   if (section_i_getocc(i - 1U, &(hnd->o[section_o[s] >> 5]), section_s[s]) != 0U){
    break; /* Topmost occupation found: Size of section */
   }
   i --;
  }
 }else{                   /* Other sections return zero size */
  i = 0U;
 }

 return i;
}



/* Get section data pointer for reading, combining into an application binary.
** The size of the memory area can be retrieved by section_getsize(), */
uint16 const* section_getdata(section_t* hnd)
{
 return (&(hnd->d[section_o[hnd->s]]));
}
